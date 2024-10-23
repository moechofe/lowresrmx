# LowResRMX manual

> This is a fork of the excellent fantasy console Lowres NX by Timo "Inutilis" Kloss.<br>
> This is a heavily modified version and way more complicated to use, I recommend you to stay on the original app:
> [LowRes NX Coder on App Store](https://apps.apple.com/app/lowres-nx-coder/id1318884577).

<!-- FIXME: spell checked start -->

Create your games and play them on the go using an iPhone or iPad. LowResRMX is a virtual game console and game development environment that includes all the tools you need to start and finish a project.

It **will** includes a project manager, a code editor, and various tools to edit game assets: sprites, palettes, backgrounds, instruments, and music.

> **The documentation and included tools are still in development.**

## Difference with Lowres NX

The goal 🥅 of the app is to provide a development environment that I can use during my commutes on the subway, and later, to be able to publish a real app on the stores.

The original LowRes NX, despite being an excellent development environment, lacks some capabilities, in my opinion.

### Background

4 background layers instead of 2, with 64x64 cells instead of 32x32.

Support for large 16x16 pixel cells has been removed.

Removed 🗑️ syntaxes:

`CELL SIZE bg,size`

---

Alternative syntax for the `FLIP` command:

`FLIP x,y`

Where `x` and `y` are both optional and can accept any number. If the number is different from zero, the flag will be set.

### Screen

A fantasy screen with a resolution of 216x384 pixels.

It features perfectly square pixels, a fixed portrait orientation, and always fills the entire device screen.

Depending on the device's screen ratio, the number of visible virtual pixels may vary—from 216x288 on 4:3 devices to 177x384 on 19.5:9 devices.

To detect the visible virtual pixels, use the new functions `=SHOWN.W` and `=SHOWN.H`.

---

New functions to retrieve the visible fantasy display size:

`=SHOWN.W`<br>
`=SHOWN.H`

Returns the number of visible pixels in width and height respectively.

---

New functions to query the safe area offset:

`=SAFE.L`<br>
`=SAFE.T`<br>
`=SAFE.R`<br>
`=SAFE.B`

These return the offset in virtual pixels to apply from the boundary of the fantasy device screen to reach the inner safe area specified by the device's operating system.

These functions can be used to avoid the top camera notch or the bottom inset full-width buttons.

### Colors

The 64 available colors do not follow the original EGA style of LowRes NX. Instead, they use the [FAMICUBE palette created by Arne](https://lospec.com/palette-list/famicube).

> This makes using the color chooser from the original "Gfx Designer" in LowRes NX a bit more challenging.

---

New settings with the `SYSTEM` command:

`SYSTEM setting,opacity`
<br>where `setting` ranges from `1` to `4`.

This allows you to make color `0` of the background layer opaque instead of transparent.

| setting | background |
| ------- | ---------- |
|       1 |          0 |
|       2 |          1 |
|       3 |          2 |
|       4 |          3 |

### Sprites

170 sprites instead of 64.<br>
Sprite number range from `0` to `169`.

Positions support sub-pixels with `1/16` pixel precision.

e.g.:

    SPRITE 123,SPRITE.X(123)+0.25,

### Input

Touchscreen only, with floating-point precision.

The virtual controller and the ability to use a Bluetooth controller have been removed.

The virtual keyboard is still available.

When the device's virtual keyboard is hidden by the user, the corresponding I/O [Registers](#registers) flag is updated.

Removed 🗑️ syntaxes:

`=BUTTON(p,n)`, `=UP(p)`, `=DOWN(p)`, `=LEFT(p)`, `=RIGHT(p)`, `GAMEPAD n`, `TOUCHSCREEN`

---

Changes to the `=TOUCH.X` and `=TOUCH.Y` functions: These now return floating-point values.

---

New alternative to the `WAIT` command:

`WAIT TAP`

This will wait until a device tap is detected but will still render the screen.

---

New function to ask the device if the virtual keyboard is visible.

`=KEYBOARD()`

Return `-1` if the keyboard is visible.

### Flow control

New program flow control:

`ON value GOTO label,..`<br>
`ON value GOSUB label,..`<br>
`ON value RESTORE label,..`

These commands will `GOTO`, `GOSUB` or `RESTORE` to one of the listed `label`s based on the `value`.

e.g.:

	ON I GOTO ZERO,ONE,TWO

### Data

New keyword to skip read data:

`SKIP number`

This will skip a specified `number` of data entries, so they are not read.

e.g.:

	DATA 0,1,2
	SKIP 2
	READ A
	PRINT A
	'WILL PRINT "2"

---

It's now possible to access data directly using its label:

`PRINT label`

This allows you to store a string using the `DATA` keyword and reference it in the `PRINT` command.

e.g.:

	PRINT TEST
	TEST:
	DATA "GABUZOMEU"

It also works in variables, functions, and commands:

    A$=TEST+LEFT$(TEST,2)

Numeric values are supported as well:

    TEST2:
    DATA 123.4
    PRINT TEST2*3

### Math

New function to return the ceiling value of a numerical value:

`=CEIL(value)`

---

New function to clamp a numerical value:

`=CLAMP(value,min,max)`

This returns the `value` clamped between `min` and `max`.

---

New random generator and features:

`=RND`

Works the same as before but with a different generator, using [PCG](https://www.pcg-random.org/).

`RANDOMIZE seed,address`
`=RND address`
`=RND(max,address)`

You can specify an `address` where the internal state will be stored, consuming 16 bytes.

This allows you to use multiple generators at the same time.

---

New syntax to compute length:

`=LEN(x,y)`

Calculates the length of a 2D vector using `x` and `y` as the coordinates for the two axes.

---

New function that interpolate values between `0` and `1` using easing mathematical functions:

`=EASE(function,mode,value)`

This returns the interpolation of `value` between 0.0 and 1.0, using the specified easing `function` and `mode`.

|     | function |
| --- | -------- |
|   0 | linear   |
|   1 | sine     |
|   2 | quad     |
|   3 | cubic    |
|   4 | quart    |
|   5 | quint    |
|   6 | circ     |
|   7 | back     |
|   8 | elastic  |
|   9 | bounce   |

|     | mode  |
| --- | ----- |
|  -1 | in    |
|   0 | inout |
|  +1 | out   |

### Text

New command to expose the existing overlay message API:

`MESSAGE text`

### Other

New command to support programs from the original LowRes NX:

`COMPAT`

> Note: This command will be removed at some point. Some commands from LowRes NX are already unavailable.

Enables compatibility mode:

- Forces the rendering process to keep the original device screen.
- Reverts the `RND` command and `=RND()` function to their original behavior.

This does not guarantee full compatibility but can help with some aspects.

For instance, commands and functions that have been removed will still be unavailable.

Double cell support for background is not emulated.

---

Removed 🗑️ commands:

`DISPLAY`, `KEYBOARD OPTIONAL`, `GAMEPAD`, `BUTTON`, `LEFT`, `RIGHT`, `DOWN`, `UP`, `TOUCHSCREEN`

Removed 🗑️ reserved keywords:

`ANIM`, `CLOSE`, `DECLARE`, `DEF`, `FLASH`, `FN`, `FUNCTION`, `LBOUND`, `OPEN`, `OUTPUT`, `SHARED`, `STATIC`, `TEMPO`, `VOICE`, `WRITE`

### Memory

An almost compatible memory mapping.

The total addressable memory has increased from 64 Kibit to 128 Kibit.

Lowres NX:

| addr  | size      | purpose          |
| ----- | --------- | ---------------- |
| $0000 | 32 Kibit  | Cartridge ROM    |
| $8000 | 4 Kibit   | Character Data   |
| $9000 | 2 Kibit   | BG0 Data         |
| $9800 | 2 Kibit   | BG1 Data         |
| $A000 | 16 Kibit  | Working RAM      |
| $E000 | 4 Kibit   | Persistent RAM   |
| $FE00 | 256 Bytes | Sprite Registers |
| $FF00 | 32 Bytes  | Color Registers  |
| $FF20 |           | Video Registers  |
| $FF40 |           | Audio Registers  |
| $FF70 |           | I/O Registers    |

LowResRMX:

| addr   | size       | purpose          |
| ------ | ---------- | ---------------- |
| $0000  | 8 Kibit    | BG0 data         |
| $2000  | 8 Kibit    | BG1 data         |
| $4000  | 8 Kibit    | BG2 data         |
| $6000  | 8 Kibit    | BG3 data         |
| $8000  | 4 Kibit    | Character Data   |
| $9000  | 20 Kibit   | Working RAM      |
| $E000  | 6 Kibit    | Persisent RAM    |
| $FB00  | 1020 Bytes | Sprite registers |
| $FF00  | 32 Bytes   | Color registers  |
| $FF20  | 10 Bytes   | Video registers  |
| $FF40  | 48 Bytes   | Audio registers  |
| $FF70  | 40 Bytes   | I/O registers    |
| $FFA0  | 6 Bytes    | DMA registers    |
| $10000 | 64 Kibit   | Cartridge ROM    |

TODO: Add particle/emitter registers

---

New command and registers for fast memory copying:

`DMA COPY`<br>
`DMA COPY ROM`

Performs a fast memory copy that can only be executed during interrupt calls.

It uses the following registers as data:

| addr  | purpose             |
| ----- | ------------------- |
| $FFA0 | Source address      |
| $FFA2 | Bytes count         |
| $FFA4 | Destination address |

> This will only work in a subprogram that has been called during an interrupt, including `RASTER`, `VBL`, and `PARTICLE`.

---

The `COPY` command is now slightly faster.

### Particles

The particles library reuses sprites to make them appear, disappear, change their character data, and move them across the screen.

---

`PARTICLE first,count AT address`

Declare the use of `count` sprites starting from `first` and store the internal data at `address`.

Each sprite will consume 6 bytes of memory. For each sprite:

| addr | size    | purpose         |
| ---- | ------- | --------------- |
| +0   | 2 Bytes | Speed on x axis |
| +2   | 2 Bytes | Speed on y axis |
| +4   | 1 Byte  | Appearence      |
| +5   | 1 Byte  | Current frame   |

e.g.:

	PARTICLE 1,20 AT $9000
	'Will use sprite 1 to 20 included.
	'Store internal data from $9000 to $9078 excluded.

---

`PARTICLE appearance DATA label`

Declare a `label` that contains data for the particle `appearance`.

`appearance` is a number between `0` and `23`.

The associated data must contain a list of character numbers that will be used to draw the particle. Using a negative number will loop back to the previous data value.

A common example is to use zero `0` and minus one `-1` for the last two data values, which allows the particle to disappear until the sprite is reused.

e.g.:

	MY_APPEARANCE:
	DATA 1,2,3,4,0,-1
	'Will show the character 1 to 4, each per frame.
	'And loop with the character 0.

---

`PARTICLE OFF`

Clear all sprite positions and reset all particle appearances.

---

`ON PARTICLE CALL subprogram`

For each particle, the system will call the subprogram using an interrupt.

The target `subprogram` will receive two arguments:

1. The `sprite` number,
2. The particle `address`.

e.g.:

	SUB MYPART(SPR,ADDR)
		SY=PEEKW(ADDR+2)
		POKEW ADDR+2,SY-5
		'Change the speed in y axis
	END SUB

TODO: specify the number of cycles available.

---

`EMITTER count AT address`

Declare the use of `count` emitters and store the internal data at `address`.

`count` is a number between `0` and `15`.

Each emitter will consume 6 bytes of memory. For each emitter:

| addr | size    | purpose                 |
| ---- | ------- | ----------------------- |
| +0   | 2 Bytes | Position on x axis      |
| +2   | 2 Bytes | Position on y axis      |
| +4   | 1 Byte  | Delay before emit       |
| +5   | 1 Byte  | Number of times to emit |

<!-- FIXME: spell checked end -->

---

`EMITTER emitter DATA label`

Declare a `label` that contains DATA for the `emitter`.

The associated data must follow this list:
1. `appearance` index of `appearance`, see: `PARTICLE appearance DATA label`.
2. `shape` TODO: continue doc,
3. `outer` ring radius,
4. `inner` ring radius,
5. `arc` TODO: continue doc,
6. `rotation` TODO: continue doc,
7. initial particle `speed` on x axis,
8. initial particle `speed` on y axis,
9. `explosivity` TODO: continue doc,
10. `count` of the number of particle to spawn at this frame,
11. number of frame to `delay` the re-emittion,
12. how many times to `repeat` the emittion.

When a particle get spawn, a random position is computed between the `outer` and `inner` ring.

---

`EMITTER emitter ON x,y`

Tell the `emitter` to emit particle staring at position `x` and `y`.

It will automaticly end when `repeat` reach 0.

---

`EMITTER emitter OFF`

Manually stop the `emitter`.

# How does it works

<!--FIXME: spell checked NOW -->

LowResRMX does not have a framebuffer where users paint pixels. Instead, it simulates an NES/Gameboy PPU chip, which traverses its memory to generate output pixels based on characters, backgrounds, sprites, palettes and register information.

### Colors

LowResRMX provides the user with a fixed set of 64 colors.
The colors are grouped into sets of 4 called palette, with 8 palettes available for users to colorize their creations.

The 1st color of the 1st palette will be used as the background color for the whole program. For the following palettes (2nd to 8th), the 1st color is transparent.

> But this can be altered by the `SYSTEM` command.

From now on, the terms:
- **a palette** apply to one of the 8 palettes with 4 colors inside,
- **a color** means one of the 4 colors within a palette.

Check color [registers](#Registers) for information about the memory mapping and data format.

### Characters

The 256 characters available in the dedicated memory are 8x8 pixels each and are assigned a number from 0 to 255.
Whenever this number is encountered in the background or sprite, the rendering process will draw the corresponding block of 8x8 pixels.

From now on, the term:
- **a character** is a block of 8x8 pixels and take 16 bytes.

Check character [registers](#Registers) for information about the memory mapping and data format.

### Backgrounds

LowResRMX has 4 background layers, each with an independent:
- X and Y position,
- and 64x64 character cells.

The 4 layers are drawn in order from 4th to 1st. Cells of the 1st background will be visible above the others unless the priority flag is used.

Each cells have it's own:
- character number,
- palette,
- horizontal and vertical flip,
- priority to alter which layer's cell is drawn on top of another and on top of sprites.

Layers are not framebuffers; you CANNOT easily paint pixels directly. Instead, every cell references a character number, and those hold the pixel color information.

Each layer can be shifted using the position in the X and Y axes. When reaching the edges, the cells will wrap around and remain visible.
Put another way, when the rendering process tries to draw the 65th cell of a row, because it does not exist, it will draw the 1st cell instead. This can be used to achieve infinite scrolling.

As mentioned before, the 1st color of each palette is transparent except for the 1st color of the 1st palette. It will determine the background color of the whole fantasy screen.

> But this can be altered by the `SYSTEM` command.

From now on, the terms:
- **a bg** is a layer of 64x64 cells,
- **a cell** is a region of a bg that shows a character.

Check background [registers](#Registers) for information about the memory mapping and data format.

> [Backgrounds dedicated procedures and functions](#background-api)

### Sprites

LowResRMX offers 170 sprites, each with:
- and Y position,
- a size of 8x8, 16x16, 24x24 or 32x32 pixels,
- a character number for the top left one,
- a palette,
- horizontal and vertical flip,
- and priority to show the sprite on top of bg cells with priorities.

The 170 sprites are drawn in order from 170th (bottom) to 1st (top).

Sprites are groups of characters ranging from 1x1 to 4x4. The difference with bg is that you only choose the top-left character. The other character numbers are dependent on the top-left one.

Here's an explaination:

In memory, characters are grouped by 16 per row with 16 rows, called the spritesheet.

|          |          |     |      |      |
| -------- | -------- | --- | ---- | ---- |
| `00`     | `01`     | …   | `0e` | `0f` |
| **`10`** | **`11`** | …   | `1e` | `1f` |
| **`20`** | **`21`** | …   | `2e` | `2f` |
| `30`     | `31`     | …   | `3e` | `3f` |
|          |          | …   |      |      |

For a sprite with a size of 2x2 characters, and the top-left one is the number `10` in hexadecimal, the other characters will be the ones to its right and below it, so: `11`, `20`, and `21`.

A sprite can be freely placed anywhere on the screen with 1/16 sub-pixel precision. This means you can add 0.5 to its position even if there will be no change visually.

From now on, the terms:
- **a sprite** is a group of 1x1 to 4x4 character,
- **the spritesheet** is the whole 16x16 characters available.

<!--FIXME: spell checked END -->

# Program Language

The programming language follow the path of the original Lowres NX. It's a BASIC type language and here's how to use it.

### Program

A program is a list of so called instructions and executed by the fantasy console one by one from top to bottom. The simplest and iconic program could be:

    print "hello world!"

Users are invited to create a new program, types the instructions above and run it.

Keywords and values are separated by spaces and instructions with new line.

    print "hello..."
    print "second instruction"

> The program do not care about upper or lower case, use the one you prefers.


### Identifiers

An identifier is a word defined by the user that represent an idea inside the program and they are used to declare variables, labels or procedures (more on those terms later), e.g.: `hero`, `enemies`, `HP`, `score`, `spawn_monsters`, `update_screen`, `collect_coins`, `level12`…

Valid characters for identifiers art ASCII letters, digits and the underscore `_`. But they CANNOT start by a digits and they CANNOT have more than 21 characters.

> Important: Not all identifiers are valid because some of them are reserved by the language itself, check the [list of reserved keywords](#keywords) and learn them to avoid common mistakes

### Variables

A variables is a value that retains in the memory fantasy console as long as the program is running. They can be used to store positions, scores, HP and all sort of information that users can think of. The purpose of variable is to store a value inside to get it back later.

Example: think of a score counter that start at zero and increment every time a plumber hero jump on enemies heads. A nice name for this value can be `score` and can be declared like this:

    score=0

And to increment it:

    score=score+1

As you can see, variables are declared using an identifier, that why user should take care of giving them a nice name that help them remember what's inside.

As real example, this is a program that increment and print a score every time the player tap the device screen.

    score=0
    do
        print "score",score
        wait tap
        score=score+1
    loop

Variable can also store strings:

    name$="Untel"
    print name$

### Values

Identifiers can store two types of value: number and string.

Numbers can be integer or decimal, e.g.: `123`, `-45`, `0.01`, `12345.6789`

String can only contains ASCII7 characters: letters, digits, some punctuations and some control characters: `"hello WORLD! 123"`

> Learn more about the technical information about [number type](#number) and [string type](#string).

### Labels

Another feature that used identifiers are labels. They are used to mark a position in the program to go back to it later. User have to understand that programs are executed line by line from top to bottom and labels are one solution to go back to the top of the program or anywhere else (almost).

Here is an example that demonstrate how to recreate the program above using labels:

    score=0
    start:
        print "score",score
        wait tap
        score=score+1
    goto start

A label is declared by an identifier followed by a coma `:`.

### Procedures

The last usage of identifiers are procedures. They means to be reusable sub-part of a program.

Here's a quick example:

    sub addition(a,b,c)
        c=a+b
    end sub

By running this program, nothing will happens because procedures do not get executed automaticly. Instead the user must use a dedicated command for it:

    result=0
    call addition(120,3,result)
    print "result",result

By adding this piece of code after the previous one and run the program, the solution of the operation 120+3 will be printed on the screen: `RESULT 123`.

The advantage of _calling_ proceduces is to be able to do it again and again.

    call addition(45,-78,result)
    print "result",result

This time, it will print `RESULT -33`.

TODO: link to SUB/EXIT SUB/END SUB

TODO: link to CALL

### Parameters, arguments and scopes

An other concept that came with procedures are the parameters and the arguments but first, users have to learn as thing about variable's scope.

By default, when declaring a variable in a program is scope is limited and are unknown inside a procedure. e.g.:

    score=0
    sub print_score
        print "score",score
    end sub
    call print_score

This program will produce an error by running it: `variable not initialized`. The program can't access the variable `score` inside the `print_score` proceduce.

One option is to _pass_ the variable to the proceduce. Two changes must be made for that.

1. User need to add a parameter to the proceduce declaration:

        sub print_score(s)
            print "score",s
        end sub

2. User also need to add an argument to the proceduce call:

        call print_score(score)

When the proceduce `print_score` is called, the argument `score` is _passed_ to the proceduce inside the `s` argument.

The variable `s` inside the proceduce is called a _local_ variable and again, it is not accessible from the program out of the proceduce.

Other subject, parameters are passed by reference, it's means that a variable passed to a proceduce and being modified by this procedure will stay modified by the execute exit the proceduce.

Here's an example:

    sub modify_it(a)
        a=123
    end sub
    my_value=0
    call modify_it(my_value)
    print my_value

It will print `123`

### Array

Arrays are list of values grouped into one variable:

    dim player_score(1)
    player_score(0)=123
    player_score(1)=456

Arrays are zero-based indexed.

Arrays can also store strings:

    dim player_name$(1)
    player_name$(0)="Untel"
    player_name$(1)="John Doe"

TODO: Link DIM UBOUND

### Global scope

When variables are declared in the main program body, they are local to the program body.

When variables are declared inside a proceduc body, they are local to this procedure body.

It is possible to declare a variable global so it can be accessed in both scopes.

    global score
    sub frag
        score=score+1
    end sub
    do
        cls
        print "score:",score
        wait tap
        call frag
    loop

> They have to be declare in the main program before any procedure that use it is called.

The syntaxe to declare a global array is a bit different:

    dim global enemies

TODO: link GLOBAL

### Comments

Comments are piece of text that is not executed. They are usefull for taking notes of what a particular piece of code is doing.

    'get touch position in cells coordinates
    tx=touch.x\8

> Ever avoid Captain Obvious comment

### Grammar

As say earlier, a program is a list of instructions read and executed one by one. To make it easier to read, developer use a set of spaces, identations, new lines, comments, proceduces…

Here is a set of rules on how it works:

- an instruction cannot be split on two lines.

    This will not work:
    ```
    🐞a=
      123
    ```

- label are not instruction, user can place instruction on the same line.

    ✅This will work:
    ```
    test: print 123
    wait tap
    goto test
    ```

- the only way to squiz two instructions on one line is to use the `:` colon separator.

    ✅This will work:
    ```
    x=touch.x : y=touch.y
    ```

    > I recommand to place spaces before and after the `:` colon until I fix the parsing issue

- `IF...THEN` use a different syntax for on line instruction, and `:` colon as instruction separator will not work correctly.

    This will not work as intended:
    ```
    do
    x=0 : y=0
    if tap then x=touch.x : ❓y=touch.y
    print x,y
    wait tap
    loop
    ```
    `y=touch.y` will always be executed.

- Comment can be placed after the `:` colon instruction separator.

    ✅This will work:
    ```
    a=123 :'Default value
    ```

- Identation as no meaning, user can place them whatever they want.

    ✅This will work:
    ```
                print       "far"
    print "near"
    ```

- Check the [list of reserved keywords](#keywords), as user cannot use them for identifiers.

### Expression

TODO: continue

# BASIC instructions

TODO: continue

### Flow control

`END`

**Stop the execution of the program.**

It has the same effect as the execution reach the end of the program.

---

`IF/THEN/ELSE IF/ELSE/END IF`

**Conditionnaly execute instructions.**

Syntax:

    IF condition THEN
        instruction...
    [ELSE IF condition THEN
        instruction...]*
    [ELSE
        instruction]
    END IF

Will execute the list of `instructions...` if the above `condition` is true.
The `condition` must be an [expression](#expression).

It's possible to have multiple `ELSE IF` blocks but a maximum of one `ELSE` block is authorized.

Real example of a game that ask playre to guess a number:

    again: print "guess the number"
    answer=rnd(100)
    retry: input "1-100:";guess
    if guess<1 or guess>100 then
        print "a number between 1 and 100"
        goto retry
    else if guess=answer then
        print "you got it!"
        print
        goto again
    else if guess<answer then
        print "too low"
        goto retry
    else
        print "too high"
        goto retry
    end if

**Short syntax:**

    IF condition THEN instruction

This form only allow one instruction, but it's also shorter.

Real example of the same game with less lines:

    again: print "guess the number"
    answer=rnd(100)
    retry: input "1-100:";guess
    if guess=answer then
        print "you got it!"
        print
        goto again
    end if
    if guess<1 or guess>100 then print "a number between 1 and 100"
    if guess<answer then print "too low"
    if guess>answer then print "too high"
    goto retry

---

`GOTO`

**Jump to a labeled instruction.**

Syntax:

    GOTO label

With `label` being declared somewhere in the program.

Real example of a tool that flip a coin:

    again:
        print
        print "tap to flip a coin"
        wait tap
        coin=rnd(2)
        if coin=1 then goto heads
        if coin=2 then goto tails
        goto edge
    heads:
        print "heads"
        goto again
    tails:
        print "tails"
        goto again
    edge:
        print "edge of the coin!"
        goto again

---

`GOSUB/RETURN`

**Jump to a sub-routine.**

Expect to found a `RETURN` instruction to go back where it was right after the `GOSUB`.

The return location is store in a stack allow user to jump to sub-routine inside sub-routine. But the stack is limited. A good habit is to always have one `RETURN` for each `GOSUB`.

A common usage of sub-routine is to reuse a piece of code multiple times instead of rewrite it again.

> User should probably put all the sub-routine in one place near the end of the program to enshure they are not executed when not wanted.

Real example. A score is incremented using time and tap:

    score=0
    gosub update
    t=timer
    do
        if timer-t>30 then gosub increase
        if tap then gosub increase
        wait vbl
    loop
    increase:
        inc score
        t=timer
    update:
        text 2,2,"score:"+str$(score)
        return

This example is tricky, there is one `RETURN`, two labels and three `GOSUB`. The good habit explain above is broken here, but here is the explaination.

The important things is not the number of `RETURN` is the number of times the instruction is executed. User have to understand that one `GOSUB` execution will increase by one the size of the stack, and one execution of `RETURM` will reduce the same stack by exactly one. Every time the `WAIT VBL` instruction is reach, the stack size is empty, and that's is good sign. It means that the stack will not be overflowed.

TODO: Difference with SUB

---

`ON GOTO`

**Jump table.**


Jump to one of the listed label according to a value.

Syntax:

    ON value GOTO label0,label1,label2...

Will read the `value` and jump to:
- the first label if `value` equal `0` zero,
- the second label if `value` equal `1` one,
- ...

Real example:

    again:
        wait 30
        on rnd(1) goto zero,one
    zero:
        print "zero"
        goto again
    one:
        print "one"
        goto again

---

`SUB/EXIT SUB/END SUB`

**Defining a proceduce.**

Syntax:

    SUB name [(parameters...)]
        instruction...
    END SUB

This will define a procedure called `name` and can optionnaly received a list of values from the `parameters` list.

Syntax of the `parameters` list:

    (identifier[$][()], identifier[$][()]...)

Hm, it's a mess, let's explain this. A parameter is always a `identifier` and can be followed by the type of value it contains. The list of possibilities are:

- a number: `my_num`
- a string: `my_str$`
- an array of number: `my_num_array()`
- an array of string: `my_str_array$()`

An empty list of `parameters` is not valid. Simply remove the parenthesis `()`.

Proceduces are isolated small program that can be executed using the [`CALL` instruction](#CALL). Isolated because the variables declared inside it's body are local, learn more on [scopes](#Parameters, arguments and scopes). For comparison, the body of sub-routines reached using `GOSUB` are not isolated.

An other different with the sub-routines is that, the definitions of them are only executed throughout the [`CALL` instruction](#CALL), so, it is safe to place them anywhere inside the program.

Real example, a game where player need to enter a sequence of digits and computer try to prevent it:

    'perturb player input
    sub perturb
        if cursor.x<10 then print str$(rnd(9));
    end sub

    'handle player input
    sub handle(c$)
        a=asc(c$)
        if a=8 then call delete
        if a<48 or a>57 then exit sub
        if rnd<0.3 then call perturb
        if cursor.x<10 then print c$;
    end sub

    'delete last character
    sub delete
        if cursor.x=0 then exit sub
        locate cursor.x-1,cursor.y
        text cursor.x,cursor.y," "
    end sub

    'check if input is correct
    sub check(r$)
        r$="       "
        text 0,2,"       "
        if cursor.x<10 then exit sub
        ok=-1
        for i=0 to 9
            if cell.c(i,1)-208<>i then ok=0
        next i
        r$="failure"
        if ok then r$="success"
    end sub

    'main game
    print "write number form 0 to 9"
    result$="       "
    keyboard on
    randomize timer
    do
        k$=inkey$
        if k$<>"" then call handle(k$)
        wait vbl
        call check(result$)
        text 0,2,result$
    loop

> This example show proceduces without argument: `perturb`, `delete`, proceduce that take one input argument `handle` and one proceduce that will output a value in the parameter `check`

---

**Exiting early.**

Syntax:

    EXIT SUB

TODO: wait for next do loop repeat until while wend exit sub end sub call exit sub global dim

TODO: GOTO inside SUB

TODO: playing with GOSUB and RETURN

TODO: ON GOSUB

TODO: language instructions

TODO: about comments

TODO: on restore

# API instructions

## Background API

> [How backgrounds works](#backgrounds)

### Cell's attributes

`BG layer`

Change the current background to `layer` for further cells operations.

Wait `layer` a number from `0` to `3`.

    TEXT 10,10,"/"
    BG 1
    TEXT 10,10,"\"

---

`PAL p`

Change the current palette to `p` for further cells operations.

    PRINT "HELLO",
    PAL 1
    PRINT "WORLD"

---

`FLIP h,v`

Change the horizontal and vertical flip attributes to `h` and `v` respectively for further cells operations.

    FLIP 1,0
    PRINT "DLROW OLLEH"

---

`PRIO p`

Change the priority to `p` for further cells operations.

    SPRITE 0,78,78,1
    PRIO 1
    TEXT 10,10,"S"
    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    003C7E6A7E7E3C000000183C00000000

### Background's cell

`BG FILL x1,y1 TO x2,y2 CHAR c`

Fills all cells from `x1`,`y1` coordinates to `x2`,`y2` with character `c` and the [current attributes](#cells-attributes).

    BG FILL 1,1 to 5,5 char 1
    BG FILL 2,2 to 4,4 char 2
    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    FFFFFFFFFFFFFFFF0000000000000000
    0000000000000000FFFFFFFFFFFFFFFF

`CLS`

Clear all background layers with character zero `0`, resets the current window to the default and the backgrounds scrolling values.

`CLS layer`

Only clear the background numbered `layer` with character zero `0`. Do not alter the scrolling value.

---

`CELL cx,cy,c`

Sets the cell of the current background at `cx`,`cy` coordinates to the character `c` with the [current attributes](#cells-attributes).

E.g. draw a face:

    CELL 10,10,1
    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    003C7E6A7E7E3C000000183C00000000

> Use `BG`, `PAL`, `FLIP`, `PRIO` and `ATTR` to alter the [current attributes](#cells-attributes).

---

`=CELL.C(cx,cy)`

Return the character of the current background at `cx`,`cy` coordinates.

---

TODO: continue

### Cell attributes

### Background's layer

### Background's memory

`BG COPY sx,sy,sw,sh TO dx,dy`

Copies cell's informations (characters and attributes) from memory source to current background.

Copies a rectangle at `sx`,`sy` coordinates and `sw`,`sh` size from the source, and paste it to the background at `dx`,`dy`.

> By default the source data point to `=ROM(3)`, changes it using `BG SOURCE ROM(4)`.

    BG SOURCE ROM(4)
    BG COPY 0,0,4,1 to 4,4
    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    FFFFFFFFFFFFFFFF0000000000000000
    0000000000000000FFFFFFFFFFFFFFFF
    #4:TEST BG
    000004010100020001000200

---

`BG SOURCE a`<br>
`BG SOURCE a,w,h`

Specify the memory address `a` where source data will be used for background copy operations. Intended to be used jointly with `BG COPY sx,sy,sw,sh TO dx,dy`.

When width and height `w,h` are omitted, the first 4 bytes from memory are used to detect and store the `w` and `h` parameters. Actual data start at `a+4`.

| addr | purpose        |
| ---- | ---------------|
|  a+0 | always zero    |
|  a+1 | always zero    |
|  a+2 | width in cell  |
|  a+3 | height in cell |
|  a+4 | cell's data    |

When width and height `w,h` are provided, the actual data start at `a`.

> By default the source data point to `=ROM(3)`.

---

`MCELL cx,cy,c`

Similar to `CELL cx,cy,c` but modify the source in memory instead of the current background.

The source must point to writable memory, so it will not work with `BG SOURCE ROM(3)`.

TODO: example

TODO: continue

TODO: API

TODO: touch screen

TODO: about memory

TODO: about sound

TODO: about priority

TODO: about keywords

TODO: about numeric technical

TODO: how window not updated when on-resized

# Registers

Sprite Registers:

There are 170 sprites, each occupies 6 bytes:

| addr  | size    | purpose      |
| ----- | ------- | ------------ |
| $FB00 | 6 Bytes | 1st sprite   |
| $FB06 | 6 Bytes | 2nd sprite   |
| $FB0C | 6 Bytes | 3rd sprite   |
| …     |         | …            |
| $FEF6 | 6 Bytes | 170th sprite |
| $FEFC | 4 Bytes | Not used     |

For each sprite:

| offset | size    | purpose              |
| ------ | ------- | -------------------- |
| +0     | 2 Bytes | Position on X axis   |
| +2     | 2 Bytes | Position on Y axis   |
| +4     | 1 Byte  | Character number     |
| +5     | 1 Byte  | Character attributes |

Both position on x and y axis use sub-pixels values. To advance by 1 pixel, the values should get increased by 16.
Also, they are both offseted by 32 pixels. To place a sprite in the 0x0 coordinates, the values should be 512x512.

Attributes bits:

| bits | purpose         |
| ---- | --------------- |
| 0..2 | Palette number  |
| 3    | Horizontal flip |
| 4    | Vertical flip   |
| 5    | Priority        |
| 6..7 | Size            |

Sprite size:

| binary | purpose                        |
| ------ | ------------------------------ |
| %00    | 8x8 pixels or 1x1 character    |
| %01    | 16x16 pixels or 2x2 characters |
| %10    | 24x24 pixels or 3x3 characters |
| %11    | 32x32 pixels or 4x4 characters |

---

Color registers:

There are 8 palettes with 4 colors each:

| addr  | size     | purpose         |
| ----- | -------- | --------------- |
| $FF00 | 32 Bytes | Color registers |

For each palette:

| offset | size   | purpose         |
| ------ | ------ | --------------- |
| +0     | 1 Byte | 1st color value |
| +1     | 1 Byte | 2nd color value |
| +2     | 1 Byte | 3rd color value |
| +3     | 1 Byte | 4th color value |

---

Video registers:

| addr  | size     | purpose                     |
| ----- | -------- | --------------------------- |
| $FF20 | 2 Bytes  | Background layer 0 scroll X |
| $FF22 | 2 Bytes  | Background layer 0 scroll Y |
| $FF24 | 2 Bytes  | Background layer 1 scroll X |
| $FF26 | 2 Bytes  | Background layer 1 scroll Y |
| $FF28 | 2 Bytes  | Background layer 2 scroll X |
| $FF2A | 2 Bytes  | Background layer 2 scroll Y |
| $FF2C | 2 Bytes  | Background layer 3 scroll X |
| $FF2E | 2 Bytes  | Background layer 3 scroll Y |
| $FF30 | 2 Bytes  | Raster line number          |
| $FF32 | 1 Byte   | Display attributes          |
| $FF33 | 12 Bytes | Not used                    |

Display attributes:

| bits | purpose                    |
| ---- | -------------------------- |
| 0    | Sprites enabled            |
| 1    | Background layer 0 enabled |
| 2    | Background layer 1 enabled |
| 3    | Background layer 2 enabled |
| 4    | Background layer 3 enabled |

---

TODO: Audio registers:

| addr  | size     | purpose         |
| ----- | -------- | --------------- |
| $FF40 | 48 Bytes | Audio registers |

---

I/O registers:

| addr  | size    | purpose                        |
| ----- | ------- | ------------------------------ |
| $FF70 | 4 Bytes | Last touch position X          |
| $FF74 | 4 Bytes | Last touch position Y          |
| $FF78 | 2 Bytes | Pixels shown in width          |
| $FF7a | 2 Bytes | Pixels shown in height         |
| $FF7c | 2 Bytes | Pixels outside the safe zone   |
| $FF7e | 2 Bytes | Pixels outside the safe zone   |
| $FF80 | 2 Bytes | Pixels outside the safe zone   |
| $FF82 | 2 Bytes | Pixels outside the safe zone   |
| $FF84 | 1 Byte  | ASCII code of last pressed key |
| $FF85 | 1 Byte  | Other I/O status bits          |

Last touch position X and Y are stored as float and currently LowResRMX do not have a way to peek float from memory, use TOUCH.X and TOUCH.Y function instead.

Pixels shown represent the number of fantasy pixels that is visible by the user according to their device screen ratio. TODO: link See: SHOWN.W/H

Pixels outsied the safe zone represent the number of fantasy pixels that are visible but should be considered unsafe for touch input as they are outside the safe area. TODO: link See: SAFE.L/T/R/B

Other I/O status bits:

| bits | purpose                                   |
| ---- | ----------------------------------------- |
| 0    | Pause currently active                    |
| 1    | Fantasy screen currently touched          |
| 2    | Device virtual keyboard currently visible |

---

DMA registers:

| addr  | size    | purpose                 |
| ----- | ------- | ----------------------- |
| $FFA0 | 2 Bytes | Source address          |
| $FFA2 | 2 Bytes | Number of bytes to copy |
| $FFA4 | 2 Bytes | Destination address     |
