// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/shell_browser_main.h"

#include <iostream>

#include "base/command_line.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/threading/thread_restrictions.h"
#include "content/public/browser/browser_main_runner.h"
#include "content/shell/shell.h"
#include "content/shell/shell_browser_context.h"
#include "content/shell/shell_content_browser_client.h"
#include "content/shell/shell_switches.h"
#include "webkit/support/webkit_support.h"

namespace {

GURL GetURLForLayoutTest(const char* test_name) {
#if defined(OS_ANDROID)
  // DumpRenderTree is not currently supported for Android using the content
  // shell.
  NOTIMPLEMENTED();
  return GURL::EmptyGURL();
#else
  std::string path_or_url = test_name;
  std::string pixel_hash;
  std::string timeout;
  std::string::size_type separator_position = path_or_url.find(' ');
  if (separator_position != std::string::npos) {
    timeout = path_or_url.substr(separator_position + 1);
    path_or_url.erase(separator_position);
    separator_position = path_or_url.find(' ');
    if (separator_position != std::string::npos) {
      pixel_hash = timeout.substr(separator_position + 1);
      timeout.erase(separator_position);
    }
  }
  // TODO(jochen): use pixel_hash and timeout.
  GURL test_url = webkit_support::CreateURLForPathOrURL(path_or_url);
  {
    // We're outside of the message loop here, and this is a test.
    base::ThreadRestrictions::ScopedAllowIO allow_io;
    webkit_support::SetCurrentDirectoryForFileURL(test_url);
  }
  return test_url;
#endif
}

}  // namespace

// Main routine for running as the Browser process.
int ShellBrowserMain(const content::MainFunctionParams& parameters) {
  scoped_ptr<content::BrowserMainRunner> main_runner_(
      content::BrowserMainRunner::Create());

  int exit_code = main_runner_->Initialize(parameters);

#if defined(OS_ANDROID)
  DCHECK(exit_code < 0);

  // Return 0 so that we do NOT trigger the default behavior. On Android, the
  // UI message loop is managed by the Java application.
  return 0;
#else
  if (exit_code >= 0)
    return exit_code;

  if (CommandLine::ForCurrentProcess()->HasSwitch(
        switches::kCheckLayoutTestSysDeps)) {
    return 0;
  }

  bool layout_test_mode =
      CommandLine::ForCurrentProcess()->HasSwitch(switches::kDumpRenderTree);

  if (layout_test_mode) {
    char test_string[2048];
    content::ShellBrowserContext* browser_context =
        static_cast<content::ShellContentBrowserClient*>(
            content::GetContentClient()->browser())->browser_context();

    while (fgets(test_string, sizeof(test_string), stdin)) {
      char *new_line_position = strchr(test_string, '\n');
      if (new_line_position)
        *new_line_position = '\0';
      if (test_string[0] == '\0')
        continue;

      content::Shell::CreateNewWindow(browser_context,
                                      GetURLForLayoutTest(test_string),
                                      NULL,
                                      MSG_ROUTING_NONE,
                                      NULL);
      main_runner_->Run();

      content::Shell::CloseAllWindows();

      // Test footer.
      std::cout << "#EOF\n";
      std::cout.flush();

    }
    exit_code = 0;
  } else {
    exit_code = main_runner_->Run();
  }

  main_runner_->Shutdown();

  return exit_code;
#endif
}
