### 1.0_56

- Allow seed stored in PRAM to be saved when using:

        RANDOMIZE seed, address
        RND address
        RND(max, adress)

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

