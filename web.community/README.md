
Install `php8.1-cli` `php8.1-dev` `build-essential` `php8.1-mbstring`

Install https://github.com/redis/hiredis

Install https://github.com/nrk/phpiredis

Enable the `phpiredis` for php cli

Install libsodium-dev

Install https://github.com/jedisct1/libsodium-php

Install libzstd-dev

Install https://github.com/kjdev/php-ext-zstd

    cd sources
    php -S 0.0.0.0:8080 index.php

Sauce: https://silverhammermba.github.io/password/

#### How to simulate a share from the app

This method only works when `lowresrmx.top` point to `localhost`

Navigate to: `http://lowresrmx.top:8080/upload?p=QkcgU09VUkNFIFJPTSg0KQpCRyBDT1BZIDAsMCw0LDEgdG8gNCw0CiMyOk1BSU4gQ0hBUkFDVEVSUwowMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAwMApGRkZGRkZGRkZGRkZGRkZGMDAwMDAwMDAwMDAwMDAwMAowMDAwMDAwMDAwMDAwMDAwRkZGRkZGRkZGRkZGRkZGRgojNDpURVNUIEJHCjAwMDAwNDAxMDEwMDAyMDAwMTAwMDIwMAoK
&t=iVBORw0KGgoAAAANSUhEUgAAAB4AAAAfCAYAAADwbH0HAAAACXBIWXMAAA7EAAAOxAGVKw4bAAAD/ElEQVRIDcVWTUhUURQ+EwUxC2uhiZYu1MxSx1BzkYFERkVTkATVInRRy2jTpqIWBkVgRCm0SFAxqDYtwigqEMshMJUcx58cJmIsB1PChNyEvOY7753bfW+ezkwtOvDm3nvuOfe73znn3jser9dL/0PWJgE1rHlPvJU+VPoY/bRlTTKP2LvLMGHQsfv5Ym5ofdGl1SZjTBlb/fRz3k+L4R4a6emkU9uWGGD683JaQE7jpMBvbx+nCn8TfQ91UyS8THsqCngN9P9FkgIDoMJC6B/55MD6+8J0yzHyKR8BDGwRWmH78KOX8FmibEWRSutxHCejseU07a2poWgsRpHpaeq60J2wDmx06X/9niIvJqFKucJ1YBuovjA2oAuAdNlTv4vSBbflWJjKok5A6AFw7+x+NomNv+K22bERVib5EcbMtjAvT5m7gWLyVsM+yixoUnZ+3xLV7vBS+0IJ61INuSousF1NZB6gT/sCyrQn6KXA+BIh3OmIAi4v9VF+Tg51dTxhf7DHh9DqkQDDo3W1CkMYQ1FWXkzNjy+hyzcdOiuJCjUcAIwNOGV0LKhUvQMDVDLYTbmlJTQzxpVMk9VmlWODQ4Mhqqouo6snrouPa6Wr4lopp+ItLULeGx8wZPWf8OqgiFrhQZVzsE8AV4yft5+XtVdtZ4uLEuZx5kXAODQ6JUNu3QpOgGFgDL5so4zMXDbenLuFW/x8nfnC/cX5GZqLhrkvG0BqkIp0wVWosRoW3e5LrG7ZBFpsDBugqTABHKAAF2DJMdYT5lLxceYq7Dpj2BpTw2ZVCxiUIjpz6LDRTccaGBxjJ7hsAu3G7A1y/XK+ncDwN5Dvwso69BOE2Wra0HqDGTvDrZlwV3Iv+XYDhiFCQmDvZD4RHMAUs5U8Ywy29d8eUF8gSnW1+XQzuhNqJThiODnWo+Ox5diyYlC8QMWVDQzuZAm7Q2fuUOebVnYBKM7tupPEoLvPRenZRAsNBT9YS5rNwuwPNVY3l6UxZDG5rSLDfTyFopKKF++srGzuyh0ApgDFpgFa5TNZM9P42ZZig5MOrEDhiFsMLZhBwNrJfG5ulgAubzbCDMGmAQpwgOpM2SD+owOzTioTR0RYy9kVJxkDFOCQG1aY0QeYzhgVjXtcCgs2NmCAStgwebj+ABoWgMmndHFQ2SheKGEsEdBzrIcZ/jZgAYXj3Y52dT4FaLUWbzLAReSRkKjpbGGjA3sAWPTL/JuDvicS4BBKnmVRjBFazMtmAYq3GYK51rYj3Me8FQHbQ6GfYwMOzgXw/DVemeRF9J+ua+azePGRrjX7MqetZQOFlQ3YsQSM+Uxbet05Fb0sp/uJjvQLxM3ATQfndPUKUDq/ARtS+sQgtazXAAAAAElFTkSuQmCC&n=infdev`

