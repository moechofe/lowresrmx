
Install `php8.1-cli` `php8.1-dev` `build-essential` `php8.1-mbstring`

Install https://github.com/redis/hiredis

Install https://github.com/nrk/phpiredis

Enable the `phpiredis` for php cli

Install libsodium-dev

Install https://github.com/jedisct1/libsodium-php

Install libzstd-dev

Install https://github.com/kjdev/php-ext-zstd

    cd sources
    php -S localhost:8080 index.php

Sauce: https://silverhammermba.github.io/password/

## Database:


#### Shared program related

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

#### Posted messages related

- _(int)_ `"seq:entry"` sequence for message entry

- _(hash)_ `"f:ENTRY_TOKEN_ID"` first message entry

    - _(string)_ `["uid"]` `USER_ID` author user ID
    - _(string)_ `["title"]` Title of the message (or the program)
    - _(string)_ `["text"]` content of the message
    - _(string)_ `["ct"]` ATOM timestamp for creation time
    - _(string)_ `["author"]` Author name at creation

- _(hash)_ `"e:ENTRY_TOKEN_ID` following message entries

    - _(string)_ `["fid"]` `ENTRY_TOKEN_ID` first message entry ID
    - _(string)_ `["uid"]` `USER_ID` author user ID
    - _(string)_ `["ct"]` ATOM timestamp for creation time
    - _(string)_ `["author"]` Author name at creation
    - _(string)_ `["content"]` content of the message

> _ _(sortedset)_ `"f:ENTRY_TOKEN_ID:e"` list of following post entries
>
>      with score is the ATOM timestamp of the creation time

- _(sortedset)_ `"w:WHERE_ID"` list the first message entries


#### Logged user related

- _(list)_ `"u:USER_ID:s"` list allowed session

    - _(string)_ `[…]` `SESSION_ID`

- _(int)_ `"seq:google"` sequence for google login token creation

- _(string)_ `"l:LOGIN_TOKEN_ID"` nothing

- _(hash)_ `"s:SESSION_ID"`

    - _(string)_ `["uid"]` `USER_ID` user id
    - _(string)_ `["status"]` one of the following:
      - `"allowed"`
      - `"revoked"`
    - _(string)_ `["ct"]` ATOM timestamp for creation time
    - _(string)_ `["at"]` ATOM timestamp for access time

#### User data related

- _(hash)_ `"u:USER_ID"` public profile

    - _(string)_ `["name"]`
    - _(string)_ `["picture"]` URL of a public picture
    - _(string)_ `["author"]` current `AUTHOR_NAME`

- _(hash)_ `"u:USER_ID:s"` user settings

    - _(string)_ `["locale"]` ISO 639-1:2002

- _(list)_ `"u:USER_ID:f"` list of first post entries ID

    with score is the ATOM timestamp of the creation time

- _(sortedset)_ `"u:USER_ID:e"` list of post entries ID

    with score is the ATOM timestamp of the creation time

- _(list)_ `"u:USER_ID:a"` list of `AUTHOR_NAME`

#### User author name relaned

- _(string)_ `"a:AUTHOR_NAME"` `USER_ID`


