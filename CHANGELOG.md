### 1.0_58

- Add zoomable screenshot to the about page.

- Add table-of-content for the website documentation.

- Prevent SDL to change the player title HTML tag.

- Add 2 links to the program and the thumbnail.

- Add markdown backtick parsing to allow monospace font and unparsed markdown.

- Add markdown triple backtick parsing to allow block of code.

- Fix missing 4.1 dB in all generated sound.

- Change how pulse width affect the square waveforme

    Remove symetry.
    Prevent 0% duty.
    Prevent extreme values.
    A pulse width of 15 is the new square wave.

- Use the pulse width to shape the triangle waveform

    A width of 7 is an exact triangle wave.
    The LFO width amount now modulates the triangle too.

- GFX: Remove flashing selected color border.

- GFX: Re enable energy saving mode.

### 1.0_57

- Allow seed stored in PRAM to be saved when using:

        RANDOMIZE seed, address
        RND address
        RND(max, address)

- Website now anonymously tracks website activity: page views per type and API calls.

    1. counters are stored in redis,
    2. a prometheus node exporter gathers them,
    3. and they are shown in grafana.

- Fix a typo in an error message: "needed" -> "seeded".

- Prevent rejected promises from being logged in the browser console when the user is not signed in.

- Prevent robots from crawling `share.html` and `setting.html`.

- Fix discord embedded "play" and "thumbnail" links.

- Allow `/replace` API call to update the creation date after a week, allowing the post to rise in rank again.

- Allow `/replace` API call to trigger a notification for subscribed users.

- Replace `clang-format` with `uncrustify` for the `c` backend and frontend.

- Fix typo for `BG TINT` in documentation.

- Add a "beta" frieze on Apple Store link.
