import 'package:flutter_test/flutter_test.dart';
import 'package:texture_plugin/texture_plugin.dart';
import 'package:texture_plugin/texture_plugin_platform_interface.dart';
import 'package:texture_plugin/texture_plugin_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockTexturePluginPlatform
    with MockPlatformInterfaceMixin
    implements TexturePluginPlatform {

  @override
  Future<String?> getPlatformVersion() => Future.value('42');

	@override
	Future<String?> createTexture() {
		// TODO: implement createTexture
		throw UnimplementedError();
	}
}

void main() {
  final TexturePluginPlatform initialPlatform = TexturePluginPlatform.instance;

  test('$MethodChannelTexturePlugin is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelTexturePlugin>());
  });

  test('getPlatformVersion', () async {
    TexturePlugin texturePlugin = TexturePlugin();
    MockTexturePluginPlatform fakePlatform = MockTexturePluginPlatform();
    TexturePluginPlatform.instance = fakePlatform;

    expect(await texturePlugin.getPlatformVersion(), '42');
  });
}
