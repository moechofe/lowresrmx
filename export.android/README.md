
# Rename the application-id

    1. Pickup a value for **application-id** (or package-name), it has to be unique but you can't know if it's available until you'll try to publish your app on Google Play Store.

        `tld.author_domain.game_name`

    2. Edit `android-project/app/build.gradle`:

        Change `android.namespace` and `android.defaultConfig.applicationId` values with your **application-id**.

    3. Edit `android-project/app/src/main/java/lowresrmx/author_name/game_name/MyActivity.java`:

        Change `package` value by your **application-id**.

    3. Browse `android-project/app/src/main/java`:

        Rename the directories hierarchy to match the **application-id**, each dots replaced by slashes. E.g.: `tld/author_domain/game_name`.

# Change the loaded program

    1. Copy your program into `android-project/app/src/main/assets/app.rmx`.

# Change the logo image, application name and splash screen color

    1. Edit `android-project/app/src/main/res/values/strings.xml`:

        Change `resources.string` value with the visible name of your app. E.g.: "Super Game".

    2. Find an icon generator online, e.g.: https://icon.kitchen/

    3. Download, uncompress and browse to `IconKitchen-Output/android/res/`

        Replace all the folders that start by `mipmap-...` to `android-project/app/src/main/res`

    4. Edit `android-project/app/src/main/res/values/colors.xml`

        Change `resources.color` value with your color.

# Generate APK

    1. Build >> Generate Signed Bundle or APK >> Android App Bundle

        And follow https://developer.android.com/studio/publish/app-signing?utm_source=android-studio-app&utm_medium=app#generate-key

    2. locate the bundle in `app/release`
