import 'package:lowresrmx/core/runtime.dart';

enum OutlineKind {
  label,
  sub,
  mark,
}

/// One label, SUB declaration or ''' marker, as listed by the outline drawer.
class OutlineEntry {
  final String identifier;
  final OutlineKind kind;
  final Location location;

  OutlineEntry(
      {required this.identifier, required this.kind, required this.location});
}
