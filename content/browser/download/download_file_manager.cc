// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/download/download_file_manager.h"

#include <set>
#include <string>

#include "base/bind.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "base/stl_util.h"
#include "base/utf_string_conversions.h"
#include "content/browser/download/base_file.h"
#include "content/browser/download/download_buffer.h"
#include "content/browser/download/download_create_info.h"
#include "content/browser/download/download_file_impl.h"
#include "content/browser/download/download_interrupt_reasons_impl.h"
#include "content/browser/download/download_request_handle.h"
#include "content/browser/download/download_stats.h"
#include "content/browser/power_save_blocker.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/download_manager.h"
#include "content/public/browser/download_manager_delegate.h"
#include "googleurl/src/gurl.h"
#include "net/base/io_buffer.h"

using content::BrowserThread;
using content::DownloadFile;
using content::DownloadId;
using content::DownloadManager;

namespace {

class DownloadFileFactoryImpl
    : public DownloadFileManager::DownloadFileFactory {
 public:
  DownloadFileFactoryImpl() {}

  virtual content::DownloadFile* CreateFile(
      DownloadCreateInfo* info,
      const DownloadRequestHandle& request_handle,
      DownloadManager* download_manager,
      bool calculate_hash,
      const net::BoundNetLog& bound_net_log) OVERRIDE;
};

DownloadFile* DownloadFileFactoryImpl::CreateFile(
    DownloadCreateInfo* info,
    const DownloadRequestHandle& request_handle,
    DownloadManager* download_manager,
    bool calculate_hash,
    const net::BoundNetLog& bound_net_log) {
  return new DownloadFileImpl(
      info, new DownloadRequestHandle(request_handle),
      download_manager, calculate_hash,
      scoped_ptr<PowerSaveBlocker>(
          new PowerSaveBlocker(
              PowerSaveBlocker::kPowerSaveBlockPreventSystemSleep)).Pass(),
      bound_net_log);
}

}  // namespace

DownloadFileManager::DownloadFileManager(DownloadFileFactory* factory)
    : download_file_factory_(factory) {
  if (download_file_factory_ == NULL)
    download_file_factory_.reset(new DownloadFileFactoryImpl);
}

DownloadFileManager::~DownloadFileManager() {
  DCHECK(downloads_.empty());
}

void DownloadFileManager::Shutdown() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  BrowserThread::PostTask(
      BrowserThread::FILE, FROM_HERE,
      base::Bind(&DownloadFileManager::OnShutdown, this));
}

void DownloadFileManager::OnShutdown() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::FILE));
  STLDeleteValues(&downloads_);
}

void DownloadFileManager::CreateDownloadFile(
    DownloadCreateInfo* info, const DownloadRequestHandle& request_handle,
    DownloadManager* download_manager, bool get_hash,
    const net::BoundNetLog& bound_net_log) {
  DCHECK(info);
  VLOG(20) << __FUNCTION__ << "()" << " info = " << info->DebugString();
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::FILE));

  // Life of |info| ends here. No more references to it after this method.
  scoped_ptr<DownloadCreateInfo> infop(info);

  // Create the download file.
  scoped_ptr<DownloadFile> download_file(download_file_factory_->CreateFile(
      info, request_handle, download_manager, get_hash, bound_net_log));

  net::Error init_result = download_file->Initialize();
  if (net::OK != init_result) {
    // Error:  Handle via download manager/item.
    BrowserThread::PostTask(
        BrowserThread::UI,
        FROM_HERE,
        base::Bind(
            &DownloadManager::OnDownloadInterrupted,
            download_manager,
            info->download_id.local(),
            0,
            "",
            content::ConvertNetErrorToInterruptReason(
                init_result, content::DOWNLOAD_INTERRUPT_FROM_DISK)));
  } else {
    DCHECK(GetDownloadFile(info->download_id) == NULL);
    downloads_[info->download_id] = download_file.release();
  }

  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&DownloadManager::StartDownload, download_manager,
                 info->download_id.local()));
}

DownloadFile* DownloadFileManager::GetDownloadFile(
    DownloadId global_id) {
  DownloadFileMap::iterator it = downloads_.find(global_id);
  return it == downloads_.end() ? NULL : it->second;
}

