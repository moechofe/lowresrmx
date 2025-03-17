- [ ] core: make =KEYBOARD return the size of the keyboard
- [ ] core: see where =PAUSE is usefull, interrupt? no, deleteme
- [ ] core: remove PAUSE ON/OFF but keep PAUSE
- [ ] core: reuse PAUSE to provide a simple debugger
- [ ] core: make sure to prevent pause inside interrupt
- [x] core: make print print newline if not engough space to print
- [x] ios_app: prevent upper case on save.
- [ ] core: feature that force portrait on fantasy side, not device.
- [ ] core: make `x` and `y` of `SCROLL` cmd optional
- [ ] particle_lib: I should return memory access error when machine_peek_short()
- [x] core: implement a 2x zoom per bg.
- [ ] core: give access to cycle counter to user
- [ ] ios_app: add a security to prevent user to upload non wanted content, generate a signature on the ios side and check it on the web, I found libsodium for swift. I'm using TTL content right now.

- [ ] web_app: https://vigneshwarar.substack.com/p/hackernews-ranking-algorithm-how

    rank = (P-1)/((T+2)^2)
    P = Points
    T = Age in hours
    G = 1.8

- [ ] ios_app: Prevent to share without a thumbnail
- [ ] ios_app: Remove `NSAllowsArbitraryLoads` and `NSAppTransportSecurity`, see: https://stackoverflow.com/questions/31254725/transport-security-has-blocked-a-cleartext-http
- [ ] flutter_app: Hide black screen at start..
- [ ] flutter_app: Show the search/replace bar above the code editor, not inside
- [ ] flutter_app: Make the search/replace shorter.
- [ ] flutter_app: Find a way to show line number, or longer line that wrap.
- [ ] core: show more info in trace, like variables counter, label counter...
- [ ] Change Project name: Demake Remix
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

- [ ] flutter_app: Can I prevent user to hide the keyboard?

    No, but I can setup a special use case for when INPUT is used, has it actually block the game.

- [x] core: Add a function to test  the keyboard visibility
- [ ] core: Remove pause feature. Or make it usefull for dev. Trace the "paused" message instead of in center of the screen.
- [-] flutter_app: enable ctrl/shift/insert... shortcut for cut copy past.

    Cannot be done, the mapping for shortcut to action has been implement in the wrong way, preventing to map multiple shortcut to the same action (intent)
    See: https://github.com/reqable/re-editor/blob/100e56708ff3dc93955e9e59c053aff944d7c193/lib/src/code_shortcuts.dart#L64

- [x] flutter_app: Input.key is not set on flutter side.
- [x] 🐞 I/O register not working: $FF84, 1 Byte, ASCII code of last pressed key
- [ ] flutter_app: Keyboard shortcut to run the app.
- [ ] flutter_app: Scroll to created program in library_page.
- [ ] flutter_app: Allow to change the size of the grid item in the library_page.
- [ ] flutter_app: Share to save to download.

    See: https://medium.com/@jigneshpatel24/share-file-in-flutter-image-pdf-video-document-multiple-files-e40b26251b0d

- [x] flutter_app: Update re_editor.
- [-] flutter_app: No context menu available for the text editor?

    Re-Editor implements the control logic of the desktop context menu and the mobile long-press selection menu, but does not provide a default UI. Developers need to implement the SelectionToolbarController interface and setup it through toolbarController.

        CodeEditor(
            toolbarController: _MyToolbarController(),
        );

    The example show in re_editor repository do not work on Android but on linux.

    I probably need to implement a custom toolbar above the keyboard instead. I suspect showMenu to remove the focus on the editor and then, it selecte the whole line instead of the selection made by the user. Why did it act differently on Android and Linux?, I dunno.

- [ ] flutter_app: How to mesure performance and battery consumption?
- [ ] core: Add a ceil function

    The function already exists, the entry in the manual is missing.

- [ ] client_app: Add an option to hide the run_page menu on Android.
- [ ] client_app: What about the system bar?
- [x] client_app: The keyboard should not resize the canvas but alter it's size.
- [x] client_app: The exit button on the run_page is not well placed.
- [ ] 🐞 client_app: At 1st program run, the program is stuck and not get executed correctly.
- [ ] client_app: When no tools are available in the edit_drawer, simply display how to add tool.
- [ ] client_app/core: I should be able to check if the keyboard has been closed by the user and change the flag on the core side.

    https://pub.dev/packages/flutter_keyboard_visibility