## Database:


#### Shared program related

> A shared program (with it's thumbnail) is a user's program stored in the database and linked to it's author by it's user id.
> That doesn't means it has been published.

- _(int)_ `"seq:uptoken"` sequence for upload token creation

- _(hash)_ `"t:UPLOAD_TOKEN_ID"` temporay uploaded program **TTL**

    <!-- - _(string)_ `["uid"]` `USER_ID` author user ID -->
    - _(string)_ `["prg"]` Zstandard compressed source for program
    - _(string)_ `["img"]` Zstandard compressed source for image
    - _(string)_ `["name"]` Program name

- _(hash)_ `"p:PROGRAM_ID"` shared program

    - _(string)_ `["uid"]` `USER_ID` author user ID
    - _(string)_ `["prg"]` Zstandard compressed source for program
    - _(string)_ `["img"]` Zstandard compressed source for thumbnail
    - _(string)_ `["name"]` Name of the program at upload
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
    - _(string)_ `["prg"]` Zstandard compressed source for program
    - _(string)_ `["img"]` Zstandard compressed source for thumbnail

- _(hash)_ `"e:ENTRY_TOKEN_ID` following message entries

    - _(string)_ `["fid"]` `ENTRY_TOKEN_ID` first message entry ID
    - _(string)_ `["uid"]` `USER_ID` author user ID
    - _(string)_ `["ct"]` ATOM timestamp for creation time
    - _(string)_ `["author"]` Author name at creation
    - _(string)_ `["content"]` content of the message
    - _(string)_ `["prg"]` Zstandard compressed source for program
    - _(string)_ `["img"]` PNG compressed source for thumbnail

> _ _(sortedset)_ `"f:ENTRY_TOKEN_ID:e"` list of following post entries
>
>      with score is the ATOM timestamp of the creation time

- _(sortedset)_ `"w:WHERE_ID"` list the first message entries


#### Logged user related

- _(list)_ `"u:USER_ID:s"` list allowed session

    > TODO: What is the purpose of this?
    > I think it was to revoke access from a administration console or something. See: revokeSession()

    - _(string)_ `[…]` `SESSION_ID`

- _(int)_ `"seq:google"` sequence for google login token creation

- _(string)_ `"l:LOGIN_TOKEN_ID"` optionally an upload token **TTL**

- _(hash)_ `"s:SESSION_ID"` **TTL**

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

- _(hash)_ `"u:USER_ID:g"` user settings

    - _(string)_ `["locale"]` ISO 639-1:2002

- _(list)_ `"u:USER_ID:f"` list of first post entries ID

    with score is the ATOM timestamp of the creation time

- _(sortedset)_ `"u:USER_ID:e"` list of post entries ID

    with score is the ATOM timestamp of the creation time

- _(list)_ `"u:USER_ID:a"` list of `AUTHOR_NAME`


#### User author name related

- _(string)_ `"a:AUTHOR_NAME"` `USER_ID`

    > Used to have a custom URL on the website must be unique.