void DownloadFileManager::UpdateInProgressDownloads() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::FILE));
  for (DownloadFileMap::iterator i = downloads_.begin();
       i != downloads_.end(); ++i) {
    DownloadId global_id = i->first;
    DownloadFile* download_file = i->second;
    DownloadManager* manager = download_file->GetDownloadManager();
    if (manager) {
      BrowserThread::PostTask(BrowserThread::UI, FROM_HERE,
          base::Bind(&DownloadManager::UpdateDownload,
                     manager,
                     global_id.local(),
                     download_file->BytesSoFar(),
                     download_file->CurrentSpeed(),
                     download_file->GetHashState()));
    }
  }
}

void DownloadFileManager::StartDownload(
    DownloadCreateInfo* info, const DownloadRequestHandle& request_handle) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  DCHECK(info);

  DownloadManager* manager = request_handle.GetDownloadManager();
  DCHECK(manager);  // Checked in |DownloadResourceHandler::StartOnUIThread()|.

  // |bound_net_log| will be used for logging the both the download item's and
  // the download file's events.
  net::BoundNetLog bound_net_log =
      manager->CreateDownloadItem(info, request_handle);
  bool hash_needed = manager->GenerateFileHash();

  BrowserThread::PostTask(BrowserThread::FILE, FROM_HERE,
      base::Bind(&DownloadFileManager::CreateDownloadFile, this,
                 info, request_handle, make_scoped_refptr(manager),
                 hash_needed, bound_net_log));
}

// We don't forward an update to the UI thread here, since we want to throttle
// the UI update rate via a periodic timer. If the user has cancelled the
// download (in the UI thread), we may receive a few more updates before the IO
// thread gets the cancel message: we just delete the data since the
// DownloadFile has been deleted.
void DownloadFileManager::UpdateDownload(
    DownloadId global_id, content::DownloadBuffer* buffer) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::FILE));
  scoped_ptr<content::ContentVector> contents(buffer->ReleaseContents());

  download_stats::RecordFileThreadReceiveBuffers(contents->size());

  DownloadFile* download_file = GetDownloadFile(global_id);
  bool had_error = false;
  for (size_t i = 0; i < contents->size(); ++i) {
    net::IOBuffer* data = (*contents)[i].first;
    const int data_len = (*contents)[i].second;
    if (!had_error && download_file) {
      net::Error write_result =
          download_file->AppendDataToFile(data->data(), data_len);
      if (write_result != net::OK) {
        // Write failed: interrupt the download.
        DownloadManager* download_manager =
            download_file->GetDownloadManager();
        had_error = true;

        int64 bytes_downloaded = download_file->BytesSoFar();
        std::string hash_state(download_file->GetHashState());

        // Calling this here in case we get more data, to avoid
        // processing data after an error.  That could lead to
        // files that are corrupted if the later processing succeeded.
        CancelDownload(global_id);
        download_file = NULL;  // Was deleted in |CancelDownload|.

        if (download_manager) {
          BrowserThread::PostTask(
              BrowserThread::UI, FROM_HERE,
              base::Bind(&DownloadManager::OnDownloadInterrupted,
                         download_manager,
                         global_id.local(),
                         bytes_downloaded,
                         hash_state,
                         content::ConvertNetErrorToInterruptReason(
                             write_result,
                             content::DOWNLOAD_INTERRUPT_FROM_DISK)));
        }
      }
    }
    data->Release();
  }
}

