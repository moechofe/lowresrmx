import 'dart:io';
import 'package:flutter/foundation.dart';
import 'package:image/image.dart' as img;
import 'dart:convert';

import 'package:lowresrmx/data/library.dart';

final httpClient = HttpClient();

final scheme="https";
final host="ret.ro.it";

Future<String> downloadProgram(String pid) async {
	final request = await httpClient.getUrl(Uri(scheme: scheme, host: host, path: "/$pid${MyLibrary.codeExtension}"));
	final response = await request.close();
	final bytes = await consolidateHttpClientResponseBytes(response);
	return utf8.decode(bytes);
}

Future<img.Image?> downloadThumbnail(String pid) async {
	final request = await httpClient.getUrl(Uri(scheme: scheme, host: host, path: "/$pid${MyLibrary.thumbExtension}"));
	final response = await request.close();
	final bytes = await consolidateHttpClientResponseBytes(response);
	return img.decodeImage(bytes);
}
