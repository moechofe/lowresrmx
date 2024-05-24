
import 'texture_plugin_platform_interface.dart';

class TexturePlugin {
  Future<String?> getPlatformVersion() {
    return TexturePluginPlatform.instance.getPlatformVersion();
  }

	Future<String?> createTexture() {
		return TexturePluginPlatform.instance.createTexture();
	}
}
