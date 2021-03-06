// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBKIT_FILEAPI_LOCAL_FILE_STREAM_WRITER_H_
#define WEBKIT_FILEAPI_LOCAL_FILE_STREAM_WRITER_H_
#pragma once

#include <utility>

#include "base/compiler_specific.h"
#include "base/callback.h"
#include "base/file_path.h"
#include "base/platform_file.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "webkit/fileapi/fileapi_export.h"
#include "webkit/fileapi/file_stream_writer.h"

namespace net {
class FileStream;
}

namespace fileapi {

// This class is a thin wrapper around net::FileStream for writing local files.
class FILEAPI_EXPORT_PRIVATE LocalFileStreamWriter : public FileStreamWriter {
 public:
  // Create a writer for the existing file in the path |file_path| starting from
  // |initial_offset|.
  LocalFileStreamWriter(const FilePath& file_path, int64 initial_offset);
  virtual ~LocalFileStreamWriter();

  // FileStreamWriteroverrides.
  virtual int Write(net::IOBuffer* buf, int buf_len,
                    const net::CompletionCallback& callback) OVERRIDE;
  virtual int Cancel(const net::CompletionCallback& callback) OVERRIDE;

 private:
  // Opens |file_path_| and if it succeeds, proceeds to InitiateSeek().
  // If failed, the error code is returned by calling |error_callback|.
  int InitiateOpen(const net::CompletionCallback& error_callback,
                   const base::Closure& main_operation);
  void DidOpen(const net::CompletionCallback& error_callback,
               const base::Closure& main_operation,
               int result);

  // Seeks to |initial_offset_| and proceeds to |main_operation| if it succeeds.
  // If failed, the error code is returned by calling |error_callback|.
  void InitiateSeek(const net::CompletionCallback& error_callback,
                    const base::Closure& main_operation);
  void DidSeek(const net::CompletionCallback& error_callback,
               const base::Closure& main_operation,
               int64 result);

  // Passed as the |main_operation| of InitiateOpen() function.
  void ReadyToWrite(net::IOBuffer* buf, int buf_len,
                    const net::CompletionCallback& callback);

  // Writes asynchronously to the file.
  int InitiateWrite(net::IOBuffer* buf, int buf_len,
                    const net::CompletionCallback& callback);
  void DidWrite(const net::CompletionCallback& callback, int result);

  // Stops the in-flight operation and calls |cancel_callback_| if it has been
  // set by Cancel() for the current operation.
  bool CancelIfRequested();

  // Initialization parameters.
  const FilePath file_path_;
  const int64 initial_offset_;

  // Current states of the operation.
  bool has_pending_operation_;
  scoped_ptr<net::FileStream> stream_impl_;
  net::CompletionCallback cancel_callback_;

  base::WeakPtrFactory<LocalFileStreamWriter> weak_factory_;
  DISALLOW_COPY_AND_ASSIGN(LocalFileStreamWriter);
};

}  // namespace fileapi

#endif  // WEBKIT_FILEAPI_LOCAL_FILE_STREAM_WRITER_H_
