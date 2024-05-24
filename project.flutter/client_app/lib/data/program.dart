import 'dart:io';

import 'package:path/path.dart' as p;

// TODO: pretty useless
class MyProgram {
  String name;

  MyProgram.fromFSE(FileSystemEntity entry)
      : name = p.basenameWithoutExtension(entry.path);
}
