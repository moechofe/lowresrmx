# OMG!<br>There is a tremendous amount of work to do!<br>Can I do a little bit of it today?

- [ ] web: Design score API and database.
- [ ] web: API to count play time.
- [ ] web: link to open in app

    `lowresnx://?name=2VoiceMusic&program=https%3A%2F%2Flowresnx.inutilis.com%2Fuploads%2F8khV2CXtCS_2VoiceMusic.nx&image=https%3A%2F%2Flowresnx.inutilis.com%2Fuploads%2F8khV2CXtCS_IMG_3098921E0F621.jpeg&topic_id=3694`

    name
    program
    image
    topic_id

- [x] web: redirect after publish
- [ ] web: handle error when publish fails, redirect to something.
- [x] web: apply the post text limit on javascript and report it to the user live.
- [x] web: should comment and show comments on published prg
- [x] web: should handle comment added.
- [ ] web: allow to play published prg.
- [ ] web: allow to favorite prg.
- [ ] web: allow to increase interest score, by visiting, playing, commenting, favoriting.
- [ ] web: served published png should use file and not redis
- [ ] web: Use rel=author on prg link
- [ ] web: Make sure all *.php match using $url['path']
- [x] web: Make sure "banned" is checked everywhere it's needed
- [ ] web: Allow to list user or author published prg
- [x] web: Make sure to not serve pages for non published prg, aka pid/program_id as it as comment and everything. pid.html should not work, but. What about the web-player?
- [x] web: stop storing img and prg in redis during publish
- [x] web: publish a shared prg
- [x] web: show a published prg
- [ ] web: provide a way for user to create their avatar using a prg
- [ ] web: provide a way to ban (and unlist), first entry, comment, user

- [ ] core: provide a way to change the offset where sound data are readed for patterns and tracks
- [x] core: fix history in overlay debugger when % was typed
- [ ] core: check how volume, mix, wave, pulse, length parameters are parse
- [x] core: Add filter to DIM command of the debugger
- [ ] core: `WAIT TAP` seems weird, check about VBL and RASTER interrupt
- [x] core: fix HEX$ with negative numbers
- [x] flutter: should update and render in the same scope
- [ ] core: when keyboard is hidden, message will stay in place, can I erase it?
- [ ] flutter/xcode: try to create a reverse luminance button for the top-left top-right buttons
- [x] flutter: first command seems to not be executed the first time
- [ ] core: can I prevent trace to draw on the message line?
- [x] core: remove `PARTICLE appearance DATA label`, prevent auto changing the sprite number
- [ ] windows: copy de dll https://stackoverflow.com/questions/13556885/how-to-change-the-executable-output-directory-for-win32-builds-in-cmake
- [x] overlay windows should use SAFE and update at resize
- [ ] core: can I use the last 80 bytes to share data across all ROM?
- [x] core: clean mess about ioRegisters and memory map
- [x] core: make =KEYBOARD return the size of the keyboard
- [x] core: see where =PAUSE is usefull, interrupt? no, deleteme
- [x] core: remove PAUSE ON/OFF but keep PAUSE
- [x] core: reuse PAUSE to provide a simple debugger
- [x] core: make sure to prevent pause inside interrupt
- [x] core: make print print newline if not engough space to print
- [x] xcode: prevent upper case on save.
- [ ] core: feature that force portrait on fantasy side, not device.
- [x] core: make `x` and `y` of `SCROLL` cmd optional
- [ ] particle_lib: I should return memory access error when machine_peek_short()
- [x] core: implement a 2x zoom per bg.
- [x] core: give access to cycle counter to user
- [ ] xcode: add a security to prevent user to upload non wanted content, generate a signature on the ios side and check it on the web, I found libsodium for swift. I'm using TTL content right now.

- [ ] web_app: https://vigneshwarar.substack.com/p/hackernews-ranking-algorithm-how

    rank = (P-1)/((T+2)^G)
    P = Points
    T = Age in hours
    G = 1.8

