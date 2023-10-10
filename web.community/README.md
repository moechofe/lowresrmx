
Install `php8.1-cli` `php8.1-dev` `build-essential`

Install https://github.com/redis/hiredis

Install https://github.com/nrk/phpiredis

Enable the `phpiredis` for php cli

Install libsodium-dev

Install https://github.com/jedisct1/libsodium-php

Install https://github.com/kjdev/php-ext-zstd

    cd sources
    php -S localhost:8080

Sauce: https://silverhammermba.github.io/password/

## Database:

- _(int)_ `"seq:token"` sequence for upload token creation

- _(hash)_ `"t:UPLOAD_TOKEN_ID"` temporay uploaded program

    - _(string)_ `["uid"]` `USER_ID` author user ID
    - _(string)_ `["prg"]` Zstandard compressed source for program
    - _(string)_ `["img"]` PNG compressed source for image

- _(hash)_ `"p:PROGRAM_ID"` shared program

    - _(string)_ `["uid"]` `USER_ID` author user ID
    - _(string)_ `["prg"]` Zstandard compressed source for program
    - _(string)_ `["img"]` PNG compressed source for image
    - _(string)_ `["ct"]` ATOM timestamp for creation time
    - _(string)_ `["author"]` Author name at creation

- _(list)_ `"u:USER_ID:p"` list of shared program

    - _(string)_ `[…]` `PROGRAM_ID`

- _(int)_ `"seq:google"` sequence for google login token creation

- _(string)_ `"l:LOGIN_TOKEN_ID"` nothing

- _(hash)_ `"s:SESSION_ID"`

    - _(string)_ `["uid"]` `USER_ID` user id
    - _(string)_ `["status"]` one of the following:
      - `"allowed"`
      - `"revoked"`
    - _(string)_ `["ct"]` ATOM timestamp for creation time
    - _(string)_ `["at"]` ATOM timestamp for access time

- _(list)_ `"u:USER_ID:s"` list allowed session

    - _(string)_ `[…]` `SESSION_ID`

- _(hash)_ `"u:USER_ID"` public profile

    - _(string)_ `["name"]`
    - _(string)_ `["picture"]` URL of a public picture

- _(hash)_ `"u:USER_ID:s"` user settings

    - _(string)_ `["locale"]` ISO 639-1:2002
