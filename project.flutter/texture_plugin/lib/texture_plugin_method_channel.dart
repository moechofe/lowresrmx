import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'texture_plugin_platform_interface.dart';

/// An implementation of [TexturePluginPlatform] that uses method channels.
class MethodChannelTexturePlugin extends TexturePluginPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('texture_plugin');

  @override
  Future<String?> getPlatformVersion() async {
    final version = await methodChannel.invokeMethod<String>('getPlatformVersion');
    return version;
  }

	@override
	Future<String?> createTexture() async {
		return await methodChannel.invokeMethod<String>('createTexture');
	}
}
