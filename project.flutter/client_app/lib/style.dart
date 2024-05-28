import 'package:flutter/material.dart';

const Map<String, TextStyle> styleLight = {
  'root': TextStyle(color: Color.fromARGB(255, 0, 0, 0)),
  'keyword': TextStyle(color: Color.fromARGB(255, 197, 32, 109)),
  'number': TextStyle(color: Color.fromARGB(255, 1, 84, 139)),
  'string': TextStyle(color: Color.fromARGB(255, 42, 49, 87)),
  'comment': TextStyle(color: Color.fromARGB(255, 100, 100, 104)),
};

const Map<String, TextStyle> styleDark = {
  'root': TextStyle(color: Color(0xffffffff)),
  'keyword': TextStyle(color: Color.fromARGB(255, 233, 136, 92)),
  'number': TextStyle(color: Color.fromARGB(255, 247, 72, 72)),
  'string': TextStyle(color: Color.fromARGB(255, 228, 238, 86)),
  'comment': TextStyle(color: Color.fromARGB(255, 223, 109, 206)),
};

const TextStyle libraryItemTextStyle = TextStyle(
	fontFamily: 'Roboto',
	fontWeight: FontWeight.w400,
	fontSize: 18,
);

