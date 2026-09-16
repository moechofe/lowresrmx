//
//  Generated file. Do not edit.
//

// clang-format off

#include "generated_plugin_registrant.h"

#include <share_plus/share_plus_windows_plugin_c_api.h>
#include <sw_rend/sw_rend_plugin_c_api.h>
#include <url_launcher_windows/url_launcher_windows.h>
#include <zstandard_windows/zstandard_windows_plugin_c_api.h>

void RegisterPlugins(flutter::PluginRegistry* registry) {
  SharePlusWindowsPluginCApiRegisterWithRegistrar(
      registry->GetRegistrarForPlugin("SharePlusWindowsPluginCApi"));
  SwRendPluginCApiRegisterWithRegistrar(
      registry->GetRegistrarForPlugin("SwRendPluginCApi"));
  UrlLauncherWindowsRegisterWithRegistrar(
      registry->GetRegistrarForPlugin("UrlLauncherWindows"));
  ZstandardWindowsPluginCApiRegisterWithRegistrar(
      registry->GetRegistrarForPlugin("ZstandardWindowsPluginCApi"));
}
