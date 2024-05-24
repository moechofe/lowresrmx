//
//  Generated file. Do not edit.
//

// clang-format off

#include "generated_plugin_registrant.h"

#include <texture_plugin/texture_plugin.h>

void fl_register_plugins(FlPluginRegistry* registry) {
  g_autoptr(FlPluginRegistrar) texture_plugin_registrar =
      fl_plugin_registry_get_registrar_for_plugin(registry, "TexturePlugin");
  texture_plugin_register_with_registrar(texture_plugin_registrar);
}
