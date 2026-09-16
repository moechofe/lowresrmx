import 'dart:ffi' as ffi;

import 'package:core_plugin/core_plugin.dart';
import 'package:core_plugin/core_plugin_bindings_generated.dart';

import 'package:lowresrmx/core/runtime.dart';
import 'package:lowresrmx/data/outline_entry.dart';

/// Tokenizes [source] with the engine (core_syntax.h) and returns every label and SUB declaration, in source order.
List<OutlineEntry> scanOutline(String source) {
  final ffi.Pointer<ffi.Void> syntax = syntaxCreate();
  if (syntax == ffi.nullptr) {
    return const [];
  }
  try {
    final int count = syntaxScan(syntax, source);
    if (count == 0) {
      return const [];
    }
    final ffi.Pointer<SyntaxSpan> spans = syntaxSpans(syntax);
    if (spans == ffi.nullptr) {
      return const [];
    }

    final List<OutlineEntry> entries = [];

    int byteOffset = 0;
    int unitOffset = 0;
    int row = 0;
    int rowStart = 0;

    for (int i = 0; i < count; ++i) {
      final SyntaxSpan span = spans[i];
      if (!span.isDeclaration) {
        continue;
      }
      final OutlineKind kind;
      if (span.kind == SyntaxKind.SyntaxLabel) {
        kind = OutlineKind.label;
      } else if (span.kind == SyntaxKind.SyntaxSub) {
        kind = OutlineKind.sub;
      } else {
        continue;
      }

      while (byteOffset < span.start && unitOffset < source.length) {
        final int unit = source.codeUnitAt(unitOffset);
        if (unit == 0x0A) {
          row += 1;
          rowStart = unitOffset + 1;
        }
        // UTF8: detect number of bytes used.
        if (unit < 0x80) {
          byteOffset += 1;
          unitOffset += 1;
        } else if (unit < 0x800) {
          byteOffset += 2;
          unitOffset += 1;
        } else if (unit >= 0xD800 &&
            unit <= 0xDBFF &&
            unitOffset + 1 < source.length &&
            source.codeUnitAt(unitOffset + 1) >= 0xDC00 &&
            source.codeUnitAt(unitOffset + 1) <= 0xDFFF) {
          byteOffset += 4;
          unitOffset += 2;
        } else {
          byteOffset += 3;
          unitOffset += 1;
        }
      }

      final int end = unitOffset + span.length;
      if (end > source.length) {
        continue;
      }
      entries.add(OutlineEntry(
          identifier: source.substring(unitOffset, end),
          kind: kind,
          location: Location(row, unitOffset - rowStart)));
    }
    return entries;
  } finally {
    syntaxDestroy(syntax);
  }
}
