
## Setup DEV environment

> Tested with php8.3 and php8.4

1. Install for Ubuntu:

    ```bash
    sudo apt install nginx-full php8.3-cli php8.3-dev build-essential php8.3-mbstring libsodium-dev libzstd-dev redis-server redis-tools
    ```

2. Build and install [HIREDIS](https://github.com/redis/hiredis) from source:

    ```bash
    cd $HOME/Documents
    git clone --branch v1.3.0 https://github.com/redis/hiredis.git
    cd hiredis
    make clean && make && sudo make install
    ```

3. Build and install [Phpiredis](https://github.com/nrk/phpiredis) from source:

    ```bash
    cd $HOME/Documents
    git clone --branch v1.1 https://github.com/nrk/phpiredis.git
    cd phpiredis
    phpize && ./configure && make clean && make && sudo make install
    ```

4. Build and install [libsodium-php](https://github.com/jedisct1/libsodium-php) from source:

    ```bash
    cd $HOME/Documents
    git clone https://github.com/jedisct1/libsodium-php.git
    cd libsodium-php
    phpize && ./configure && make clean && make && sudo make install
    ```

5. Build and install [Zstd Extension for PHP](https://github.com/kjdev/php-ext-zstd) from source:

    ```bash
    cd $HOME/Documents
    git clone https://github.com/kjdev/php-ext-zstd.git
    cd php-ext-zstd
    phpize && ./configure --with-libzstd && make clean && make && sudo make install
    ```

6. Enable extensions for php cli on Ubuntu:

    ```bash
    cat /etc/php/8.3/cli/conf.d/30-lowresrmx.ini
    ```
    ```ini
    extension=mbstring.so
    extension=phpiredis.so
    extension=sodium.so
    extension=zstd.so
    ```

7. Check for loaded extensions:

    ```bash
    php -i | grep mbstring
    php -i | grep redis
    php -i | grep sodium
    php -i | grep zstd
    ```

Create and fill the `sources/private.php` file using the given `private.sample.php`

8. Start the server:

    ```bash
    cd sources
    php -S 0.0.0.0:8080 index.php
    ```

9. Add to your `/etc/hosts`:

    ```hosts
    127.0.0.1 lowresrmx.top
    ```

## Redis Database

#### Uploaded program related

> Uploaded program are private.

- _(int)_ `"seq:uptoken"` sequence for upload token creation

- _(hash)_ `"t:UPLOAD_TOKEN_ID"` temporay uploaded program **TTL**

    - _(str)_ `["prg"]` Zstandard compressed source for program
    - _(str)_ `["img"]` Thumbnail PNG binary
    - _(str)_ `["name"]` Program name

- _(hash)_ `"p:PROGRAM_ID"` uploaded program

    - _(str)_ `["uid"]` `USER_ID` author user ID
    - _(str)_ `["prg"]` Zstandard compressed source for program
    - _(str)_ `["img"]` Thumbnail PNG binary
    - _(str)_ `["name"]` Name of the program at upload
    - _(str)_ `["ct"]` ATOM timestamp for creation time
    - _(str)_ `["author"]` Author name at creation
    - _(str)_ `["first"]` `ENTRY_TOKEN_ID` of posted program.

- _(list)_ `"u:USER_ID:p"` list of uploaded program

    - _(str)_ `[…]` `PROGRAM_ID`

#### Published messages related

> Published program is public.

- _(int)_ `"seq:entry"` sequence for message entry

- _(hash)_ `"f:ENTRY_TOKEN_ID":f` first message entry

    - _(str)_ `["uid"]` `USER_ID` author user ID
    - _(str)_ `["title"]` Title of the message (or the program)
    - _(str)_ `["text"]` content of the message
    - _(str)_ `["ut"]` ATOM timestamp for update time
    - _(str)_ `["author"]` Author name at creation
    - _(str)_ `["status"]?` one of the following:
      - `null`
      - `"unlisted"`
      - `"banned"`
    - _(str)_ `["name"]` Name of the program at upload

- _(int)_ `"f:ENTRY_TOKEN_ID:s"` sequence for comment id `CID`

- _(hash)_ `"f:ENTRY_TOKEN_ID:CID` `CID`

    - _(str)_ `["uid"]` `USER_ID` author user ID
    - _(str)_ `["ct"]` ATOM timestamp for creation time
    - _(str)_ `["author"]` Author name at creation
    - _(str)_ `["text"]` content of the message
    - _(str)_ `["status"]` one of the following:
      - `null`
      - `"banned"`

- _(list)_ `"f:ENTRY_TOKEN_ID:c"` `CID` comment message entry

- _(list)_ `"u:USER_ID:f"` list of first entries ID

    - _(str)_ `[…]` `PROGRAM_ID`

#### Forum related

- _(sortedset)_ `"w:WHERE_ID"` list the first message entries

    With score being the time when the post is created.

    With `WHERE_ID` can be: `"show"`, `"chat"`, `"help"`.

#### Logged user related

- _(int)_ `"seq:SERVICE"` sequence for login service token creation

    With `SERVICE` can be: `"google"`, `"discord"`.

- _(str)_ `"l:LOGIN_TOKEN_ID"` optionally an upload token **TTL**

- _(hash)_ `"s:SESSION_ID"` **TTL**

    - _(str)_ `["uid"]` `USER_ID` user id
    - _(str)_ `["status"]` one of the following:
      - `"allowed"`
      - `"revoked"`
      - `"banned"`
    - _(str)_ `["ct"]` ATOM timestamp for creation time
    - _(str)_ `["at"]` ATOM timestamp for access time

- _(list)_ `"u:USER_ID:s"` list allowed session

    > TODO: What is the purpose of this?
    > I think it was to revoke access from a administration console or something. See: revokeSession()

    - _(str)_ `[…]` `SESSION_ID`

#### User data related

- _(hash)_ `"u:USER_ID"` public profile

    - _(str)_ `["name"]`
    - _(str)_ `["picture"]` URL of a public picture
    - _(str)_ `["author"]` current `AUTHOR_NAME`
    - _(str)_ `["locale"]` ISO 639-1:2002

- _(sortedset)_ `"u:USER_ID:c"` list of comment entries ID

    with score is the ATOM timestamp of the creation time

- _(list)_ `"u:USER_ID:a"` list of `AUTHOR_NAME`

- _(str)_ `"a:AUTHOR_NAME"` `USER_ID`

    > Used to have a custom URL on the website must be unique.

#### Rank related

- _(zset)_ `"r:WHERE_ID` `ENTRY_TOKEN_ID` list the first message entries

    With score being the computed score.

    With `WHERE_ID` can be: `"all"`, `"show"`, `"chat"`, `"help"`

- _(set)_ `"r:ENTRY_TOKEN_ID:v"` list of users that upvoted the entry

    - _(str)_ `[…]` `USER_ID`

- _(hash)_ `"r:ENTRY_TOKEN_ID:d` Details for score

    - _(int)_ `["pts"]` Computed points
    - _(int)_ `["vote"]` Cached upvotes
    - _(int)_ `["comm"]` Counter of comment posted
    _ _(str)_ `["w"]` WHERE_ID of the forum
    - _(str)_ `["ct"]` ATOM timestamp for creation time
    - _(str)_ `["ut"]` ATOM timestamp for update time

        > TODO: `["ut"]` not used, is it usefull?

