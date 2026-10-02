import 'package:lowresrmx/core/runtime.dart';

enum OutlineKind {
  label,
  sub,
  mark,
  rom,
}

/// One label, SUB declaration, ''' marker or ROM entry header, as listed by the outline drawer.
class OutlineEntry {
  final String identifier;
  final OutlineKind kind;
  final Location location;

  OutlineEntry(
      {required this.identifier, required this.kind, required this.location});
}