- [ ] client_app: Restore previously pushed route.
- [x] client_app: Allow to open the app using a file.
- [x] client_app: Make MyProgramPreference independent beetween pages

    Because the routes are replacing each other, I can setup a Provider in edit page and retrieve it in the drawer or in the run page. I need a proper FutureBuilder for each one.

- [x] backend_core: I should remove KEYBOARD OPTIONAL

    User has no way to open the keyboard manually anyway.

- [ ] client_app: Keep an historic of changes.
- [ ] It seems a good idea to store the thumbnail inside the cartridge.
- [x] onSaveDataDisk should save on datadisk or edited
- [ ] finish the vscode-extension
- [x] 🐞 the flutter has editor has a number of characters limit, it crop at save.
- [ ] allow to remember the sorting option of the outline app wise, in the flutter app.
- [ ] add undo/redo button
- [ ] add custom symbols keyboard bar
- [ ] fix the dev menu when running the desktop version
- [x] delete fnc_BUTTON
- [ ] move pcg to a directory
- [x] redo the project architecture
- [x] rename file extension `.nx` to `.rmx`
- [ ] plugin for popular code editor
- [ ] add a text tool to the bg editor
- [ ] stoping clear overlay on pause
- [ ] run and loop an interrupt during pause, and allow to unpause
- [ ] allow multiple instructions per line
- [x] add an interrupt for emitter
- [x] remove LET, REM
- [ ] allow to create line shape with emitter or rect
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
- [-] Auto color transition
- [ ] Provide an export dedicated for web mobile, that handle ratio and open the window accordingly, test fullscreen, test input.
- [-] try this easing func:

	1/(1+c*((1/x)-1)^k)

	and it's derivated

	(c*k*((1/x)-1)^k-1) / ((c*x*((1/x)-1)^k+x)^2)

- [ ] use 2 bytes with 1/16 precision number for particle animation progress
- [ ] rename particle data and reserve a new keyword, one should talk about the character num and one for the palette num: CHAR and PALETTE
- [ ] provide another hard coded palette with system command to switch it
- [x] I removed CELL SIZE cmd, update the doc
- [x] remove the TokenDISPLAY and command
- [x] Remove GAMEPAD
- [x] Allow to update shown.w/h during runtime
- [ ] protect unmapped memory access
- [x] try to make the 0nth color non transparent
- [x] expose message api to user
- [x] create smaller program picture
- [ ] there is a `view.endEditing(true)`, I should try to use it to fix the scrolling issue with the minimap
- [ ] flutter dev tool in --profile mode talk about pre-compiling shader, see: https://docs.flutter.dev/perf/shader
- [ ] implement google drive
- [x] move the runtime in an isolate
- [ ] allow to show the keyboard
- [ ] hide the system navigation bar
- [x] show the trace and store in preference
- [ ] I should try to replace the library by the editor instead of pushing it
- [ ] Clear overlay when resized
- [ ] Allow to sync with icloud
- [ ] Prevent modify post that is too old.
- [ ] Can I avoid uppercaseString of data_import?
- [x] Use PopScope on edit_page to return to library_page
- [ ] A tool to show the RETURN stack
- [ ] A SAFE demo
- [ ] Change default color
- [ ] TRACE windows should use SAFE and update at resize
- [ ] delay asking for stars notation
- [ ] refresh session TTL as long as user visiting the website
- [ ] refresh google/discord oauth token
- [ ] enable __Host- session cookie: Sauce: https://datatracker.ietf.org/doc/html/draft-west-cookie-prefixes-05#section-3.2
- [ ] enable Secure session cookie
- [ ] redo the dev menu for the desktop version
- [ ] 🐞 when message get hidden, the first char of the trace is not displayed
- [ ] add `BG x1,y2 TO x2,y2 ATTR a` maybe `PRIO`, `FLIP` or `PAL` too.
- [ ] backup all .nx from the current app and make the new one that use .rmx
