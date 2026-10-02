/// A ROM entry header: `#<id>:` at the start of a line, id in group 1.
final RegExp romHeader = RegExp(r'^#(\d+):', multiLine: true);

/// ROM entries last copied from the edit drawer. They live as long as the app,
/// so they can be pasted into another program.
Map<int, String> romClipboard = const {};

/// Every `#id:` block of [source] in text order: the id (-1 when it does not
/// fit an int) and the text from the header up to the next header, trailing
/// whitespace removed.
List<(int, String)> _romBlocks(String source) {
  final List<RegExpMatch> headers = romHeader.allMatches(source).toList();
  return [
    for (int i = 0; i < headers.length; i++)
      (
        int.tryParse(headers[i].group(1)!) ?? -1,
        source
            .substring(headers[i].start,
                i + 1 < headers.length ? headers[i + 1].start : source.length)
            .trimRight()
      )
  ];
}

/// The ROM entries of an .rmx source, by id (0..15). A repeated id keeps the
/// first one, like the engine.
Map<int, String> parseRomEntries(String source) {
  final Map<int, String> entries = {};
  for (final (int id, String text) in _romBlocks(source)) {
    if (id >= 0 && id <= 15) entries.putIfAbsent(id, () => text);
  }
  return entries;
}

/// Clipboard text for [ids]: entries in ascending id order, separated by one
/// blank line, ending with a newline — the layout `data_export` writes.
String romEntriesText(Map<int, String> entries, Iterable<int> ids) {
  final List<int> sorted = ids.where(entries.containsKey).toList()..sort();
  return "${sorted.map((id) => entries[id]!).join("\n\n")}\n";
}

/// [source] with the entries of [clip] listed in [ids] pasted in. An entry the
/// source already has is replaced in place. A missing one is inserted before
/// the first entry with a higher id, or at the end. The code before the ROM
/// section is kept verbatim, and entries end up separated by one blank line.
String pasteRomEntries(
    String source, Map<int, String> clip, Iterable<int> ids) {
  final Set<int> pasting = ids.where(clip.containsKey).toSet();
  if (pasting.isEmpty) return source;
  final int? romStart = romHeader.firstMatch(source)?.start;
  final String code = romStart != null
      ? source.substring(0, romStart)
      : source.trim().isEmpty
          ? ""
          : "${source.trimRight()}\n\n";
  final List<(int, String)> blocks = _romBlocks(source);
  final Set<int> replaced = {};
  for (int i = 0; i < blocks.length; i++) {
    final int id = blocks[i].$1;
    if (pasting.contains(id) && replaced.add(id)) blocks[i] = (id, clip[id]!);
  }
  for (final int id in pasting.difference(replaced).toList()..sort()) {
    final int at = blocks.indexWhere((block) => block.$1 > id);
    blocks.insert(at < 0 ? blocks.length : at, (id, clip[id]!));
  }
  return "$code${blocks.map((block) => block.$2).join("\n\n")}\n";
}
