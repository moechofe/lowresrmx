import 'dart:io';
import 'package:flutter/foundation.dart';
import 'package:image/image.dart' as img;
import 'dart:convert';

import 'package:zstandard/zstandard.dart';

import 'package:lowresrmx/data/library.dart';

final httpClient = HttpClient();

final scheme = "https";
final host = "ret.ro.it";

Future<String> downloadProgram(String pid) async {
  final request = await httpClient.getUrl(
      Uri(scheme: scheme, host: host, path: "/$pid${MyLibrary.codeExtension}"));
  final response = await request.close();
  final bytes = await consolidateHttpClientResponseBytes(response);
  return utf8.decode(bytes);
}

Future<img.Image?> downloadThumbnail(String pid) async {
  final request = await httpClient.getUrl(Uri(
      scheme: scheme, host: host, path: "/$pid${MyLibrary.thumbExtension}"));
  final response = await request.close();
  final bytes = await consolidateHttpClientResponseBytes(response);
  return img.decodeImage(bytes);
}

class RetroitUploadException implements Exception {
  final String code;
  const RetroitUploadException(this.code);
  @override
  String toString() => "RetroitUploadException($code)";
}

Future<Uri> shareProgram(String programName) async {
  final Uint8List program;
  try {
    program = await (await MyLibrary.getCodeFile(programName)).readAsBytes();
  } catch (_) {
    throw const RetroitUploadException("LPD");
  }

  final Uint8List thumbnail;
  try {
    thumbnail = await (await MyLibrary.getThumbFile(programName)).readAsBytes();
  } catch (_) {
    throw const RetroitUploadException("LID");
  }

  final Uint8List? compressed = await Zstandard().compress(program, 3);
  if (compressed == null) throw const RetroitUploadException("CPD");

  final String body = jsonEncode({
    "n": programName,
    "p": base64Encode(compressed),
    "i": base64Encode(thumbnail),
  });

  final HttpClientResponse response;
  final Uint8List bytes;
  try {
    final request = await httpClient
        .postUrl(Uri(scheme: scheme, host: host, path: "/upload"));
    request.headers.contentType = ContentType.json;
    request.write(body);
    response = await request.close().timeout(const Duration(seconds: 30));
    bytes = await consolidateHttpClientResponseBytes(response)
        .timeout(const Duration(seconds: 30));
  } catch (_) {
    throw const RetroitUploadException("NWE");
  }

  if (response.statusCode < 200 || response.statusCode > 299) {
    throw const RetroitUploadException("IVR");
  }
  if (bytes.isEmpty) throw const RetroitUploadException("NDA");

  final Object? token;
  try {
    token = jsonDecode(utf8.decode(bytes));
  } catch (_) {
    throw const RetroitUploadException("PJE");
  }
  if (token is! String || token.isEmpty) {
    throw const RetroitUploadException("IVT");
  }

  return Uri(
      scheme: scheme,
      host: host,
      path: "/share",
      queryParameters: {"uptoken": token});
}