void DownloadFileManager::OnResponseCompleted(
    DownloadId global_id,
    content::DownloadInterruptReason reason,
    const std::string& security_info) {
  VLOG(20) << __FUNCTION__ << "()" << " id = " << global_id
           << " reason = " << InterruptReasonDebugString(reason)
           << " security_info = \"" << security_info << "\"";
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::FILE));
  DownloadFile* download_file = GetDownloadFile(global_id);
  if (!download_file)
    return;

  download_file->Finish();

  DownloadManager* download_manager = download_file->GetDownloadManager();
  if (!download_manager) {
    CancelDownload(global_id);
    return;
  }

  if (reason == content::DOWNLOAD_INTERRUPT_REASON_NONE) {
    std::string hash;
    if (!download_file->GetHash(&hash) ||
        BaseFile::IsEmptyHash(hash)) {
      hash.clear();
    }

    BrowserThread::PostTask(
        BrowserThread::UI, FROM_HERE,
        base::Bind(&DownloadManager::OnResponseCompleted,
                   download_manager, global_id.local(),
                   download_file->BytesSoFar(), hash));
  } else {
    BrowserThread::PostTask(
        BrowserThread::UI, FROM_HERE,
        base::Bind(&DownloadManager::OnDownloadInterrupted,
                   download_manager,
                   global_id.local(),
                   download_file->BytesSoFar(),
                   download_file->GetHashState(),
                   reason));
  }
  // We need to keep the download around until the UI thread has finalized
  // the name.
}

// This method will be sent via a user action, or shutdown on the UI thread, and
// run on the download thread. Since this message has been sent from the UI
// thread, the download may have already completed and won't exist in our map.
void DownloadFileManager::CancelDownload(DownloadId global_id) {
  VLOG(20) << __FUNCTION__ << "()" << " id = " << global_id;
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::FILE));
  DownloadFileMap::iterator it = downloads_.find(global_id);
  if (it == downloads_.end())
    return;

  DownloadFile* download_file = it->second;
  VLOG(20) << __FUNCTION__ << "()"
           << " download_file = " << download_file->DebugString();
  download_file->Cancel();

  EraseDownload(global_id);
}

void DownloadFileManager::CompleteDownload(DownloadId global_id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::FILE));

  if (!ContainsKey(downloads_, global_id))
    return;

  DownloadFile* download_file = downloads_[global_id];

  VLOG(20) << " " << __FUNCTION__ << "()"
           << " id = " << global_id
           << " download_file = " << download_file->DebugString();

  download_file->Detach();

  EraseDownload(global_id);
}

void DownloadFileManager::OnDownloadManagerShutdown(DownloadManager* manager) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::FILE));
  DCHECK(manager);

  std::set<DownloadFile*> to_remove;

  for (DownloadFileMap::iterator i = downloads_.begin();
       i != downloads_.end(); ++i) {
    DownloadFile* download_file = i->second;
    if (download_file->GetDownloadManager() == manager) {
      download_file->CancelDownloadRequest();
      to_remove.insert(download_file);
    }
  }

  for (std::set<DownloadFile*>::iterator i = to_remove.begin();
       i != to_remove.end(); ++i) {
    downloads_.erase((*i)->GlobalId());
    delete *i;
  }
}

// Actions from the UI thread and run on the download thread

// The DownloadManager in the UI thread has provided an intermediate .crdownload
// name for the download specified by 'id'. Rename the in progress download.
//
// There are 2 possible rename cases where this method can be called:
// 1. tmp -> foo.crdownload (not final, safe)
// 2. tmp-> Unconfirmed.xxx.crdownload (not final, dangerous)
// TODO(asanka): Merge with RenameCompletingDownloadFile() and move
//               uniquification logic into DownloadFile.
void DownloadFileManager::RenameInProgressDownloadFile(
    DownloadId global_id,
    const FilePath& full_path,
    bool overwrite_existing_file,
    const RenameCompletionCallback& callback) {
  VLOG(20) << __FUNCTION__ << "()" << " id = " << global_id
           << " full_path = \"" << full_path.value() << "\"";
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::FILE));

  DownloadFile* download_file = GetDownloadFile(global_id);
  if (!download_file) {
    BrowserThread::PostTask(BrowserThread::UI, FROM_HERE,
                            base::Bind(callback, FilePath()));
    return;
  }

  VLOG(20) << __FUNCTION__ << "()"
           << " download_file = " << download_file->DebugString();
  FilePath new_path(full_path);
  if (!overwrite_existing_file) {
    int uniquifier =
        file_util::GetUniquePathNumber(new_path, FILE_PATH_LITERAL(""));
    if (uniquifier > 0) {
      new_path = new_path.InsertBeforeExtensionASCII(
          StringPrintf(" (%d)", uniquifier));
    }
  }

  net::Error rename_error = download_file->Rename(new_path);
  if (net::OK != rename_error) {
    CancelDownloadOnRename(global_id, rename_error);
    new_path.clear();
  }
  BrowserThread::PostTask(BrowserThread::UI, FROM_HERE,
                          base::Bind(callback, new_path));
}

