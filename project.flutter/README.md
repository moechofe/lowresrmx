## Setup for Ubuntu 24.04.3 LTS

1. Install Java `sudo adb install openjdk-21-jre-headless`
1. Install [Android Studio](https://developer.android.com/studio).
2. Install `Android SDK Build-Tools`, `Android SDK Command-line Tools`, `CMake`, `Android SDK Platforms-Tools` using the `SDK Manager`.
3. Install [Flutter](https://docs.flutter.dev/get-started/install).

    > Last version tested: Flutter version 3.35.1 & Dart version 3.9.0

4. Install adb: `sudo apt install google-android-platform-tools-installer`

### Run on Linux using [Visual Studio Code](https://code.visualstudio.com/) and [Flutter extension](https://marketplace.visualstudio.com/items?itemName=Dart-Code.flutter)

1. Open the Command Palette and choose `Debug: Select and Start Debugging` ⇒ `Dart & Flutter…` ⇒ `client_app (Flutter Linux) LowResRMX`.

### Run on Linux using a terminal

```bash
cd project.flutter/client_app
flutter run -dlinux
```

### Run on a real Android device using [Visual Studio Code](https://code.visualstudio.com/) and [Flutter extension](https://marketplace.visualstudio.com/items?itemName=Dart-Code.flutter)

1. Enable [Developer mode](https://www.android.com/intl/en_uk/articles/enable-android-developer-settings/).
2. Connect the device using USB cables.
3. Check for connected devices: `adb devices`
4. Open the Command Palette and choose `Debug: Select and Start Debugging` ⇒ `Dart & Flutter…` ⇒ `client_app (Flutter YOUR_DEVICE_HERE) LowResRMX`.

### Run on a real Android device in release mode using a terminal

```bash
cd project.flutter/client_app
flutter run --release -d$(adb devices|head -2|tail -1|awk '{print $1}')
```

