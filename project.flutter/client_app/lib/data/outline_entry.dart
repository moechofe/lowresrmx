
import 'package:flutter/material.dart';

class OutlineEntry {
	final String identifier;
	final int position;

	OutlineEntry(this.identifier, this.position);
}

class MyOutlineEntries extends ChangeNotifier {
	final List<OutlineEntry> _entries = [];

	List<OutlineEntry> get entries => _entries;
	set entries(List<OutlineEntry> entries) {
		_entries.clear();
		_entries.addAll(entries);
		notifyListeners();
	}
}