- [ ] xcode: Prevent to share without a thumbnail
- [ ] xcode: Remove `NSAllowsArbitraryLoads` and `NSAppTransportSecurity`, see: https://stackoverflow.com/questions/31254725/transport-security-has-blocked-a-cleartext-http
- [ ] flutter: Hide black screen at start..
- [ ] flutter: Show the search/replace bar above the code editor, not inside
- [ ] flutter: Make the search/replace shorter.
- [ ] flutter: Find a way to show line number, or longer line that wrap.
- [ ] Change Project name: Demake Remix, Retro Game Creator
- [ ] manual: Add `ON EMITTER CALL`
- [ ] core: Remove LET.
- [ ] core: redo the particle system, use more variable

    What is missing right now?

    1. The ability to make a particle designer
    2. ?

    3. Proposal

        I can keep the appearence stored in data, I like it.
        I should try to change the emitter data and use variables

            EMITTER eid [ON x,y] [DATA a,s,o,i,c,r,sx,sy,e,n,d,p]

        With `a`,`s`,`o`,`i`,`c`,`r`,`sx`,`sy`,`e`,`n`,`d` and `p` are all optional.

    4. Proposal

        If the library is simulating actually external chip, it can hold additional internal memory modifiable using commands.

    5. Map emitter paramater in memory, allowing to copy them

- [ ] flutter: Can I prevent user to hide the keyboard?

    No, but I can setup a special use case for when INPUT is used, has it actually block the game.

- [x] core: Add a function to test  the keyboard visibility
- [x] core: Remove pause feature. Or make it usefull for dev. Trace the "paused" message instead of in center of the screen.
- [-] flutter: enable ctrl/shift/insert... shortcut for cut copy past.

    Cannot be done, the mapping for shortcut to action has been implement in the wrong way, preventing to map multiple shortcut to the same action (intent)
    See: https://github.com/reqable/re-editor/blob/100e56708ff3dc93955e9e59c053aff944d7c193/lib/src/code_shortcuts.dart#L64

- [x] flutter: Input.key is not set on flutter side.
- [x] 🐞 I/O register not working: $FF84, 1 Byte, ASCII code of last pressed key
- [ ] flutter: Keyboard shortcut to run the app.
- [ ] flutter: Scroll to created program in library_page.
- [ ] flutter: Allow to change the size of the grid item in the library_page.
- [ ] flutter: Share to save to download.

    See: https://medium.com/@jigneshpatel24/share-file-in-flutter-image-pdf-video-document-multiple-files-e40b26251b0d

- [x] flutter: Update re_editor.
- [ ] flutter: No context menu available for the text editor?

    Re-Editor implements the control logic of the desktop context menu and the mobile long-press selection menu, but does not provide a default UI. Developers need to implement the SelectionToolbarController interface and setup it through toolbarController.

        CodeEditor(
            toolbarController: _MyToolbarController(),
        );

    The example show in re_editor repository do not work on Android but on linux.

    I probably need to implement a custom toolbar above the keyboard instead. I suspect showMenu to remove the focus on the editor and then, it selecte the whole line instead of the selection made by the user. Why did it act differently on Android and Linux?, I dunno.

- [ ] flutter: How to mesure performance and battery consumption?
- [x] core: Add a ceil function

    The function already exists, the entry in the manual is missing.

- [ ] flutter: Add an option to hide the run_page menu on Android.
- [ ] flutter: What about the system bar?
- [x] flutter: The keyboard should not resize the canvas but alter it's size.
- [x] flutter: The exit button on the run_page is not well placed.
- [x] 🐞 flutter: At 1st program run, the program is stuck and not get executed correctly.
- [ ] flutter: When no tools are available in the edit_drawer, simply display how to add tool.
- [ ] flutter/core: I should be able to check if the keyboard has been closed by the user and change the flag on the core side.

    https://pub.dev/packages/flutter_keyboard_visibility

- [ ] flutter: Restore previously pushed route.
- [x] flutter: Allow to open the app using a file.
- [x] flutter: Make MyProgramPreference independent beetween pages

    Because the routes are replacing each other, I can setup a Provider in edit page and retrieve it in the drawer or in the run page. I need a proper FutureBuilder for each one.

- [x] backend_core: I should remove KEYBOARD OPTIONAL

    User has no way to open the keyboard manually anyway.

