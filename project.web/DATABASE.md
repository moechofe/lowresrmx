
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
    - _(str)_ `["pid"]` `PROGRAM_ID` program ID
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

- _(hash)_ `"f:ENTRY_TOKEN_ID:CID"` `CID`

    - _(str)_ `["uid"]` `USER_ID` author user ID
    - _(str)_ `["ct"]` ATOM timestamp for creation time
    - _(str)_ `["author"]` Author name at creation
    - _(str)_ `["text"]` content of the message
    - _(str)_ `["status"]` one of the following:
      - `null`
      - `"banned"`

- _(list)_ `"f:ENTRY_TOKEN_ID:c"` `CID` comment message entry

- _(list)_ `"u:USER_ID:f"` list of first entries ID

    - _(str)_ `[…]` `ENTRY_TOKEN_ID`

#### Forum related

- _(sortedset)_ `"w:WHERE_ID"` list the first message entries

    - _(str)_ `[…]` `ENTRY_TOKEN_ID`

    With score being the time when the post is created.

    With `WHERE_ID` can be: `"show"`, `"chat"`, `"help"`.

#### Logged user related

- _(int)_ `"seq:SERVICE"` sequence for login service token creation

    With `SERVICE` can be: `"google"`, `"discord"`, `"github"`.

- _(str)_ `"l:LOGIN_TOKEN_ID"` optionally an upload token **TTL**

- _(hash)_ `"s:SESSION_ID"` **TTL**

    - _(str)_ `["uid"]` `USER_ID` user id
    - _(str)_ `["status"]` one of the following:
      - `"allowed"`
      - `"revoked"` will remove this session
      - `"banned"` prevent to login with this session
    - _(str)_ `["ct"]` ATOM timestamp for creation time
    - _(str)_ `["at"]` ATOM timestamp for access time
    - _(str)_ `["csrf"]` CRSF token

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

- _(list)_ `"u:USER_ID:c"` list of comment entries ID

- _(list)_ `"u:USER_ID:a"` list of `AUTHOR_NAME`

<!-- TODO: implement
- _(str)_ `"a:AUTHOR_NAME"` `USER_ID`

    > Used to have a custom URL on the website must be unique. -->

#### Notification related

> Notifications are computed on read (pull model): a post is "unseen" for a user
> when its activity time `r:ENTRY_TOKEN_ID:d["at"]` is newer than the user's
> watermark `"u:USER_ID:t"`. Nothing is stored per notification, so unseen state
> is discarded automatically.

- _(zset)_ `"u:USER_ID:n"` posts the user is involved in (authored or commented) **TTL**

    - _(str)_ `[…]` `ENTRY_TOKEN_ID`

    With score being the unix time of the last involvement.
    Capped to the newest 50 members; TTL refreshed on each write.

- _(str)_ `"u:USER_ID:t"` unix timestamp the notifications were last checked

    Advanced to `now` whenever the user acts (publish, comment, upvote) or fetches
    `GET /notif`, so a user is never notified of their own activity.

<!-- TODO: design
#### Image related

- _(int)_ `"seq:image"` sequence for image token creation

- _(string)_ `"i:IMAGE_ID"` temporay uploaded image filename **TTL**

- _(list)_ `"f:ENTRY_TOKEN_ID:i"` `IMAGE_ID` list of used image filename in the post -->

#### Rank related

- _(zset)_ `"r:WHERE_ID"` list the first message entries

    - _(str)_ `[…]` `ENTRY_TOKEN_ID`

    With score being the computed score.

    With `WHERE_ID` can be: `"all"`, `"show"`, `"chat"`, `"help"`

- _(set)_ `"r:ENTRY_TOKEN_ID:v"` list of users that upvoted the entry

    - _(str)_ `[…]` `USER_ID`

- _(hash)_ `"r:ENTRY_TOKEN_ID:d` Details for score

    - _(int)_ `["pts"]` Computed points
    - _(int)_ `["vote"]` Cached upvotes
    - _(int)_ `["comm"]` Counter of comment posted
    - _(str)_ `["w"]` WHERE_ID of the forum
    - _(str)_ `["ct"]` ATOM timestamp for creation and replace time
    - _(str)_ `["ut"]` ATOM timestamp for update time
    - _(int)_ `["at"]` unix timestamp of last activity (comment or upvote), for notifications

#### Reaction related

> Emoji reactions are cosmetic: they never affect rank (`r:*`) or notifications.
> A `TARGET` is a post (`ENTRY_TOKEN_ID`) or a comment (`ENTRY_TOKEN_ID:CID`).

- _(set)_ `"x:TARGET"` index of emoji strings currently used on the target

    - _(str)_ `[…]` `EMOJI`

- _(set)_ `"x:TARGET:v:EMOJI"` users who reacted to TARGET with EMOJI

    - _(str)_ `[…]` `USER_ID`

    When the last user removes `EMOJI`, this set is emptied and `EMOJI` is
    removed from `"x:TARGET"`, so no orphan keys accumulate. The number of
    distinct emoji per target is capped at `MAX_REACTION_KINDS`.

#### Admin related

- _(list)_ `"adm:de"` list of users marked to delete everything

    - _(str)_ `USER_ID`
