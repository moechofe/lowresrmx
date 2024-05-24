import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'texture_plugin_method_channel.dart';

abstract class TexturePluginPlatform extends PlatformInterface {
  /// Constructs a TexturePluginPlatform.
  TexturePluginPlatform() : super(token: _token);

  static final Object _token = Object();

  static TexturePluginPlatform _instance = MethodChannelTexturePlugin();

  /// The default instance of [TexturePluginPlatform] to use.
  ///
  /// Defaults to [MethodChannelTexturePlugin].
  static TexturePluginPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [TexturePluginPlatform] when
  /// they register themselves.
  static set instance(TexturePluginPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }

	Future<String?> createTexture() {
		throw UnimplementedError('createTexture() has not been implemented.');
	}
}