- [ ] flutter: Keep an historic of changes.
- [ ] It seems a good idea to store the thumbnail inside the cartridge.
- [x] onSaveDataDisk should save on datadisk or edited
- [ ] finish the vscode-extension
- [x] 🐞 the flutter has editor has a number of characters limit, it crop at save.
- [ ] allow to remember the sorting option of the outline app wise, in the flutter app.
- [x] flutter: add undo/redo button
- [x] flutter: add custom symbols keyboard bar
- [ ] sdl: fix the dev menu when running the desktop version
- [x] delete fnc_BUTTON
- [x] redo the project architecture
- [x] rename file extension `.nx` to `.rmx`
- [ ] flutter: syntaxe highlight plugin for popular code editor
- [ ] add a text tool to the bg editor
- [x] add an interrupt for emitter
- [x] remove LET, REM
- [x] allow to login with google account
- [x] allow to upload program
- [x] allow to publish programs
- [ ] allow to list own programs
- [ ] allow to create posts from program
- [ ] allow to upload update version of a program
- [ ] allow to give a score to each post
- [ ] allow to list community posts
- [ ] Fix singleLine if with multiple statement per line, Check: `interpreter->isSingleLineIf`
- [ ] Trigo function should use 0..1 instead of 0..2PI
- [ ] Provide an export dedicated for web mobile, that handle ratio and open the window accordingly, test fullscreen, test input.
- [ ] use 2 bytes with 1/16 precision number for particle animation progress
- [ ] rename particle data and reserve a new keyword, one should talk about the character num and one for the palette num: CHAR and PALETTE
- [ ] provide another hard coded palette with system command to switch it
- [x] I removed CELL SIZE cmd, update the doc
- [x] remove the TokenDISPLAY and command
- [x] Remove GAMEPAD
- [x] Allow to update shown.w/h during runtime
- [x] protect unmapped memory access
- [x] try to make the 0nth color non transparent
- [x] expose message api to user
- [x] create smaller program picture
- [ ] there is a `view.endEditing(true)`, I should try to use it to fix the scrolling issue with the minimap
- [ ] flutter dev tool in --profile mode talk about pre-compiling shader, see: https://docs.flutter.dev/perf/shader
- [ ] implement google drive

        cd project.flutter/android
        ./gradlew signInReport

    Then copy the SHA-1 in the Google Cloud Console in Credentials

        flutter clean

    Sauce:
    - https://medium.com/codebrew/flutter-google-sign-in-without-firebase-3680713966fb
    - https://support.google.com/cloud/answer/6158849#installedapplications&android&zippy=%2Cnative-applications%2Candroid
    - https://developers.google.com/drive/api/guides/enable-sdk
    - https://medium.com/@anupama.pathirage/using-oauth-2-0-to-access-google-apis-1dbd01edea9a
    - https://stackoverflow.com/questions/68955545/flutter-how-to-backup-user-data-on-google-drive-like-whatsapp-does
    - https://pub.dev/packages/workmanager
    - https://pub.dev/packages/permission_handler
    - https://stackoverflow.com/questions/50561737/getting-permission-to-the-external-storage-file-provider-plugin
    - https://pub.dev/packages/shared_storage
    - https://pub.dev/packages/cloud_firestore
    - https://pub.dev/packages/dropbox_client

- [x] move the runtime in an isolate
- [ ] allow to show the keyboard
- [ ] hide the system navigation bar
- [x] show the trace and store in preference
- [ ] I should try to replace the library by the editor instead of pushing it
- [ ] Clear overlay when resized
- [ ] Allow to sync with icloud
- [ ] Prevent modify post that is too old.
- [x] Can I avoid uppercaseString of data_import?
- [x] Use PopScope on edit_page to return to library_page
- [ ] A tool to show the RETURN stack
- [x] A SAFE demo
- [x] Change default color
- [ ] delay asking for stars notation
- [ ] refresh session TTL as long as user visiting the website
- [ ] refresh google/discord oauth token
- [ ] enable `__Host-` session cookie: Sauce: https://datatracker.ietf.org/doc/html/draft-west-cookie-prefixes-05#section-3.2
- [ ] enable Secure session cookie
- [ ] redo the dev menu for the desktop version
- [ ] 🐞 when message get hidden, the first char of the trace is not displayed
- [ ] add `BG x1,y2 TO x2,y2 ATTR a` maybe `PRIO`, `FLIP` or `PAL` too.
- [ ] backup all .nx from the current app and make the new one that use .rmx
