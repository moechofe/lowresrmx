### 1.0_59

- Fix rendering with COMPAT mode.

    LowRes NX modified editor are back.

- Reuse the pulse width to filter the noise waveform:

    - A width of 7 is the unfiltered.
    - Lower values apply a lowpass.
    - Higher values apply a highpass.
    - The LFO width amount now sweeps the noise filter.
    - The LFO invert flag chooses the side of the sweep.
    - All noise is 5.2 dB quieter, to make every width equally loud.

- Replace the sawtooth waveform by a sine waveform:

- Reuse the pulse width to add intervals to the sine waveform:

    - A width of 7 is the bare sine.
    - Lower values add one or two partials under the played note.
    - Higher values add one or two partials over the played note.
    - The LFO width amount now steps through the intervals.
    - The LFO invert flag chooses the direction of the steps.

### 1.0_58

- Add a zoomable screenshot to the About page.

- Add a table of contents for the website documentation.

- Prevent SDL from changing the player title HTML tag.

- Add two links to the program and the thumbnail.

- Add Markdown backtick parsing to allow monospace font and unparsed Markdown.

- Add Markdown triple backtick parsing to allow code blocks.

- Cache variables (and arrays) with tokens to reduce lookup impact on CPU.

- Fix missing 4.1 dB in all generated sounds.

- Change how pulse width affects the square waveform:

    - Remove symmetry.
    - Prevent 0% duty.
    - Prevent extreme values.
    - A pulse width of 15 is the new square wave.

- Use pulse width to shape the triangle waveform:

    - A width of 7 is an exact triangle wave.
    - The LFO width amount now modulates the triangle as well.

- GFX: Remove flashing selected color border.

- GFX: Re-enable energy saving mode.

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
