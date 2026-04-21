import 'dart:io';
import 'package:flutter/foundation.dart';
import 'package:image/image.dart' as img;
import 'dart:convert';

final httpClient = HttpClient();

final scheme="https";
final host="ret.ro.it";

Future<String> downloadProgram(String pid) async {
	final request = await httpClient.getUrl(Uri(scheme: scheme, host: host, path: "/$pid.rmx"));
	final response = await request.close();
	final bytes = await consolidateHttpClientResponseBytes(response);
	return utf8.decode(bytes);
}

Future<img.Image?> downloadThumbnail(String pid) async {
	final request = await httpClient.getUrl(Uri(scheme: scheme, host: host, path: "/$pid.png"));
	final response = await request.close();
	final bytes = await consolidateHttpClientResponseBytes(response);
	return img.decodeImage(bytes);
}
