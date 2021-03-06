# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'test_support_content',
      'type': 'static_library',
      'defines!': ['CONTENT_IMPLEMENTATION'],
      'dependencies': [
        '../build/temp_gyp/googleurl.gyp:googleurl',
        'content_browser',
        'content_common',
        '../skia/skia.gyp:skia',
        '../testing/gmock.gyp:gmock',
        '../testing/gtest.gyp:gtest',
        '../third_party/libvpx/libvpx.gyp:libvpx',
        '<(webkit_src_dir)/Source/WebKit/chromium/WebKit.gyp:webkit',
        '../ui/surface/surface.gyp:surface',
        '../ui/ui.gyp:ui_test_support',
        '../webkit/support/webkit_support.gyp:appcache',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        'public/test/accessibility_test_utils_win.h',
        'public/test/browser_test.h',
        'public/test/content_test_suite_base.h',
        'public/test/js_injection_ready_observer.h',
        'public/test/mock_download_item.h',
        'public/test/mock_download_manager.h',
        'public/test/test_navigation_observer.h',
        # TODO(phajdan.jr): All of those files should live in content/test (if
        # they're only used by content) or content/public/test (if they're used
        # by other embedders).
        'browser/download/mock_download_file.cc',
        'browser/download/mock_download_file.h',
        'browser/geolocation/arbitrator_dependency_factories_for_test.cc',
        'browser/geolocation/arbitrator_dependency_factories_for_test.h',
        'browser/geolocation/fake_access_token_store.cc',
        'browser/geolocation/fake_access_token_store.h',
        'browser/geolocation/mock_location_provider.cc',
        'browser/geolocation/mock_location_provider.h',
        'browser/mock_content_browser_client.cc',
        'browser/mock_content_browser_client.h',
        'browser/renderer_host/media/mock_media_observer.cc',
        'browser/renderer_host/media/mock_media_observer.h',
        'browser/renderer_host/test_backing_store.cc',
        'browser/renderer_host/test_backing_store.h',
        'browser/renderer_host/test_render_view_host.cc',
        'browser/renderer_host/test_render_view_host.h',
        'browser/web_contents/test_web_contents.cc',
        'browser/web_contents/test_web_contents.h',
        'common/test_url_constants.cc',
        'common/test_url_constants.h',
        'gpu/gpu_idirect3d9_mock_win.cc',
        'gpu/gpu_idirect3d9_mock_win.h',
        'renderer/mock_content_renderer_client.cc',
        'renderer/mock_content_renderer_client.h',
        'test/accessibility_test_utils_win.cc',
        'test/browser_test_base.cc',
        'test/browser_test_base.h',
        'test/content_test_suite.cc',
        'test/content_test_suite.h',
        'test/content_test_suite_base.cc',
        'test/gpu/gpu_test_config.cc',
        'test/gpu/gpu_test_config.h',
        'test/gpu/gpu_test_expectations_parser.cc',
        'test/gpu/gpu_test_expectations_parser.h',
        'test/mock_download_item.cc',
        'test/mock_download_manager.cc',
        'test/mock_keyboard.cc',
        'test/mock_keyboard_driver_win.cc',
        'test/mock_keyboard_driver_win.h',
        'test/mock_keyboard.h',
        'test/mock_render_process.cc',
        'test/mock_render_process.h',
        'test/mock_render_process_host.cc',
        'test/mock_render_process_host.h',
        'test/mock_render_thread.cc',
        'test/mock_render_thread.h',
        'test/mock_resource_context.cc',
        'test/mock_resource_context.h',
        'test/mock_web_ui.cc',
        'test/mock_web_ui.h',
        'test/net/url_request_abort_on_end_job.cc',
        'test/net/url_request_abort_on_end_job.h',
        'test/render_view_fake_resources_test.cc',
        'test/render_view_fake_resources_test.h',
        'test/render_view_test.cc',
        'test/render_view_test.h',
        'test/test_browser_context.cc',
        'test/test_browser_context.h',
        'test/test_browser_thread.cc',
        'test/test_browser_thread.h',
        'test/test_content_client.cc',
        'test/test_content_client.h',
        'test/test_content_client_initializer.cc',
        'test/test_content_client_initializer.h',
        'test/test_file_error_injector.cc',
        'test/test_file_error_injector.h',
        'test/test_navigation_observer.cc',
        'test/test_notification_tracker.cc',
        'test/test_notification_tracker.h',
        'test/test_renderer_host.cc',
        'test/test_renderer_host.h',
        'test/test_render_view_host_factory.cc',
        'test/test_render_view_host_factory.h',
        'test/test_url_fetcher_factory.cc',
        'test/test_url_fetcher_factory.h',
        'test/test_web_contents_view.cc',
        'test/test_web_contents_view.h',
        'test/unittest_test_suite.cc',
        'test/unittest_test_suite.h',
        'test/web_contents_tester.cc',
        'test/web_contents_tester.h',

        # TODO(phajdan.jr): Those files should be moved to webkit
        # test support target.
        '../webkit/appcache/appcache_test_helper.cc',
        '../webkit/appcache/appcache_test_helper.h',
        '../webkit/quota/mock_special_storage_policy.cc',
        '../webkit/quota/mock_special_storage_policy.h',
      ],
      'conditions': [
        ['enable_webrtc==1', {
          'sources': [
            'renderer/media/mock_media_stream_dependency_factory.cc',
            'renderer/media/mock_media_stream_dependency_factory.h',
            'renderer/media/mock_media_stream_dispatcher.cc',
            'renderer/media/mock_media_stream_dispatcher.h',
            'renderer/media/mock_peer_connection_impl.cc',
            'renderer/media/mock_peer_connection_impl.h',
            'renderer/media/mock_web_peer_connection_00_handler_client.cc',
            'renderer/media/mock_web_peer_connection_00_handler_client.h',
            'renderer/media/mock_web_peer_connection_handler_client.cc',
            'renderer/media/mock_web_peer_connection_handler_client.h',
            'test/webrtc_audio_device_test.cc',
            'test/webrtc_audio_device_test.h',
          ],
          'dependencies': [
            '../third_party/libjingle/libjingle.gyp:libjingle_peerconnection',
            '../third_party/webrtc/modules/modules.gyp:audio_device',
            '../third_party/webrtc/modules/modules.gyp:video_capture_module',
            '../third_party/webrtc/system_wrappers/source/system_wrappers.gyp:system_wrappers',
            '../third_party/webrtc/video_engine/video_engine.gyp:video_engine_core',
            '../third_party/webrtc/voice_engine/voice_engine.gyp:voice_engine_core'],
        }],
        ['toolkit_uses_gtk == 1', {
          'dependencies': [
            '../build/linux/system.gyp:gtk',
          ],
        }],
        ['use_glib == 1', {
          'dependencies': [
            '../build/linux/system.gyp:glib',
          ],
        }],
        ['use_aura==1', {
          'dependencies': [
            '../ui/aura/aura.gyp:test_support_aura',
            '../ui/compositor/compositor.gyp:compositor',
          ],
        }],
        ['OS=="win"', {
          'dependencies': [
            '../third_party/iaccessible2/iaccessible2.gyp:iaccessible2',
          ],
        }],
      ],
    },
    {
      'target_name': 'content_unittests',
      'type': '<(gtest_target_type)',
      'defines!': ['CONTENT_IMPLEMENTATION'],
      'dependencies': [
        'content_browser',
        'content_gpu',
        'content_plugin',
        'content_renderer',
        'test_support_content',
        'content_resources.gyp:content_resources',
        '../base/base.gyp:test_support_base',
        '../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../crypto/crypto.gyp:crypto',
        '../gpu/gpu.gyp:gpu_unittest_utils',
        '../ipc/ipc.gyp:test_support_ipc',
        '../jingle/jingle.gyp:jingle_glue_test_util',
        '../media/media.gyp:media_test_support',
        '../net/net.gyp:net_test_support',
        '../skia/skia.gyp:skia',
        '../testing/gmock.gyp:gmock',
        '../testing/gtest.gyp:gtest',
        '../third_party/libjingle/libjingle.gyp:libjingle',
        '../third_party/libvpx/libvpx.gyp:libvpx',
        '<(webkit_src_dir)/Source/WebKit/chromium/WebKit.gyp:webkit',
        '../ui/gl/gl.gyp:gl',
        '../ui/ui.gyp:ui',
        '../v8/tools/gyp/v8.gyp:v8',
        '../webkit/support/webkit_support.gyp:database',
        '../webkit/support/webkit_support.gyp:glue',
        '../webkit/support/webkit_support.gyp:quota',
        '../webkit/support/webkit_support.gyp:webkit_media',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        'app/startup_helper_win.cc',
        'browser/accessibility/browser_accessibility_mac_unittest.mm',
        'browser/accessibility/browser_accessibility_manager_unittest.cc',
        'browser/accessibility/browser_accessibility_win_unittest.cc',
        'browser/appcache/chrome_appcache_service_unittest.cc',
        'browser/browser_thread_unittest.cc',
        'browser/browser_url_handler_impl_unittest.cc',
        'browser/child_process_security_policy_unittest.cc',
        'browser/debugger/devtools_manager_unittest.cc',
        'browser/device_orientation/provider_unittest.cc',
        'browser/download/base_file_unittest.cc',
        'browser/download/byte_stream_unittest.cc',
        'browser/download/download_buffer_unittest.cc',
        'browser/download/download_file_manager_unittest.cc',
        'browser/download/download_file_unittest.cc',
        'browser/download/download_id_unittest.cc',
        'browser/download/download_item_impl_unittest.cc',
        'browser/download/download_manager_impl_unittest.cc',
        'browser/download/save_package_unittest.cc',
        'browser/gamepad/gamepad_provider_unittest.cc',
        'browser/geolocation/device_data_provider_unittest.cc',
        'browser/geolocation/geolocation_provider_unittest.cc',
        'browser/geolocation/gps_location_provider_unittest_linux.cc',
        'browser/geolocation/location_arbitrator_unittest.cc',
        'browser/geolocation/network_location_provider_unittest.cc',
        'browser/geolocation/wifi_data_provider_common_unittest.cc',
        'browser/geolocation/wifi_data_provider_linux_unittest.cc',
        'browser/geolocation/wifi_data_provider_unittest_win.cc',
        'browser/geolocation/win7_location_api_unittest_win.cc',
        'browser/geolocation/win7_location_provider_unittest_win.cc',
        'browser/host_zoom_map_impl_unittest.cc',
        'browser/in_process_webkit/indexed_db_quota_client_unittest.cc',
        'browser/in_process_webkit/indexed_db_unittest.cc',
        'browser/in_process_webkit/webkit_thread_unittest.cc',
        'browser/intents/intent_injector_unittest.cc',
        'browser/intents/internal_web_intents_dispatcher_unittest.cc',
        'browser/mach_broker_mac_unittest.cc',
        'browser/notification_service_impl_unittest.cc',
        'browser/plugin_loader_posix_unittest.cc',
        'browser/renderer_host/gtk_key_bindings_handler_unittest.cc',
        'browser/renderer_host/media/audio_input_device_manager_unittest.cc',
        'browser/renderer_host/media/audio_renderer_host_unittest.cc',
        'browser/renderer_host/media/media_stream_dispatcher_host_unittest.cc',
        'browser/renderer_host/media/video_capture_controller_unittest.cc',
        'browser/renderer_host/media/video_capture_host_unittest.cc',
        'browser/renderer_host/media/video_capture_manager_unittest.cc',
        'browser/renderer_host/render_view_host_unittest.cc',
        'browser/renderer_host/render_widget_host_unittest.cc',
        'browser/renderer_host/render_widget_host_view_aura_unittest.cc',
        'browser/renderer_host/render_widget_host_view_mac_editcommand_helper_unittest.mm',
        'browser/renderer_host/render_widget_host_view_mac_unittest.mm',
        'browser/renderer_host/resource_dispatcher_host_unittest.cc',
        'browser/renderer_host/text_input_client_mac_unittest.mm',
        'browser/resolve_proxy_msg_helper_unittest.cc',
        'browser/site_instance_impl_unittest.cc',
        'browser/speech/chunked_byte_buffer_unittest.cc',
        'browser/speech/endpointer/endpointer_unittest.cc',
        'browser/speech/google_one_shot_remote_engine_unittest.cc',
        'browser/speech/speech_recognizer_impl_unittest.cc',
        'browser/ssl/ssl_host_state_unittest.cc',
        'browser/system_message_window_win_unittest.cc',
        'browser/trace_subscriber_stdio_unittest.cc',
        'browser/web_contents/navigation_controller_impl_unittest.cc',
        'browser/web_contents/navigation_entry_impl_unittest.cc',
        'browser/web_contents/render_view_host_manager_unittest.cc',
        'browser/web_contents/web_contents_delegate_unittest.cc',
        'browser/web_contents/web_contents_impl_unittest.cc',
        'browser/web_contents/web_contents_view_mac_unittest.mm',
        'browser/web_contents/web_drag_dest_mac_unittest.mm',
        'browser/web_contents/web_drag_source_mac_unittest.mm',
        'browser/webui/web_ui_message_handler_unittest.cc',
        'common/android/address_parser_unittest.cc',
        'common/mac/attributed_string_coder_unittest.mm',
        'common/mac/font_descriptor_unittest.mm',
        'common/gpu/gpu_info_unittest.cc',
        'common/gpu/gpu_memory_manager_unittest.cc',
        'common/hi_res_timer_manager_unittest.cc',
        'common/indexed_db/indexed_db_dispatcher_unittest.cc',
        'common/inter_process_time_ticks_converter_unittest.cc',
        'common/net/url_fetcher_impl_unittest.cc',
        'common/page_zoom_unittest.cc',
        'common/resource_dispatcher_unittest.cc',
        'common/sandbox_mac_diraccess_unittest.mm',
        'common/sandbox_mac_fontloading_unittest.mm',
        'common/sandbox_mac_unittest_helper.h',
        'common/sandbox_mac_unittest_helper.mm',
        'common/sandbox_mac_system_access_unittest.mm',
        'gpu/gpu_info_collector_unittest.cc',
        'gpu/gpu_info_collector_unittest_win.cc',
        'renderer/active_notification_tracker_unittest.cc',
        'renderer/android/email_detector_unittest.cc',
        'renderer/android/phone_number_detector_unittest.cc',
        'renderer/gpu/input_event_filter_unittest.cc',
        'renderer/media/audio_message_filter_unittest.cc',
        'renderer/media/capture_video_decoder_unittest.cc',
        'renderer/media/video_capture_impl_unittest.cc',
        'renderer/media/video_capture_message_filter_unittest.cc',
        'renderer/paint_aggregator_unittest.cc',
        'renderer/pepper/pepper_broker_impl_unittest.cc',
        'renderer/v8_value_converter_impl_unittest.cc',
        'test/gpu/gpu_test_config_unittest.cc',
        'test/gpu/gpu_test_expectations_parser_unittest.cc',
        'test/run_all_unittests.cc',
        '../webkit/media/buffered_data_source_unittest.cc',
        '../webkit/media/buffered_resource_loader_unittest.cc',
        '../webkit/media/cache_util_unittest.cc',
        '../webkit/media/skcanvas_video_renderer_unittest.cc',
        '../webkit/media/test_response_generator.cc',
        '../webkit/media/test_response_generator.h',
        '../webkit/mocks/mock_weburlloader.cc',
        '../webkit/mocks/mock_weburlloader.h',
      ],
      'conditions': [
        ['enable_webrtc==1', {
          'sources': [
            'browser/renderer_host/p2p/socket_host_test_utils.h',
            'browser/renderer_host/p2p/socket_host_tcp_unittest.cc',
            'browser/renderer_host/p2p/socket_host_tcp_server_unittest.cc',
            'browser/renderer_host/p2p/socket_host_udp_unittest.cc',
            'renderer/media/media_stream_dispatcher_unittest.cc',
            'renderer/media/media_stream_impl_unittest.cc',
            'renderer/media/peer_connection_handler_jsep_unittest.cc',
            'renderer/media/peer_connection_handler_unittest.cc',
            'renderer/media/rtc_video_decoder_unittest.cc',
            'renderer/media/webrtc_audio_device_unittest.cc',
            'renderer/p2p/p2p_transport_impl_unittest.cc',
          ],
          'dependencies': [
            '../third_party/libjingle/libjingle.gyp:libjingle_peerconnection',
            '../third_party/webrtc/modules/modules.gyp:video_capture_module',
            '../third_party/webrtc/system_wrappers/source/system_wrappers.gyp:system_wrappers',
            '../third_party/webrtc/video_engine/video_engine.gyp:video_engine_core',
            '../third_party/webrtc/voice_engine/voice_engine.gyp:voice_engine_core',
          ]
        }],
        # TODO(jrg): remove the OS=="android" section?
        # http://crbug.com/113172
        # Understand better how media_stream_ is tied into Chromium.
        ['enable_webrtc==0 and OS=="android"', {
          'sources/': [
            ['exclude', '^renderer/media/media_stream_'],
          ],
        }],
        ['input_speech==0', {
          'sources/': [
            ['exclude', '^browser/speech/'],
          ]
        }],
        ['notifications==0', {
           'sources!': [
             'renderer/active_notification_tracker_unittest.cc',
           ],
        }],
        ['use_x11 == 1', {
          'dependencies': [
            '../build/linux/system.gyp:dbus',
            '../dbus/dbus.gyp:dbus_test_support',
          ],
        }],
        ['OS=="win" and win_use_allocator_shim==1', {
          'dependencies': [
            '../base/allocator/allocator.gyp:allocator',
          ],
        }],
        ['OS=="win"', {
          'dependencies': [
            '../third_party/iaccessible2/iaccessible2.gyp:iaccessible2',
          ],
        }],
        ['OS=="mac"', {
          # These flags are needed to run the test on Mac.
          # Search for comments about "xcode_settings" in chrome_tests.gypi.
          'xcode_settings': {'OTHER_LDFLAGS': ['-Wl,-ObjC']},
        }],
        ['chromeos==1', {
          'sources/': [
            ['exclude', '^browser/renderer_host/gtk_key_bindings_handler_unittest.cc'],
          ],
        }],
        ['OS == "win" or (toolkit_uses_gtk == 1 and selinux == 0)', {
          'dependencies': [
            '../sandbox/sandbox.gyp:sandbox',
          ],
        }],
        ['use_aura==1', {
          'dependencies': [
            '../ui/aura/aura.gyp:aura',
          ],
          'sources!': [
            'browser/accessibility/browser_accessibility_win_unittest.cc',
          ],
        }],
        ['OS == "android"', {
          'sources!': [
            'browser/geolocation/device_data_provider_unittest.cc',
            'browser/geolocation/gps_location_provider_unittest_linux.cc',
            'browser/geolocation/network_location_provider_unittest.cc',
            'browser/geolocation/wifi_data_provider_common_unittest.cc',
            'browser/geolocation/wifi_data_provider_linux_unittest.cc',
          ],
        }],
        ['OS == "android" and gtest_target_type == "shared_library"', {
          'dependencies': [
            '../testing/android/native_test.gyp:native_test_native_code',
          ]
        }],
      ],
    },
    {
      'target_name': 'content_browsertests',
      'type': 'executable',
      'defines!': ['CONTENT_IMPLEMENTATION'],
      'dependencies': [
        'content_browser',
        'content_gpu',
        'content_plugin',
        'content_renderer',
        'content_shell_lib',
        'test_support_content',
        '../base/base.gyp:test_support_base',
        '../net/net.gyp:net_test_support',
        '../skia/skia.gyp:skia',
        '../testing/gtest.gyp:gtest',
        '../ui/ui.gyp:ui',
        '../webkit/support/webkit_support.gyp:glue',
      ],
      'include_dirs': [
        '..',
      ],
      'defines': [
        'HAS_OUT_OF_PROC_TEST_RUNNER',
      ],
      'sources': [
        'app/startup_helper_win.cc',
        'test/content_browser_test.h',
        'test/content_browser_test.cc',
        'test/content_test_launcher.cc',
        'test/test_launcher.cc',
        'test/test_launcher.h',
      ],
      'conditions': [
        ['OS=="win"', {
          'resource_include_dirs': [
            '<(SHARED_INTERMEDIATE_DIR)/webkit',
          ],
          'sources': [
            'shell/resource.h',
            'shell/shell.rc',
            # TODO:  It would be nice to have these pulled in
            # automatically from direct_dependent_settings in
            # their various targets (net.gyp:net_resources, etc.),
            # but that causes errors in other targets when
            # resulting .res files get referenced multiple times.
            '<(SHARED_INTERMEDIATE_DIR)/net/net_resources.rc',
            '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_chromium_resources.rc',
            '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_resources.rc',
            '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_strings_en-US.rc',
          ],
          'dependencies': [
            '<(DEPTH)/net/net.gyp:net_resources',
            '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_resources',
            '<(DEPTH)/webkit/support/webkit_support.gyp:webkit_strings',
          ],
          'configurations': {
            'Debug_Base': {
              'msvs_settings': {
                'VCLinkerTool': {
                  'LinkIncremental': '<(msvs_large_module_debug_link_mode)',
                },
              },
            },
          },
        }],
        ['OS=="win" and win_use_allocator_shim==1', {
          'dependencies': [
            '../base/allocator/allocator.gyp:allocator',
          ],
        }],
        ['OS == "win" or (toolkit_uses_gtk == 1 and selinux == 0)', {
          'dependencies': [
            '../sandbox/sandbox.gyp:sandbox',
          ],
        }],
      ],
    },
  ],
  'conditions': [
    ['target_arch=="arm" or OS=="win" or OS=="mac"', {
      'targets': [
        {
          'conditions': [
            ['target_arch=="arm"', {
              'target_name': 'omx_video_decode_accelerator_unittest',
              'include_dirs': [
                '<(DEPTH)/third_party/openmax/il',
              ],
            }],
            ['OS=="mac"', {
              'target_name': 'video_decode_accelerator_unittest',
              'dependencies': [
                '../ui/gl/gl.gyp:gl',
                '../ui/ui.gyp:ui',
              ],
              'sources!': [
                'common/gpu/media/rendering_helper_egl.cc',
              ],
            }],
            ['OS=="win"', {
              'target_name': 'dxva_video_decode_accelerator_unittest',
              'dependencies': [
                '../third_party/angle/src/build_angle.gyp:libEGL',
                '../third_party/angle/src/build_angle.gyp:libGLESv2',
                '../ui/gl/gl.gyp:gl',
              ],
              'conditions': [
                ['win_use_allocator_shim==1', {
                  'dependencies': [
                    '../base/allocator/allocator.gyp:allocator',
                  ],
                }],
              ],
            }],
          ],
          'defines!': ['CONTENT_IMPLEMENTATION'],
          'type': 'executable',
          'dependencies': [
            '../base/base.gyp:base',
            'content',
            '../testing/gtest.gyp:gtest',
            '../media/media.gyp:media',
            '../ui/ui.gyp:ui',
          ],
          'include_dirs': [
            '<(DEPTH)/third_party/angle/include',
          ],
          'sources': [
            'common/gpu/media/rendering_helper.h',
            'common/gpu/media/rendering_helper_mac.mm',
            'common/gpu/media/rendering_helper_egl.cc',
            'common/gpu/media/video_decode_accelerator_unittest.cc',
          ],
        }
      ],
    }],
    ['chromeos == 1', {
      'targets': [
        {
          'target_name': 'h264_parser_unittest',
          'type': 'executable',
          'dependencies': [
            'content_common',
            '../testing/gtest.gyp:gtest',
          ],
          'sources': [
            'common/gpu/media/h264_parser_unittest.cc',
          ],
        }
      ],
    }],
    # Special target to wrap a gtest_target_type==shared_library
    # content_unittests into an android apk for execution.
    # See base.gyp for TODO(jrg)s about this strategy.
    ['OS == "android" and gtest_target_type == "shared_library"', {
      'targets': [
        {
          'target_name': 'content_unittests_apk',
          'type': 'none',
          'dependencies': [
            '../base/base.gyp:base_java',
            'content_java',
            'content_unittests',
          ],
          'variables': {
            'test_suite_name': 'content_unittests',
            'input_shlib_path': '<(SHARED_LIB_DIR)/<(SHARED_LIB_PREFIX)content_unittests<(SHARED_LIB_SUFFIX)',
            'input_jars_paths': [
              '<(PRODUCT_DIR)/lib.java/chromium_base.jar',
              '<(PRODUCT_DIR)/lib.java/chromium_content.jar',
            ],
          },
          'includes': [ '../build/apk_test.gypi' ],
        },
      ],
    }],
  ],
}