// The DownloadManager in the UI thread has provided a final name for the
// download specified by 'id'. Rename the download that's in the process
// of completing.
//
// There are 2 possible rename cases where this method can be called:
// 1. foo.crdownload -> foo (final, safe)
// 2. Unconfirmed.xxx.crdownload -> xxx (final, validated)
void DownloadFileManager::RenameCompletingDownloadFile(
    DownloadId global_id,
    const FilePath& full_path,
    bool overwrite_existing_file,
    const RenameCompletionCallback& callback) {
  VLOG(20) << __FUNCTION__ << "()" << " id = " << global_id
           << " overwrite_existing_file = " << overwrite_existing_file
           << " full_path = \"" << full_path.value() << "\"";
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::FILE));

  DownloadFile* download_file = GetDownloadFile(global_id);
  if (!download_file) {
    BrowserThread::PostTask(BrowserThread::UI, FROM_HERE,
                            base::Bind(callback, FilePath()));
    return;
  }

  VLOG(20) << __FUNCTION__ << "()"
           << " download_file = " << download_file->DebugString();

  FilePath new_path = full_path;
  if (!overwrite_existing_file) {
    // Make our name unique at this point, as if a dangerous file is
    // downloading and a 2nd download is started for a file with the same
    // name, they would have the same path.  This is because we uniquify
    // the name on download start, and at that time the first file does
    // not exists yet, so the second file gets the same name.
    // This should not happen in the SAFE case, and we check for that in the UI
    // thread.
    int uniquifier =
        file_util::GetUniquePathNumber(new_path, FILE_PATH_LITERAL(""));
    if (uniquifier > 0) {
      new_path = new_path.InsertBeforeExtensionASCII(
          StringPrintf(" (%d)", uniquifier));
    }
  }

  // Rename the file, overwriting if necessary.
  net::Error rename_error = download_file->Rename(new_path);
  if (net::OK != rename_error) {
    // Error. Between the time the UI thread generated 'full_path' to the time
    // this code runs, something happened that prevents us from renaming.
    CancelDownloadOnRename(global_id, rename_error);
    new_path.clear();
  } else {
#if defined(OS_MACOSX)
    // Done here because we only want to do this once; see
    // http://crbug.com/13120 for details.
    download_file->AnnotateWithSourceInformation();
#endif
  }

  BrowserThread::PostTask(BrowserThread::UI, FROM_HERE,
                          base::Bind(callback, new_path));
}

int DownloadFileManager::NumberOfActiveDownloads() const {
  return downloads_.size();
}

// Called only from RenameInProgressDownloadFile and
// RenameCompletingDownloadFile on the FILE thread.
// TODO(asanka): Use the RenameCompletionCallback instead of a separate
//               OnDownloadInterrupted call.
void DownloadFileManager::CancelDownloadOnRename(
    DownloadId global_id, net::Error rename_error) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::FILE));

  DownloadFile* download_file = GetDownloadFile(global_id);
  if (!download_file)
    return;

  DownloadManager* download_manager = download_file->GetDownloadManager();
  if (!download_manager) {
    // Without a download manager, we can't cancel the request normally, so we
    // need to do it here.  The normal path will also update the download
    // history before canceling the request.
    download_file->CancelDownloadRequest();
    return;
  }

  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&DownloadManager::OnDownloadInterrupted,
                 download_manager,
                 global_id.local(),
                 download_file->BytesSoFar(),
                 download_file->GetHashState(),
                 content::ConvertNetErrorToInterruptReason(
                     rename_error,
                     content::DOWNLOAD_INTERRUPT_FROM_DISK)));
}

void DownloadFileManager::EraseDownload(DownloadId global_id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::FILE));

  if (!ContainsKey(downloads_, global_id))
    return;

  DownloadFile* download_file = downloads_[global_id];

  VLOG(20) << " " << __FUNCTION__ << "()"
           << " id = " << global_id
           << " download_file = " << download_file->DebugString();

  downloads_.erase(global_id);

  delete download_file;
}
