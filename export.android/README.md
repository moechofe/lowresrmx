
# Rename the package name

    1. Pickup a value for application-id/package-name, it has to be unique but I can't know until you'll try to publish your app on Google Play Store.

    2. Edit `android-project/app/build.gradle`:

        Change `android.namespace` and `android.defaultConfig.applicationId` values. E.g. `lowresrmx.author_name.game_name`.

    3. Browse `android-project/app/src/main/java`:

        Rename the directories to match the application-id. E.g.: `lowresrmx/author_name/game_name`.

    4. Edit `android-project/app/src/main/res/values/strings.xml`:

        Change `resources.string` value with the visible name of your app. E.g.: "Super Game".





Generate one big file with backend.core and frontend.sdl:

    bash tool.dev/tool.dev/merge_for_android_export.bash

Download the android release and place it into `app/libs/SDL3-x.y.z.aar`

Replace file name `dependencies.implementation` in `app/build.gradle`

