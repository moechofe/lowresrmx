import 'dart:convert';
import 'dart:ffi' as ffi;

import 'package:core_plugin/core_plugin.dart';
import 'package:core_plugin/core_plugin_bindings_generated.dart';

import 'package:lowresrmx/core/runtime.dart';
import 'package:lowresrmx/data/outline_entry.dart';
import 'package:lowresrmx/data/rom_entries.dart';

const String _markPrefix = "'''";

final RegExp _markKeyword = RegExp(r'^MARK[:\s]+(?=\S)', caseSensitive: false);

/// Returns every ''' marker comment of [source], in source order.
List<OutlineEntry> scanMarks(String source) {
  final List<OutlineEntry> marks = [];
  final List<String> lines = const LineSplitter().convert(source);
  for (int row = 0; row < lines.length; ++row) {
    final String line = lines[row];
    int column = 0;
    while (column < line.length &&
        (line.codeUnitAt(column) == 0x20 || line.codeUnitAt(column) == 0x09)) {
      column += 1;
    }
    if (!line.startsWith(_markPrefix, column)) {
      continue;
    }
    final String title = line
        .substring(column + _markPrefix.length)
        .trim()
        .replaceFirst(_markKeyword, '');
    if (title.isEmpty) {
      continue;
    }
    marks.add(OutlineEntry(
        identifier: title,
        kind: OutlineKind.mark,
        location: Location(row, column)));
  }
  return marks;
}

/// Returns every ROM entry header line of [source] (`#<id>:<comment>`), in
/// source order.
List<OutlineEntry> scanRomEntries(String source) {
  final List<OutlineEntry> entries = [];
  int row = 0;
  int rowStart = 0;
  for (final RegExpMatch header in romHeader.allMatches(source)) {
    for (int i = rowStart; i < header.start; ++i) {
      if (source.codeUnitAt(i) == 0x0A) row += 1;
    }
    rowStart = header.start;
    final int lineEnd = source.indexOf('\n', header.start);
    entries.add(OutlineEntry(
        identifier: source
            .substring(header.start, lineEnd < 0 ? source.length : lineEnd)
            .trimRight(),
        kind: OutlineKind.rom,
        location: Location(row, 0)));
  }
  return entries;
}

int _bySourcePosition(OutlineEntry a, OutlineEntry b) =>
    a.location.row != b.location.row
        ? a.location.row - b.location.row
        : a.location.column - b.location.column;

/// Tokenizes [source] with the engine (core_syntax.h) and returns every label and SUB
/// declaration, merged with every ''' marker comment and ROM entry header, in source order.
List<OutlineEntry> scanOutline(String source) {
  final List<OutlineEntry> entries = [
    ...scanMarks(source),
    ...scanRomEntries(source)
  ];
  final ffi.Pointer<ffi.Void> syntax = syntaxCreate();
  if (syntax == ffi.nullptr) {
    return entries..sort(_bySourcePosition);
  }
  try {
    final int count = syntaxScan(syntax, source);
    if (count == 0) {
      return entries..sort(_bySourcePosition);
    }
    final ffi.Pointer<SyntaxSpan> spans = syntaxSpans(syntax);
    if (spans == ffi.nullptr) {
      return entries..sort(_bySourcePosition);
    }

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
    entries.sort(_bySourcePosition);
    return entries;
  } finally {
    syntaxDestroy(syntax);
  }
}
