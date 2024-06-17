import 'package:flutter/foundation.dart';

import 'package:lowresrmx/core/runtime.dart';

/// Used to transport errors from the runtime to the gutter of the code editor.
class MyContinousLocation extends ChangeNotifier {
	Location location = Location(-1, -1);

	void setLocation(Location location) {
		this.location = location;
		notifyListeners();
	}
}

