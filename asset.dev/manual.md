TODO: need technical info about string, no? check ### Literals
TODO: how window not updated when on-resized
TODO: list of API instructions
TODO: default ROM FILE entries
TODO: check all TODO

TODO: check for link reach
TODO: check for doubled link
TODO: ``` or \[tab\] ?

# Retro Game Creator - Documentation

> This is a fork of the excellent fantasy console Lowres NX by Timo "Inutilis" Kloss.<br>
> This is a heavily modified version and way more complicated to use, I recommend you to stay on the original app:<br>
> 🌐[LowRes NX Coder on App Store](https://apps.apple.com/app/lowres-nx-coder/id1318884577).

## What is LowResRMX?

Create your games and play them on the go using an iPhone or iPad. LowResRMX is a virtual game console and game development environment that includes all the tools you need to start and finish a project.

It includes a project manager, a code editor, and various tools to edit game assets: sprites, palettes, backgrounds, instruments, and music.

LowResRMX simulates a virtual retro game console directly inside your device, allowing user to create, play, and share their games.

It tries to emulate old-school 8-bits/16-bits console hardware by copying their limitations. Low number of colors, limited memory, and slow processing power.

## Difference

The goal of the app is to provide a development environment that I can use during my commutes on the subway, and later, to be able to publish a real app on the stores.

The original LowRes NX, despite being an excellent development environment, lacks some capabilities, in my opinion to be used on a handled device.

> If you already know how to use Lowres NX, you can read the following list of differences below:<br>
> For new users, I recommend to [jump to the: How does it work section](#how-does-it-work)

**Background:**

- [4 background layers](#backgrounds) instead of 2, with 64x64 cells instead of 32x32.

- Support for large 16x16 pixel cells has been removed.

    Removed syntaxes: <del>`CELL SIZE`</del>.

- The [`FLIP` command take optional argument](#flip-horizontalvertical) now.

- New settings with the `SYSTEM` command to [render a layer at double size](#system-settingvalue):

- Internal scroll offset not capped to 512 any more, but 0xFFFF.

**Screen:**

- A new [fantasy screen](#screen) with a maximum resolution of 216x384 pixels.

- New functions to [retrieve the visible fantasy display size](#widthshownwheightshownh)

- New functions to [query the safe area offset](#leftsafeltopsafetrightsaferbottomsafeb):

- Removed commands and functions: <del>`DISPLAY`</del>.

**Colors:**

- The 64 available colors [do not follow the original EGA style](#colors) of LowRes NX.

    > This makes using the color chooser from the original "Gfx Designer" in LowRes NX a bit more challenging.

- New settings with the `SYSTEM` command to make the [color `0` for a layer opaque](#system-settingvalue) instead of transparent:

**Sprites:**

- 170 sprites instead of 64. Sprite number range from `0` to `169`.

- Positions support sub-pixels with `1/16` pixel precision.

        sprite 123,sprite.x(123)+0.25,

**Input/Output:**

- [Touch coordinates return floating-point](#xtouchxytouchy) values instead of integers.

- The virtual controller and the ability to use a Bluetooth controller have been removed.

    Removed gamepad related functions: <del>`=BUTTON(p,n)`</del>, <del>`=UP(p)`</del>, <del>`=DOWN(p)`</del>, <del>`=LEFT(p)`</del>, <del>`=RIGHT(p)`</del>, <del>`GAMEPAD n`</del>.

    Removed useless commands: <del>`TOUCHSCREEN`</del>, <del>`KEYBOARD OPTIONAL`</del>

- When the device's virtual keyboard is hidden by the user, the corresponding I/O [Registers](#registers) flag is updated. TODO: link

- New syntaxe to [wait until device screen is tapped](#wait-tap).

- New function that return [the height taken by the device virtual keyboard](#heightkeyboard) in pixels when it's visible.

- New `HAPTIC h` command to [trigger haptic feedback](#haptic-pattern) on the device.

**Control flow:**

- New program control flow: [`ON GOTO`](#on-goto), [`ON GOSUB`](#on-gosub) and [`ON RESTORE`](#on-restore).

**Data:**

- New keyword to [skip read data](#skip-number).

- It's now possible to [access data directly using its label](#labels) without the need to `READ` them.

**Math:**

- [Cosine, sine](#cosinecosnumbersinesinnumber) and [arc tangent](#arcatanxy) functions internally replace π by 0.5.

    > Pico-8 does exactly that.

    - an angle of 0.0 or 1.0 reprensent the right direction.
    - an angle of 0.5 reprensent the left direction.
    - an angle of 0.25 reprensent the top direction.
    - an angle of -0.25 reprensent the bottom direction.

- Removed trigonometric functions: <del>`ACOS`</del>, <del>`ASIN`</del>, <del>`HCOS`</del>, <del>`HSIN`</del>, <del>`HTAN`</del>.

- New function to return the [ceiling value](#ceilceilnumber) of a numerical value:

- New function to return the [flooring value](#floorfloornumber) of a numerical value:

- New function to [clamp a numerical value](#clampedclampvalueminmax).

- New syntax to compute [vector length](#lengthlengthx,y).

- New function to compute [easing function](#interpolationeasefunctionmodevalue).

- New random generator using 🌐[PCG](https://www.pcg-random.org/). It's more random now.

- New syntax to generate random number that allow to [save the current state](#randomrnd-addressbrrandomrndmaxaddressbrrandomize-seedaddress).

**Text:**

- New command to expose the existing [overlay message API](#message-text):

- `PRINT` command can now [word wrap before breaking word](#print-expression-list).

    If there is not enough space to print a word inside the window, a new line is inserted to avoid breaking the word in two.

**Other:**

- New command to [support programs from the original LowRes NX](#compat):

- Reuse the `PAUSE` command to bring the [internal debugger](#pause).

- Removed commands and functions: <del>`PAUSE ON/OFF`</del>, <del>`PAUSE=`</del>.

- Removed reserved keywords: <del>`ANIM`</del>, <del>`CLOSE`</del>, <del>`DECLARE`</del>, <del>`DEF`</del>, <del>`FLASH`</del>, <del>`FN`</del>, <del>`FUNCTION`</del>, <del>`LBOUND`</del>, <del>`OPEN`</del>, <del>`OUTPUT`</del>, <del>`SHARED`</del>, <del>`STATIC`</del>, <del>`TEMPO`</del>, <del>`VOICE`</del>, <del>`WRITE`</del>.

**Memory:**

- An almost compatible memory mapping.

    The total addressable memory has increased from 64 Kibit to 128 Kibit.

    Lowres NX:

|    addr | size      | purpose          |
| -------:| --------- | ---------------- |
| `$0000` | 32 Kibit  | Cartridge ROM    |
| `$8000` | 4 Kibit   | Character Data   |
| `$9000` | 2 Kibit   | BG0 Data         |
| `$9800` | 2 Kibit   | BG1 Data         |
| `$A000` | 16 Kibit  | Working RAM      |
| `$E000` | 4 Kibit   | Persistent RAM   |
| `$FE00` | 256 Bytes | Sprite Registers |
| `$FF00` | 32 Bytes  | Color Registers  |
| `$FF20` |           | Video Registers  |
| `$FF40` |           | Audio Registers  |
| `$FF70` |           | I/O Registers    |

LowResRMX:

|     addr | size       | purpose          |
| --------:| ---------- | ---------------- |
|  `$0000` | 8 Kibit    | BG0 data         |
|  `$2000` | 8 Kibit    | BG1 data         |
|  `$4000` | 8 Kibit    | BG2 data         |
|  `$6000` | 8 Kibit    | BG3 data         |
|  `$8000` | 4 Kibit    | Character Data   |
|  `$9000` | 20 Kibit   | Working RAM      |
|  `$E000` | 6 Kibit    | Persisent RAM    |
|  `$FB00` | 1020 Bytes | Sprite registers |
|  `$FF00` | 32 Bytes   | Color registers  |
|  `$FF20` | 10 Bytes   | Video registers  |
|  `$FF40` | 48 Bytes   | Audio registers  |
|  `$FF70` | 40 Bytes   | I/O registers    |
|  `$FFA0` | 6 Bytes    | DMA registers    |
| `$10000` | 64 Kibit   | Cartridge ROM    |

- New command and registers for [fast memory copying](#dma-copy-rom):

- The `COPY` command is now slightly faster. Other command that operates on memory are not affected.

**The BASIC language:**

- A colon `:` can be used to [separate multiple instructions](#instructions-separator) on the same line.

- Program code is not forced to be uppercase anymore.

**The iOS app:**

- The size of the program thumbnails are 128x128 pixels.

**The fantasy hardware:**

- The number of tokens, symbols a program can have has been increased.

- The number of CPU-cycles a program can execute per frame, per vbl and per raster has been increased to accommodate with the bigger screen size.

**Particles:**

TODO: redo it

> Work-in-progress: everything will changes

The particles library reuses sprites to make them appear, disappear, change their character data, and move them across the screen.

---

`PARTICLE first,count AT address`

Declare the use of `count` sprites starting from `first` and store the internal data at `address`.

Each sprite will consume 6 bytes of memory. For each sprite:

| addr | size    | purpose         |
| ----:| ------- | --------------- |
|   +0 | 2 Bytes | Speed on x axis |
|   +2 | 2 Bytes | Speed on y axis |
|   +4 | 1 Byte  | Appearence      |
|   +5 | 1 Byte  | Current frame   |
|   +4 | 2 Bytes | Lifetime        |

e.g.:

	PARTICLE 1,20 AT $9000
	'Will use sprite 1 to 20 included.
	'Store internal data from $9000 to $9078 excluded.

---

`PARTICLE appearance DATA label`

Declare a `label` that contains data for the particle `appearance`.

`appearance` is a number between `0` and `23`.

The associated data must contain a list of character numbers that will be used to draw the particle. Using a negative number will loop back to the previous data value.

A common example is to use zero (0) and minus one (-1) for the last two data values, which allows the particle to disappear until the sprite is reused.

e.g.:

	my_appearance:
	data 1,2,3,4,0,-1
	'will show the character 1 to 4, each per frame.
	'and loop with the character 0.

---

`PARTICLE OFF`

Clear all sprite positions and reset all particle appearances.

---

`ON PARTICLE CALL subprogram`

For each particle, the system will call the subprogram using an interrupt.

The target `subprogram` will receive two arguments:

1. The `sprite` number,
2. The particle `address`.

	sub mypart(spr,addr)
		sy=peekw(addr+2)
		pokew addr+2,sy-5
		'change the speed in y axis
	end sub

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

## How does it work

LowResRMX does not have a framebuffer where users paint pixels like in Pico-8. Instead, it simulates an NES/Gameboy PPU chip, which traverses its memory to generate output pixels based on characters, backgrounds, sprites, palettes and register information.

### Screen

The fantasy screen has a maximum resolution of 216x384 pixels.

It features perfectly square pixels, a fixed portrait orientation, and always fills the entire device screen.

Depending on the device's screen ratio, the number of visible virtual pixels may vary from 216x288 on 4:3 devices to 177x384 on 19.5:9 devices (and maybe more).

That's why LowResRMX provides an API to request the [actual visible dimension of the screen](#widthshownwheightshownh).

Addionally, the device can possess notch and bottom inset full-width buttons. For that LowResRMX provides an API to request the offset from the visible dimension to the [safe area](#leftsafeltopsafetrightsaferbottomsafeb).

### Colors

LowResRMX provides the user with a fixed set of [64 colors](#64-colors).

The colors are grouped into sets of 4 called palette, with 8 palettes available for users to colorize their creations.

The 1st color of the 1st palette will be used as the background color for the whole program. For the following palettes (2nd to 8th), the 1st color is transparent.

This can be altered by the [`SYSTEM` command](#system-setting-value).

From now on, the terms:
- **a palette** apply to one of the 8 palettes with 4 colors inside,
- **a color** means one of the 4 colors within a palette or one of the 64 available colors.

Sauce: 🌐[FAMICUBE palette created by Arne](https://lospec.com/palette-list/famicube).

### Characters

The 256 characters are block of 8x8 pixels and are assigned to a number from 0 to 255.
Whenever this number is use to be drawn in a background layer or a sprite, the rendering process will draw the corresponding block of 8x8 pixels.

From now on, the term:
- **a character** is a block of 8x8 pixels and take 16 bytes.

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

This can be altered by the [`SYSTEM` command](#system-setting-value).

From now on, the terms:
- **a layer** is a background layer of 64x64 cells,
- **a cell** is a square of a layer that shows one character, so 8x8 pixel.

Check background [registers](#Registers) for information about the memory mapping and data format.

### Sprites

LowResRMX offers 170 sprites, each with:
- X and Y position,
- a dimension of 8x8, 16x16, 24x24 or 32x32 pixels,
- a character number for the top left one,
- a palette,
- horizontal and vertical flip,
- and priority to show the sprite on top of bg cells with priorities.

The 170 sprites are drawn in order from 170th (bottom) to 1st (top).
The sprite numbers range start at zero 0, so the last one is 169.

Sprites are groups of characters ranging from 1x1 to 4x4. The difference with background is that you only choose the top-left character. The other character numbers are dependent on the top-left one. To compute it, add 16 from the first one to reach the character at the right, and add 16 from the first one to get the one below.

Example with a 2x2 sprite with the first character to be 1:

|    |    |
| -- | -- |
|  1 |  2 |
| 16 | 17 |

A sprite can be freely placed anywhere on the screen with 1/16 sub-pixel precision. This means you can add 0.5 to its position even if there will be no change visually.

### TODO: layer and prio

### Rendering

The fantasy screen is rendered line by line, this is called a raster.

During raster, the pixels from background layers and sprites are drawn according to priority flags and the palette is applied. All impacted parameters are not fixed, and a sub-routine can be used between each line to alter some parameters and create interessing visual effect, see [`ON RASTER CALL s`](#on-raster-call-s) and [`=RASTER`](#raster
).

Check the [sprite API](#sprite-api) and the [background API](#background-api).

### CPU & cycles

LowResRMX simulate a fantasy console with limitation, one of them is about the number of instruction the CPU can execute before the next frame MUST be drawn and presented to the player.

This is obtained by a set of different rules applied to an internal CPU cycle counter. When the maximum is reached, the next frame is drawn no matter what. This can have the following effect:

1. the execution of the main program is halted and will continu after the frame is drawn. The program will run slower.

2. if the maximum cycles is reach during a RASTER interrupt, the next line is not drawn.

### Cartridge

A LowResRMX cartridge is a text file containing the program code and 16 virtual files after the text.

The virtual files data are encoded in hexadecimal and should respect a specific format.

> Manually editing the virtual files data is not recommended as it can lead to unexpected behavior. Instead, use the provided tools and APIs to manipulate the virtual files.

### Virtual file

There is 16 virtual files are identified by a number (`id`) and can contain a `comment$`.

    #id:comment$
    data

- `id` is a number between (0..15).
- `comment$` is a string **without double quote** of 32 characters max.
- `data` are a list of hexadecimal literals.

When the program is started, the content of the files are loaded and [mapped to the memory](#memory-mapping).

The memory address for the files start at $10000, and the best way to know this address is to use the [`ROM(file)`](#rom-file) command.

Check the [file API](#file-api).

### Memory

The fantasy hardware simulates a [memory mapping](#memory-mapping), making access to the cartridge ROM and registers accessible using an address (`$0000`..`$1FFFF`).

Each address contains a value of one byte (0..255). Some of those addresses are readable, writable, both or none.

Check the [memory API](#memory-api).

### Sound

The fantasy hardware simulates 4 voices to play sound effect and music using an internal tracks player .

Check the [sound API](#sound-api).

### Overlay

When the program is running, there is an option in the device application that enable showing the screen ovelay.

This screen show the simulated CPU usage by counting how many [fantasy CPU cycles](#cycles) has been consumed already.

It also show any message that has been printed on the overlay using  the [`TRACE` command](#trace-expresionexpression).

### Keyboard

The fantasy hardware as access to the device virtual keyboard. It allow user to type while running a program.

Check the [input API](#input-api).

## Program Language

The programming language follow the path of the original Lowres NX. It's a BASIC type language and here's how to use it.

### Program

A program is a list of so called instructions and executed by the fantasy hardware one by one from top to bottom. The simplest and iconic program could be:

    print "hello world!"

Users are invited to create a new program, types the instruction above and run it.

Keywords and values are separated by spaces and instructions with new line.

    print "hello..."
    print "second instruction"

> The program do not care about upper or lower case, use the one you prefers.

### Instructions

Instructions are something to tell the fantasy hardware to do something.

- Assigning a [literal](#literals) or an [expression](#expressions) to a [variable](#variables).
- Executing a [command](#commands) or a [function](#functions).
- Calling a [procedure](#procedures) with or without [argument](#parameter-arguments-and-scopes).
- Jumping to a [label](#labels) or going back from the stack TODO: link

### Identifiers

1. An identifier is a word defined by the user and are used to declare variables, labels or procedures (more on those terms later), e.g.: `hero`, `enemies`, `HP`, `score`, `spawn_monsters`, `updateScreen`, `CollectCoins`, `level12`…

2. An identifier can also be a built-in function name provided by LowResRMX as part of the [API](#api-instructions).

Valid characters for identifiers are: ASCII letters, digits and the underscore `_`. But they CANNOT start by a digits and they CANNOT have more than 21 characters.

**Important:** Not all identifiers are valid because some of them are reserved by the language itself, check the [list of reserved keywords](#reserved-keywords) and learn them to avoid common mistakes

### Variables and assignation

A variables is a value that retains in the memory fantasy hardware as long as the program is running. They can be used to store positions, scores, HP and all sort of information that users can think of. The purpose of variable is to store a value inside to get it back later.

Example: think of a score counter that start at zero and increment every time a plumber hero jump on enemies heads. A nice name for this value can be `score` and can be declared and assigned like this:

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

Variable syntax:

    identifier[$][(expression)]

This allow variables to have 4 differents form:

- `my_number`
- `my_string$`
- `my_number_array()`
- `my_string_array$()`

### Literals

Identifiers can store two types of value: number and string.

Numbers can be integer or decimal, e.g.: `123`, `-45`, `0.01`, `12345.6789`

String can only contains ASCII7 characters: letters, digits, some punctuations and some control characters: `"hello WORLD! 123"`

Learn more about the technical information about [number type](#number-limits) and [string type](#string).

### Arrays

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

### Labels, jumps and embeded data

Another feature that used identifiers are labels. They are used to mark a position in the program to go back to it later. User have to understand that programs are executed line by line from top to bottom and labels are one solution to go back to the top of the program or anywhere else (almost).

Here is an example that demonstrate how to recreate the program above using labels:

    score=0
    start:
        print "score",score
        wait tap
        score=score+1
    goto start

A label is declared by an identifier followed by a coma `:` (here "start"), and the `GOTO` command will make the program execution to "jump" to the previous `label` "start", creating an infinite loop.

---

About using label identifier as [rvalue](#lvalue-and-rvalue). This feature allow use label name that lead to [constant string value](#data-rom-and-files) directly instead of having to store the value in a varible.

Example, instead of writing:

    data "chocolate"
    read like$
    print "I like",like$

You can write:

    like: data "chocolate"
    print "I like",like

And it working with in all [expression](#expression) that involve [literal string](#literals) or [string variable](#variables).

    test1: data "entanglem"
    print test1+left$(test1,3)
    test2: data 1234.5
    print test2*2

### Stack, sub-routine and return

Using `GOTO` command to jump to another part of the program is very usefull to organize the code of a program and create loop. Something is not enough, introcucing the `GOSUB` command.

The stack is a place inside the fantasy hardward that track where a program was before jumping to a label, allowing to `RETURN` to the previous location.

This allow to reach a same label from different parts of the code, while being able to return to it later.

Example, of a program that reach the same label from to places.

    one:
        print "one"
        gosub common
    two:
        print "two"
        gosub common
        goto one

    common:
        wait tap
        return

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

By adding this piece of code after the previous one and run the program, the solution of the operation 120+3 will be printed on the screen: "result 123".

The advantage of calling proceduces is to be able to do it again and again.

    call addition(45,-78,result)
    print "result",result

This time, it will print "result -33".

The differences with [sub-routine](#stack-subroutine-and-return) is the [scope](#parameters-arguments-and-scopes).

### Parameters, arguments and scopes

An other concept that came with procedures are the parameters and the arguments but first, users have to learn as thing about variable's scope.

By default, when declaring a variable in a program, it's scope is limited and the variable is unknown inside a procedure.

    score=0
    sub print_score
        🐞print "score",score
    end sub
    call print_score

This program will produce the error: "variable not initialized". The program can't access the variable `score` inside the `print_score` proceduce.

One option is to pass the variable to the proceduce. Two changes must be made for that.

1. User need to add a parameter to the proceduce declaration:

        sub print_score(s)
            print "score",s
        end sub

2. User also need to add an argument to the proceduce call:

        call print_score(score)

When the proceduce `print_score` is called, the argument `score` is passed to the proceduce inside the `s` argument.

Corrected program:

        score=0
        sub print_score(s)
            print "score",s
        end sub
        call print_score(score)

An other option is to rely on [global scope](#global-scope).

### Passed by reference

Arguments are passed by reference, it's means that a variable passed to a proceduce and being modified by this procedure will stay modified when exiting the proceduce.

    sub modify_it(a)
        a=123
    end sub
    my_value=0
    call modify_it(my_value)
    print my_value

It will print `123`.

To avoid this behavior, encapsulate the argument by round bracket `(identifier)`.

    sub modify_it(a)
        a=123
    end sub
    my_value=0
    call modify_it((my_value))
    print my_value

It will print `0`.

### Local and global scope

When variables are initialized or declared in the main program body, they are local to the program body.

When variables are initialized or declared inside a proceduce body, they are local to this procedure body.

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

MUST be declare before trying to access it.

The syntaxe to declare a global array is a bit different:

    dim global enemies

TODO: link GLOBAL

### Comments

Comments are piece of text that is not executed. They are usefull for taking notes of what a particular piece of code is doing.

    'get touch position in cells coordinates
    tx=touch.x\8

> Avoid 🌐[Captain Obvious](https://en.wikipedia.org/wiki/Captain_Obvious) comment!

### Grammar

As say earlier, a program is a list of instructions read and executed one by one. To make it easier to read, developer use a set of spaces, identations, new lines, comments, proceduces…

Here is a set of rules on how it works:

- an instruction cannot be split on two lines.

    This will not work:

        🐞a=
        123

- label are not instruction, user can place instruction on the same line.

    ✅ This will work:

        test: print 123
        wait tap
        goto test

- the only way to squiz two instructions on one line is to use the `:` colon separator.

    ✅ This will work:

        x=touch.x : y=touch.y

    > I recommand to place spaces before and after the `:` colon until I fix the parsing issue

- `IF...THEN` use a different syntax for on line instruction, and `:` colon as instruction separator will not work correctly.

    This will not work as intended:

        do
            x=0 : y=0
            if tap then x=touch.x : y=touch.y
            print x,y
            wait tap
        loop

    `y=touch.y` will always be executed.

- Comment can be placed after the `:` colon instruction separator.

    ✅ This will work:

        a=123 :'Default value


- Identation as no meaning, user can place them whatever they want.

    ✅ This will work:

        test:
                print       "far"
        print "near"


- Check the [list of reserved keywords](#keywords), as user cannot use them for identifiers.

### Expressions

Expression are used to compute values, be assigned to identifier or passed to function as arguments. Expression can be one of:

- A [literal value](#literals):

        123
        "gabu"

- A [variable identifier](#variables-and-assination):

        myVar$
        myArray(0)

- A [function call](#functions):

        cos(1.314)
        left$(name$,8)

- An [unary, binary or group operator](#operators):

        42+24
        $FF00+A*2
        count>=0
        not dead
        (2+3)*4

- A [label identifier](#labels):

        mylabel

### Operators

Operator are used inside [expression](#expression) to compute or alter it's value.

**Arithmetic operators:**

Operate on two numeric values to produce a new one.

| symbol | example   | purpose          |
| ------:| --------- | ---------------- |
|    `-` | `-42`     | negation         |
|    `^` | `x^3`     | exponentation    |
|    `*` | `2*y`     | multiplication   |
|    `/` | `x/2`     | division         |
|    `\` | `x\2`     | integer division |
|  `mod` | `x mod 2` | modulo           |
|    `+` | `c+2`     | addition         |
|    `-` | `100-d`   | subtraction      |

The priority of execution respect the mathematical rules.

**Group operator:**

Rounded parentheses `()` are used to counter the operator priority.

**Comparison operator:**

Used to compair two numeric or string values and produce `-1` if the test succeed or `0` otherwise. Generally used as [expression](#expressions) in [conditional flow control](#control-flow).

| symbol | example  | purpose
| ------:| -------- | -------
|    `=` | `a=10`   | equal
|   `<>` | `a<>100` | not equal
|    `>` | `b>c`    | greater
|    `<` | `5<x`    | less
|   `>=` | `X>=20`  | greater or eqal
|   `<=` | `X<=30`  | less or eqal

**Bitwise operator:**

Used to manipulate each bit of numeric values.

| symbol | example                        | purpose                                                     |
| ------:| ------------------------------ | ----------------------------------------------------------- |
|  `not` | `not (x=15)`<br>`not 0`        | Bits that are 0 become 1,<br>and those that are 1 become 0. |
|  `and` | `a=1 and b=12`<br>`170 and 15` | If both bits are 1, the<br>result is 1, 0 otherwise.        |
|   `or` | `x=10 or y=0`<br>`128 or 2`    | The result is 0 if both<br>bits are 0, 1 otherwise.         |
|  `xor` | `a xor b`                      | The resulst is 1 if only<br>one bit is 1, 0 otherwise.     |

**Concatenation operator:**

Copy a string at the end of another string.

| symbol | example    | purpose                           |
| ------:| ---------- | --------------------------------- |
|    `+` | `"ab"+"c"` | Concatenate two strings into one. |

### Commands

LowResRMX provide a bunch of built-in commands to manipulate the fantasy device hardware such as the graphic or sound. You'll find them by consulting the [list of API instructions](#api-instructions).

To execute a command, use it's identifier followed by a list of coma-separated arguments or other reserved keywords. The exact syntax depends on the command.

    sprite off 0 to 169
    sprite 0,40,60,1

### Functions

LowResRMX provide a bunch of built-in function to manipulate the fantasy device hardware such as the graphic or sound. You'll find them by consulting the [list of API instructions](#api-instructions).

To execute a function, use it's identifier followed by a list of coma-separated arguments surrounded by rounded parenthesis.

    print mid$("test",3,1) :'print s

A function will always return something and MUST be used as [rvalue](#lvalue-and-rvalue) inside an [expression](#expression).

### Lvalue and Rvalue

The equal `=` sign is used both as variable assignement and comparison operator, and It can lead to some confusion. Here is a tip to help you:

- If the line start by the variable name, it's [a variable assignation](#variables-and-assignation).

- In all other cases, the variable is used as [a part of an expression](#expressions).

An lvalue appear at the left side of an assignation, and rvalue appear at the right side of an assignation. Also rvalue is used as argument of a [functions](#functions) or [proceduces](#procedures).

|                  expression | type of value    |
| ---------------------------:| ---------------- |
|      [variable](#variables) | lvalue or rvalue |
|        [literal](#literals) | rvalue           |
| [function call](#functions) | rvalue           |
|  [any operator](#operators) | rvalue           |
|            [label](#labels) | rvalue           |

### Instructions separator

An instruction generally end by a new line `\n`.

It's possible to put multiple instructions using a colon `:` on one line but with limitation:

1. Can be miss-interpreted as a label declaration. So the best way is to add a space between the idenfier and the colon.

    This will not work:

        test: data 123
        🐞restore test: wait vbl

    ✅ This will work:

        test: data 123
        restore test : wait vbl

2. Does not work with the one line syntaxe of [the `IF/THEN` command](#ifthenelse-ifthelendf-ififthen).

        if expression then instruction1 : instruction2

    `instruction2` will always be executed.

### Reserved keywords

Some identifier cannot be used by the user for variables, proceduce or label name because they are keywords reserved by the language and it's API:

`ABS`, `ADD`, `AND`, `ASC`, `ATAN`, `ATTR`, `AT`, `BG`, `BIN$`, `CALL`, `CEIL`, `CELL.A`, `CELL.C`, `CELL`, `CHAR`, `CHR$`, `CLAMP`, `CLS`, `CLW`, `COLOR`, `COMPAT`, `COPY`, `COS`, `CURSOR.X`, `CURSOR.Y`, `DATA`, `DEC`, `DIM`, `DMA`, `DO`, `EASE`, `ELSE`, `EMITTER`, `END`, `ENVELOPE`, `EXIT`, `EXP`, `FILE$`, `FILES`, `FILL`, `FLIP`, `FLOOR`, `FONT`, `FOR`, `FSIZE`, `GLOBAL`, `GOSUB`, `GOTO`, `HAPTIC`"  `HEX$`, `HIT`, `IF`, `INC`, `INKEY$`, `INPUT`, `INSTR`, `INT`, `KEYBOARD`, `LEFT$`, `LEN`, `LET`, `LFO.A`, `LFO`, `LOAD`, `LOCATE`, `LOG`, `LOOP`, `MAX`, `MCELL.A`, `MCELL.C`, `MCELL`, `MESSAGE`, `MID$`, `MIN`, `MOD`, `MUSIC`, `NEXT`, `NOT`, `NUMBER`, `OFF`, `ON`, `OR`, `PALETTE`, `PAL`, `PARTICLE`, `PAUSE`, `PEEKL`, `PEEKW`, `PEEK`, `PI`, `PLAY`, `POKEL`, `POKEW`, `POKE`, `PRINT`, `PRIO`, `RANDOMIZE`, `RASTER`, `READ`, `REPEAT`, `RESTORE`, `RETURN`, `RIGHT$`, `RND`, `ROL`, `ROM`, `ROR`, `SAFE.B`, `SAFE.L`, `SAFE.R`, `SAFE.T`, `SAVE`, `SCROLL.X`, `SCROLL.Y`, `SCROLL`, `SGN`, `SHOWN.H`, `SHOWN.W`, `SIN`, `SIZE`, `SKIP`, `SOUND`, `SOURCE`, `SPRITE.A`, `SPRITE.C`, `SPRITE.X`, `SPRITE.Y`, `SPRITE`, `SQR`, `STEP`, `STOP`, `STR$`, `SUB`, `SWAP`, `SYSTEM`, `TAN`, `TAP`, `TEXT`, `THEN`, `TIMER`, `TINT`, `TOUCH.X`, `TOUCH.Y`, `TOUCH`, `TO`, `TRACE`, `TRACK`, `UBOUND`, `UNTIL`, `VAL`, `VBL`, `VIEW`, `VOLUME`, `WAIT`, `WAVE`, `WEND`, `WHILE`, `WINDOW`, `XOR`.

## BASIC instructions

To be able to use the language as a tool to make games, LowResRMX provides instructions to:

- manipulates the execution of a program: go here, go back, wait for this, do that if this happens and stuff like that.

- store same values here, read those values there, skip this...

- declare this variables, make it available there...

In the following sections, you'll learn how to do all this abstract but important things.

### Control flow

« In software, 🌐[control flow (or flow of control)](https://en.wikipedia.org/wiki/Control_flow) describes how execution progresses from one command to the next. »

-- wikipedia

#### `END`

Stop the execution of the program.

It has the same effect as the execution reach the end of the program.

#### `IF/THEN/ELSE IF/ELSE/END IF`<br>`IF/THEN`

    if expression then
        instruction...
    [else if expression then
        instruction...]
    [else if...]
    [else
        instruction]
    end if

Will execute the list of `instruction...` if the above `expression` is true.
An [expression](#expression) is evaluated as `true` if the result of the expression is different that `0` zero.

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

---

    if condition then instruction

This form only allow one `instruction`, but it's also shorter.

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

#### `DO/LOOP/EXIT`

    do
        instruction...
        [exit/goto]
    loop

Continuously execute the list of `instruction...`.

Possible way to get out of the loop is using `EXIT` or `GOTO`.

Real example of non stop moving square that bounce on the device screen:

    x=shown.w\2
    y=shown.h\2
    sx=1
    sy=1
    do
        if x=0 or x=shown.w-8 then sx=-sx
        if y=0 or y=shown.h-8 then sy=-sy
        add x,sx
        add y,sy
        sprite 0,x,y,1
        wait vbl
        if tap then exit
    loop
    print "done"

    #2:main characters
    00000000000000000000000000000000
    0000000000000000FFFFFFFFFFFFFFFF

`EXIT` command will exit one level of `DO/LOOP`.

### `REPEAT/UNTIL/EXIT`

    repeat
        instruction...
        [exit/goto]
    until expression

Repeat the list of `instruction...` until the `expression` ahead became true.
An [expression](#expressions) is evaluated as `true` if the result of the expression is different that 0.

`EXIT` command will exit one level of `REPEAT/UNTIL`.

It different from `WHILE/WEND` because `expression` is evaluated after the list of `instruction...` is executed.

    stop_now=-1
    repeat
        print "do it anyway"
    until stop_now

`EXIT` command will exit one level of `REPEAT/UNTIL`.

#### `WHILE/WEND/EXIT`

    while expression
        instruction...
        [exit]
    wend

Repeat the list of `instruction...` while the `expression` above is true.
An [expression](#expressions) is evaluated as `true` if the result of the expression is different that 0.

`EXIT` instruction will exit one level of `WHILE/WEND`.

It different from `REPEAT/UNTIL` because `expression` is evaluated before the `instruction...` list is executed.

    count=0
    while count
    print "not executed"
    wend

#### `FOR/TO/NEXT/EXIT`

    for variable=begin to ended [step incr]
        instruction...
        [exit]
    next identifier

Repeat the `instruction...` list while varying the value of the `variable` starting at `begin` [expression](#expressions) until it reach `ended` expression included.
The increment can be changed using `incr`, allowing to iterates in reverse.

Real example that print numbers from 1 to 9 in ascending and descending order.

    for i=1 to 9
        print i;
    next i
    print
    for i=9 to 1 step -1
        print i;
    next i

`EXIT` command will exit one level of `FOR/TO/NEXT`.

#### `GOTO`

    goto label

Make the execution of the program to jump at the specific `label`.

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

#### `GOSUB/RETURN`

    gosub label
    ...
    return

Store the current location of the program execution [on top of the stack](#stack-subroutine-and-return), and jump at the specific `label`.

Expect to found a `RETURN` command to go back where it was right after the `GOSUB`.

The return location is store in a stack allow user to jump to a sub-routine and return from it.

> A good habit is to always have one `RETURN` for each `GOSUB`.

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

The important things is not the number of `RETURN` appear in the code but the number of times the command is executed. User have to understand that one `GOSUB` execution will [increase by one the size of the stack](#stack-subroutine-and-return), and one execution of `RETURN` will reduce the same stack by exactly one. If every time the `WAIT VBL` instruction is executed and the stack size is empty: it's a good sign. It means that the stack will not be overflowed.

User can print the current stack using [command `TRACE` from the debugger](#dbg-trace).

#### `ON GOTO`<br>`ON GOSUB`

    on value goto label0[,label1...]

Jump to one of the listed `label` according to a `value`.

With `ON GOSUB` will store the current program location on top of the stack before jumping, allowing to `RETURN` to this location later.

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


#### `SUB/END SUB/EXIT SUB`

Syntax:

    sub name [(parameters...)]
        instruction...
    end sub

This will define a procedure called `name` and can optionnaly received a list of `parameters`.

Syntax of the `parameters` list:

    (identifier[$][()], identifier[$][()]...)

Hm, it's a mess! Let me explain this. A parameter is always a `identifier` and can be followed by the type of value it contains. The list of possibilities are:

- a number: `my_num`
- a string: `my_str$`
- an array of number: `my_num_array()`
- an array of string: `my_str_array$()`

An empty list of `parameters` is not valid. Simply remove the parenthesis `()`.

Proceduces are isolated small program that can be executed using the [`CALL` command](#CALL). Isolated because the [variables declared inside are local](#local-and-global-scope).

> Body of sub-routines are only executed throughout the [`CALL` command](#CALL), so, it is safe to place them at the beginning of the program.

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

`EXIT SUB` command will exit the `SUB`.

> Using `GOTO` inside a proceduce is probably a bad idea. Specially if the destination target is outside of the body. It will permanently increase the stack size without any possibility to reduce it.

> Using `GOSUB` inside a proceduce that jump to a label outside of the body is also weird because the scope of the procedure will be used to execute the instructions.

### Embeded data

LowResRMX provide two ways to storing data or assets inside a program.

1. Use the combinaison of `DATA` and `READ` to store readable number and string and access it when you need it.

    Real example that list the Straw Hat Pirates members:

        data 11
        data "lufy",19,"zoro",21,"nami",20
        data "usopp",21,"sanji",21,"chopper",17
        data "robin",30,"franky",36,"brook",90
        data "jinbe",46,"vivi",18
        read count
        for i=1 to count
            read name$,age
            print name$,age
        next i

    Internaly it will use a read pointer that iterates all constant values.

2. Use the [virtual file](#virtual-file) system accessible throughout the [file API](#file-api).

#### `DATA constant[,constant...]`

A list of `constant` values (numbers or strings) that can be accessed using the [`READ`](#read-variablevariable) command.

#### `READ variable[,variable...]`

Read a list of values inside `variable` that was declared using [`DATA`](#data-constantconstant).

#### `RESTORE [label]`

Move the read pointer to a specified `label`. The declared constant values that appear after the label will be read next. When ommited, restore at the beginning of the program.

#### `SKIP number`

Allow to skip a `number` of constant values by moving the read pointer.

    data "failure"
    data "success"
    skip 1
    read word$
    print word$

#### `ON RESTORE`

    on value restore label0,[label1...]

Move the read pointer to one of the listed `label` according to a `value`.

### Variables and scopes

#### `GLOBAL`

    global identifier...

Declare a list of number or string variables to be globally [accessible in all scopes](#local-and-global-scope).

`GLOBAL` are illegal inside a `SUB/END SUB` subroutine body.

#### `DIM`

    dim identifier(highest) [,identifier(highest)]...

Will declare one or more arrays with `highest`+1 number of elements.

Array of numbers or strings MUST be declared before reading or writing it.

    dim scores(1),names$(1)
    for i=0 to 1
        print names$(i);":",scores(i)
    next i

#### `DIM GLOBAL`

    dim global identifier(highest) [,identifier(highest)]...

Similar to `DIM` but will [declare the arrays](#arrays) [globally accessible](#local-and-global-scope).

`DIM GLOBAL` are illegal inside a `SUB/END SUB` subroutine body.

#### `=UBOUND`

    highest=ubound(identifier[,dimension])

Return the `highest` index of the array variable `identifier` at specified `dimension`.

#### `SWAP a,b`

Swap the values of the variable `a` and `b`. They have to share same type.

## API instructions

« An 🌐[application programming interface (API)](https://en.wikipedia.org/wiki/API) is a connection between computers or between computer programs. »

-- wikipedia

LowResRMX provide a bunch of built-in [commands](#commands) and [functions](#functions) to communicates with the differents features provided by the fantasy console: input, graphics, sound, memory and more.

### Sprite API

Sprite are limited in numbers and should be used to show moving objects above background layers. It not an oblication and cool things can be made by infringe this rules.

Check [how sprites works](#sprites) and [sprites registers](#sprite-registers).

#### `SPRITE sprite,[x],[y],[character]`

Sets the position `x` and `y` in pixel coordinates and `character` number of the `sprite`.

`x`,`y` and `character` can be ommited to keep their current value.

An example that show a smiley sprite moving in circle:

    i=0
    do
        add i,1,0 to 99
        sprite 0,cos(i/100)*40+60,sin(i/100)*40+60,1
        wait vbl
    loop
    #2:main characters
    00000000000000000000000000000000
    007EFFFFEDFFFF7E0000003636000000

Omitted parameters will keep their previous values.

#### `SPRITE sprite [PAL palette] [FLIP horizontal,vertical] [PRIO priority] [SIZE size]`

Sets one or more attributes for the `sprite`:

- `PAL palette` Change the `palette` (0..7).
- `FLIP horizontal,vertical` Flip the sprite on `horizontal` and `vertical` axis.
- `PRIO priority` Change the `priority` (0..1).
- `SIZE s` Change the size, a.k.a.: the number of characters width and height.

Omitted parameters will keep their previous values.

Example of a sprite that get flipped according to it's position on the screen:

    do
        sprite 0,touch.x,touch.y,1
        sprite 0 pal 6
        sprite 0 flip touch.x>shown.w/2,touch.y>shown.h/2
        wait vbl
    loop
    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    40A0D0E8F4D0E818C06030180C305808

#### `x=SPRITE.X(sprite)`<br>`y=SPRITE.Y(sprite)`

Return the position `x` or `y` of the `sprite` (0..169) in pixels.

Example of a sprite smoothly following the finger touch:

    sprite 0,shown.w,shown.h,1
    do
        x=sprite.x(0)+(touch.x-sprite.x(0))/4
        y=sprite.y(0)+(touch.y-sprite.y(0))/4
        sprite 0,x,y,
        wait vbl
    loop
    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    007E7E66667E7E00FF81BDA5A5BD81FF

#### `character=SPRITE.C(sprite)`

Return the first `character` number of the `sprite` (0..169).

#### `SPRITE.A sprite,attributes`

Sets all `attributes` at once for the `sprite`.

Check [Character attributes](#character-attributes).

#### `attributes=SPRITE.A(sprite)`

Return the whole `attributes` flags of the `sprite` (0..169).

Example: that show how to read sprite attributes:

    sprite 0,80,80,1
    sprite.a 0,255
    do
        text 4,4,"palette: 00000"+bin$(sprite.a(0) and %111)
        text 4,5,"flip:    000"+bin$(sprite.a(0) and %11000)
        text 4,6,"size:    0"+bin$(sprite.a(0) and %1100000)
        wait tap
        sprite.a 0,rnd(255)
    loop

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    40A0D0E8F4D0E818C06030180C305808

#### `SPRITE OFF`

Hides all sprites.

#### `SPRITE OFF sprite`

Hide one `sprite`.

Example that hide a sprite at each tap:

    for i=0 to 159
        sprite i,rnd(shown.w-8),rnd(shown.h-8),1
    next i
    i=0
    while i<159
        wait tap
        sprite off i
        add i,1
    wend

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    0000000000000000FFFFFFFFFFFFFFFF

#### `SPRITE OFF sprite1 TO sprite2`.

Hides all sprites from range `sprite1` to `sprite2` included.

#### `collides=SPRITE HIT(sprite)`

Return if the `sprite` (0..169) `collides` (0/-1) with another sprite. Collision detection is done by checking overlapping pixels that are not transparent (a.k.a.: not using the color 0).

Example of a ball that bounce on the wall and fall into holes:

    again:
    for i=1 to 20
        sprite i,rnd(shown.w)-8,rnd(shown.h)-8,2
    next i
    x=0
    y=0
    sx=1
    sy=1
    do
        sprite 0,x,y,1
        if sprite hit(0) then
            wait 30
            goto again
        end if
        wait vbl
        x=sprite.x(0)+sx
        y=sprite.y(0)+sy
        if x<0 or x>shown.w-8 then sx=-sx
        if y<0 or y>shown.h-8 then sy=-sy
    loop

    #1:MAIN PALETTES
    0405

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    003C7E7A72623C00000030240C1C0000
    000000010101021C003C7E7F7F7F3E1C

Use [`sprite=HIT`](#sprite-hit) to the colliding `sprite`.

#### `collides=SPRITE HIT(sprite,sprite1)`

Return if the `sprite` (0..169) `collides` (0/-1) with the `sprite1`.

Example of two owerlapping sprites:

    sprite 20,46,46,1
    do
    sprite 10,40+((timer/60) mod 2)*4,40,1
    if sprite hit(10,20) then
        text 8,5,"hit"
    else
        text 8,5,"   "
    end if
    wait vbl
    loop

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    FFFFFFFFF0F0F0F00000000000000000

Use [`sprite=HIT`](#sprite-hit) to the colliding `sprite`.

#### `=SPRITE.HIT(sprite,sprite1 to sprite2)`

Return if the `sprite` (0..169) `collides` with any other from the range `sprite1` to `sprite2` included (0..169).

Use [`sprite=HIT`](#sprite-hit) to the colliding `sprite`.

#### `sprite=HIT`

Return the `sprite` which collided, resulting of the last `=SPRITE.HIT()` function call (any of the three form).

Example of a ball that bounce on the wall and destroy the obstable:

    for i=1 to 20
        sprite i,rnd(shown.w)-8,rnd(shown.h)-8,2
    next i
    again:
    x=0
    y=0
    sx=1
    sy=1
    do
        sprite 0,x,y,1
        if sprite hit(0,1 to 20) then
            wait 15
            sprite off hit
            goto again
        end if
        wait vbl
        x=sprite.x(0)+sx
        y=sprite.y(0)+sy
        if x<0 or x>shown.w-8 then sx=-sx
        if y<0 or y>shown.h-8 then sy=-sy
    loop

    #1:MAIN PALETTES
    0405

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    003C7E7A72623C00000030240C1C0000
    000000010101021C003C7E7F7F7F3E1C

### Background API

The most basic way of drawing using the background API is to, first, choose which cell's attribute to use, then paint the cells with a characters number.

Another way of doing is to copy attributes and characters from memory.

Aside to that, there is an API to scroll the layers.

Text generally use background mechanics as well, there is an [API to draw text](#text-api) too.

Check [How backgrounds works](#backgrounds) and [background data](#background-data).

#### `CLS`

Clear all background layers with character zero 0, resets the current window to the default and the layer scrolling values.

#### `CLS layer`

Only clear the background layer numbered `layer` with character zero 0. Do not alter the scrolling value.

#### `SCROLL layer,[x],[y]`

Set the scroll offset on `x` and `y` axis of the `layer` in pixels.

    text 10,10,"hello!"
    bg 1
    text 10,10,"hello!"
    scroll 0,0,-4

Omitted parameters will keep their previous values.

#### `x=SCROLL.X(layer)`<br>`y=SCROLL.Y(layer)`

Return the scroll offset on `x` and `y` axis of the `layer` in pixels.

Example of getting the scroll offset of the background layer:

    for x=0 to 63
    for y=0 to 63
        cell x,y,rnd(3)
    next y
    next x
    do
        x=scroll.x(0)+(touch.x-scroll.x(0))/4
        y=scroll.y(0)+(touch.y-scroll.y(0))/4
        scroll 0,x,y
        wait vbl
    loop

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    FFFFFFFFFFFFFFFF0000000000000000
    0000000000000000FFFFFFFFFFFFFFFF
    FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

#### `BG layer`

Change the current `layer` (0..3) for further cells draw operations.

    bg 0
    text 10,10,"\"
    bg 1
    text 10,10,"/"

#### `PAL palette`

Change the current `palette` for further cells draw operations.

    print "hello",
    pal 1
    print "world"

#### `FLIP [horizontal],[vertical]`

Change the `horizontal` and `vertical` flip attributes for further cells draw operations, considering 0 as not flipped and something else as flipped.

    flip 1,0
    print "dlrow olleh"

Omitted parameters will keep their previous values.

#### `PRIO priority`

Change the `priority` for further cells draw operations.

    sprite 0,78,78,1
    prio 1
    text 10,10,"s"

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    003C7E6A7E7E3C000000183C00000000

#### `ATTR attributes`

Sets the palette, flip and priority `attributes` all at once for further cells draw operations.

Check the [cell attributes reference](#cell-attributes).

#### `CELL x,y,[character]`

Draw to the `x`,`y` cell of the [`current layer`](#bg-layer) with the `character` using the current attributes.

By omiting the `character` argument, the command will only alter the attributes: palette, flip and priority.

Use [`BG layer`](#bg-layer), [`PAL palette`](#pal-palette), [`FLIP horizontal,vertical`](#flip-horizontal-vertical), [`PRIO priority`](#prio-priority) and [`ATTR attributes`](#attr-attributes) to alter the [current attributes](#cells-attributes).

E.g. draw a face:

    cell 10,10,1

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    003C7E6A7E7E3C000000183C00000000

E.g. draw an inverted blue r letter:

    cell 10,10,242
    flip 1,0
    pal 1
    cell 10,10,


#### `character=CELL.C(x,y)`

Return the `character` of the `x`,`y` cell from the [`current layer`](#bg-layer).

Example of a (silly) way to read the ASCII code of a char:

    bg 2
    for x=0 to shown.w\8
    for y=0 to shown.h\8
        cell x,y,192+rnd(63)
    next y
    next x
    bg 1
    bg copy 0,0,6,4 to 4,4

    do
        bg 2
        c=cell.c(touch.x\8,touch.y\8)
        bg 0
        text 5,5,"$"+right$("0"+hex$(c-192),2)
        wait vbl
    loop

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
    0000000000000000FFFFFFFFFFFFFFFF

    #3:MAIN BG
    00000604010001000100010001000000
    01000100010001000100020001000100
    01000100010002000000020002000200
    02000200

#### `attributes=CELL.A(x,y)`

Return the `attributes` of the `x`,`y` call from [`current layer`](#bg-layer).

Example that display cell's attributes:

    bg 2
    for x=0 to shown.w\8
    for y=0 to shown.h\8
        attr rnd(255)
        cell x,y,192+rnd(63)
    next y
    next x
    bg 1
    bg fill 5,5 to 13,9 char 2
    bg fill 4,4 to 12,8 char 1
    bg 0
    pal 0
    prio 1
    flip 0,0
    text 5,5,"Pal:"
    text 5,6,"Flip:"
    text 5,7,"Prio:"
    do
        bg 2
        a=cell.a(touch.x\8,touch.y\8)
        bg 0
        text 9,5,right$("00"+bin$(a and %111),3)
        text 10,6,right$("0"+bin$((a\8) and %11),2)
        text 10,7,right$("0"+bin$((a\32) and %11),2)
        wait vbl
    loop

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
    0000000000000000FFFFFFFFFFFFFFFF

Check the [cell attributes reference](#cell-attributes).

#### `BG FILL x1,y1 TO x2,y2 CHAR character`

Fills all cells from `x1`,`y1` coordinates to `x2`,`y2` with `character` and the current attributes.

    bg fill 1,1 to 5,5 char 1
    bg fill 2,2 to 4,4 char 2

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    FFFFFFFFFFFFFFFF0000000000000000
    0000000000000000FFFFFFFFFFFFFFFF

#### `TINT y,y [PAL palette] [FLIP horizontal,vertical] [PRIO priority]`

Sets to the cell `x`,`y` (0..63) of the [`current layer`](#bg-layer), one or more attributes: `palette` (0..3), `horizontal` (0/1) and `vertical` (0/1) flip and `priority` (0/1).

    print "hello!"
    tint 5,0 pal 7

#### `BG TINT y1,y1 TO x2,y2 [PAL palette] [FLIP horizontal,vertical] [PRIO priority]`

Sets to the cells from the rectangle `x1`,`y1` to `x2`,`y2` (0..63), one or more attributes: `palette` (0..3), `horizontal` (0/1) and `vertical` (0/1) flip and `priority` (0/1).

Similar to `TINT y,y [PAL pal] [FLIP h,v] [PRIO p]` but with a rectangle of cells instead of just one.

#### `BG SCROLL x1,y1 to x2,y2 step x3,y3`

Move the cell's attributes and character from the rectangle `x1`,`y1` to `x2`,`y2` (0..63) of the c[`current layer`](#bg-layer) by `x3`,`y3` in cell coordinates.

This feature is internally used to scroll text when it reach the bottom of the [window](#window-xywidthheightlayer).

#### `BG SOURCE address[,width,height]`

Set the memory `address` to use as source for [`BG COPY x1,y1,width,height TO x2,y2`](#bg-copy-x1-y1-width-height-to-x2-y2) operations.

When `width` and `height` are specified, they are used as indiquating the number of cells in the source.

If they are not specified, the [official background data format](#background-source-data) are used to retrieve the width and the height.

> When the program is stared the default address is taken by internally executing `BG SOURCE ROM(3)`

Example that generate background using random numbers:

    a=$9000
    for r=0 to 63
        for c=0 to 63
            poke a,1
            inc a
            poke a,rnd(1)*8
            inc a
        next c
    next r

    bg source $9000,64,64
    bg copy 0,0,64,64 to 0,0

    do
    x=0
    add x,sin(timer/600)*cos(timer/6000)*200
    add x,sin(timer/500)*50

    y=0
    add y,cos(timer/400)*sin(timer/4000)*200
    add y,cos(timer/700)*50

    scroll 0,x,y
    wait vbl
    loop

#### `BG COPY x1,y1,width,height TO x2,y2`

Copy from the rectangle `x1`,`y1` (0..63) with `width`,`height` (0..63) the cell's attributes and character numbers from background source specified previously using [`BG SOURCE address[,width,height]`](#bg-source-address-width-height) to `x2`,`y2` (0..63) of the current background layer.

#### `MCELL x,y,character`

Draw to the `x`,`y` cell to the background source specified previously using [`BG SOURCE address[,width,height]`](#bg-source-address-width-height) with the `character` using the current attributes.

Similar to [`CELL x,y,character`](#cell-x-y-character) but modify the source in memory instead of the [`current layer`](#bg-layer).

The source MUST point to writable memory. It will not work with `BG SOURCE ROM(3)`.

Example that continously draw to the background source:

    text 4,4,"touch to copy"
    bg 1
    bg source $a000,64,64

    do
        pal rnd(7)
        mcell rnd(ceil(shown.w/8)),rnd(ceil(shown.h/8)),rnd(1)
        if touch then
            bg copy 0,0,ceil(shown.w/8),ceil(shown.h/8) to 0,0
        end if
        wait vbl
    loop

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    FFFFFFFFFFFFFFFF0000000000000000

#### `character=MCELL.C(x,y)`

Return the `character` of the `x`,`y` cell from the background source.

Similar to [`character=CELL.C(x,y)`](#character-cell-c-x-y) but modify the source in memory instead of the [`current layer`](#bg-layer).

#### `attributes=MCELL.A(x,y)`

Return the `attributes` of the `x`,`y` call from the background source.

Similar to [`attributes=CELL.A(x,y`](#attributes-cell-a-x-y) but modify the source in memory instead of the [`current layer`](#bg-layer).

#### `TEXT x,y,text$`

Will print `text$` on the [`current layer`](#bg-layer) starting at `x`,`y` cell using the current background attributes on the [`current layer`](#bg-layer).

  data "ga","bu","zo","meu"
  for i=0 to 3
    pal i
    read c$
    text 8+i*2,10,c$
  next i

#### `NUMBER x,y,number,count`

Will print the latest `count` digits of the `number` on the current layer starting at `x`,`y` cell using the current background attributes on the [`current layer`](#bg-layer).

Similar to [`TEXT x,y,text$`](#text-xytext) but dedicated to print number instead of text.

    score=123
    number 10,10,score,6

#### `FONT first`

Sets the `first` character number that will defines the range of ASCII characters used for `TEXT`, `NUMBER` and `PRINT` commands.

The default value is 192, which points to where the default font is loaded at the beginning of the program if the characters are not used. It MAY be overriden by `LOAD` operation.

### Text API

Commands dedicated to draw text on layers using characters data. 64 characters can be reserved to print characters on screen. By default thoses characters number are (192..255) but can be changed using the [`FONT` command](#font-first).

#### `WINDOW x,y,width,heigh,layer`

Sets the current window at `x`,`y` with `width`,`height` and on `layer` where text will be drawn.

    window 4,8,shown.w\8-8,20,0
    print "Oh my god! There's a tremendous amount of work to do. Can I do a little bit of it today."

By default, the window is sets inside the safe area delimited by the safe functions.

#### `CLW`

Clears the window by replacing all the cells by the character 0. It also reset the cursor position.

#### `LOCATE x,y`

Move the cursor location at `x`,`y` in cells coordinates inside the window.

    for i=1 to 9
        locate i,i
        print str$(i);
    next i

#### `=CURSOR.X`<br>`=CURSOR.Y`

Return the cursor location `x`,`y` in cells coordinates inside the window.

#### `PRINT expression-list`

Outputs string `expression-list` onto the current window.

`expression-list` can be one or more of:
- a string or numeric literal,
- a variable identifier,
- a label that point to a string data.

When two items are separated by a coma `,` a space will be inserted between them.

When two items are separated by semicolon `;` no space is inserted.

Example with different expressions:

    print "literal"
    v$="variable"
    print v$
    txt: data "data"
    print txt

Example with different separators:

    print "glu";"ed"
    print "sepa","rated"

    do
        wait 2
        print ".";
    loop

#### `PRINT`

Outputs a new line.

### Input API

LowResRMX support touch input with float-point precision (but not multi-touches), and limited keyboard input.

User can also rely on the OS virtual keyboard to capture typed characters.

#### `touched=TOUCH`

Return -1 if the device fantasy screen is currently `touched`.

Example that print a text only when device screen is touched:

    do
        cls
        if touch then text 4,4,"touched"
        wait vbl
    loop

#### `touched=TAP`

Return -1 for exactly one frame, if the device fantasy screen is `touched`.

Example of a flappy letter going down by gravity and up when tapping on the screen:

    sprite 0,0,shown.h/2,226
    gravity=3
    do
        wait vbl
        x=sprite.x(0)+1
        if x+8>shown.w then x=0
        if tap then gravity=-9
        y=clamp(sprite.y(0)+gravity,0,shown.h-8)
        gravity=min(3,gravity+1)
        sprite 0,x,y,
    loop

#### `x=TOUCH.X`<br>`y=TOUCH.Y`

Returns the last pixel position `x`,`y` touched. It returns a floating-point number, with a 1/16 pixel precisions.

    do
        cls
        print touch.x;",";touch.y
        wait vbl
    loop

#### `KEYBOARD ON`<br>`KEYBOARD OFF`

Show or hide the device virtual keyboard. Because user can hide the keybord using a dedicated key, do not assume the keyboard is visible. Use `KEYBOARD ON` inside a loop or rely on `=KEYBOARD` to detect when the keyboard is shown or not.

#### `height=KEYBOARD`

Return the `height` of the virtual screen that has been occluded by the keyboard.

#### `INPUT [prompt;]variable`<br>`INPUT [prompt;]variable$`

Wait for the user to type a text or a number and store it to the `variable` or `variable$`.

Optionally, a `prompt` can be printed on the screen before the user input.

    a=rnd(9)+10
    b=rnd(9)
    print "Captcha:"
    print str$(a);"+";str$(b)
    do
        input "?";answer
        if answer=a+b then goto pass
    loop
    pass: print "you may pass"

#### `pressed$=INKEY$`

Returns only once the last `pressed$` key. It's a string containing one ASCII character supported by LowResRMX. If no key was pressed or if the pressed key is not supported, it returns an empty string. TODO: link

Example that output the pressed key:

    do
        keyboard on
        k$=inkey$
        if k$<>"" then
            print k$;
        end if
        wait vbl
    loop

#### `frames=TIMER`

Returns the number of `frames` since LowResRMX was launched. The value wraps to 0 when 5184000 is reached, which is about 24 hours.

#### `WAIT TAP`

Will stop execution of the program until a touch is made.

While waiting for a tap, interrupt sub-routines for `VBL/RASTER/PARTICLE/EMITTER` are still executed.

### Display API

#### `PALETTE palette,[c0],[c1],[c2],[c3]`

Sets the four colors on the height available `palette` (0..7). The color 0 of the palette 0 is generally used as backdrop color. (It can be change using the `SYSTEM` command.) `c0`, `c1`, `c2`, `c3` can accept a numeric value between 0 and 63, omit them to keep the current value. Consult the [64 colors reference](#64-colors) to choose the color you want.

#### `=COLOR(palette,color)`

Returns one of the [64 colors](#64-colors) associated to the pair `palette` (0..7), `color` (0..3).

#### `width=SHOWN.W`<br>`height=SHOWN.H`

Returns the `width` and `height` in pixels of the visible area of the fantasy screen.

#### `left=SAFE.L`<br>`top=SAFE.T`<br>`right=SAFE.R`<br>`bottom=SAFE.B`

Returns the `left`, `top`, `right` and `bottom` offset in pixels to apply from the boundary of the fantasy device screen to reach the inner safe area specified by the device's operating system.

These functions can be used to avoid the top camera notch or the bottom inset full-width buttons.

#### `SPRITE VIEW ON`<br>`SPRITE VIEW OFF`

Enable or disable the rendering of all sprites.

#### `BG VIEW ON`<br>`BG VIEW OFF`

Enable or disable the rendering of all background layers.

#### `WAIT VBL`<br>`WAIT frame`

Wait for a number of `frame` to be drawn. Keyword `VBL` is similar to 1. This is used to:
- draw frames at 60 FPS,
- reduce device CPU usage,
- preserve device battery.

You probably MAY place at least one `WAIT VBL` inside every loop that wait for player input or to present something to the player.

Example that show the effect:

    sprite 0,shown.w/2-4,shown.h/2-4,240
    locate 2,2
    print "touch to wait vbl"
    do
        x=sprite.x(0)
        y=sprite.y(0)
        add x,1,-8 to shown.w
        add y,1,-8 to shown.h
        sprite 0,x,y,
        if touch then wait vbl
    loop

`WAIT VBL` is equal `WAIT 1`

#### `ON VBL CALL s`

Before the execution of the code for the next frame, will execute the sub-routine `s`.

The code MUST be short, read more about [CPU cycles](#cycles).

This can be used to execute code at every frame, independently of which part of the code is generating frames. Here is an example that demonstrate:

    sub v
        x=sprite.x(0)
        add x,1,-8 to shown.w
        y=sprite.y(0)
        add y,1,-8 to shown.h
        sprite 0,x,y,1
    end sub
    on vbl call v

    one:
    palette 0,43,,,
    wait tap
    goto two

    two:
    palette 0,59,,,
    wait tap
    goto one

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    0000000000000000FFFFFFFFFFFFFFFF

#### `ON VBL OFF`

Will stop the execution of a sub-routine during VBL interrupt.

#### `ON RASTER CALL routine`

Before the rendering of the next line of the screen, will execute a sub-`routine`. This is usefull in conjonction with [`=RASTER`](#raster).

The code MUST be shorterest, read more about [CPU cycles](#cycles).

This can be used to chance colors or scroll background layers, example:

    text 3,8,"tap to toggle"
    cell 10,10,1

    sub r
    if raster>=80 then
        scroll 0,0,80-raster
        palette 0,,,raster mod 64,
    else if raster=0 then
        scroll 0,0,0
        palette 0,,,0,
    end if
    end sub

    do
        wait tap
        on raster call r
        wait tap
        on raster off
        scroll 0,0,0
        palette 0,,,0,
    loop

#### `=RASTER`

Return the fantasy screen line number currently rendered.

#### `ON RASTER OFF`

Will stop the execution of a sub-routine during RASTER interrupt.

### Math API

#### `pi=PI`

Return the constant π.

#### `cosine=COS(number)`<br>`sine=SIN(number)`

Return the `cosine` or `sine` (0..1) value of a `number`.

`COS()` and `SIN()` take (0..1) instead of (0..π*2), and `SIN()` is inverted.

#### `arc=ATAN(x,y)`

Return the `arc` tangent value of a `x`,`y` vector.

`ATAN()` returns an value between (-0.5..0.5) making it suitable for use with `COS()` and `SIN()`.

Example that show how to compute an angle from a position, and a position from an angle:

    'get the center position of the screen
    cx=shown.w/2-4
    cy=shown.h/2-4
    sprite 50,cx,cy,1

    'initial position from the center
    dx=40
    dy=20
    sprite 40,cx+dx,cy+dy,1
    sprite 40 pal 1
    sprite 30 pal 2

    do
        'new position from touch
        dx=touch.x-cx
        dy=touch.y-cy
        sprite 40,cx+dx,cy+dy,1

        'get angle from vector
        a=atan(dx,dy)

        'get position from angle
        x=cos(a)*30
        y=sin(a)*30

        sprite 30,cx+x,cy+y,1
        wait vbl
    loop

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    3C7EFFFFFFFF7E3C0000000000000000

#### `absolute=ABS(number)`

Return the `absolute` value of a `number`.

#### `exponential=EXP(number)`

Return the `exponential` value of a `number`.

#### `logarithm=LOG(number)`

Return the `logarithm` value of a `number`.

#### `square=SQR(number)`

Return the `square` root value of a `number`.

#### `sign=SGN(number)`

Return the `sign` (+1/-1) of a `number`.

#### `floor=FLOOR(number)<br>floor=INT(number)`

Return the `floor` value of a `number`.

#### `ceil=CEIL(number)`

Return the `ceil` value of a `number`.

#### `INC variable`<br>`DEC variable`

Increment or decrement by 1 the value of a `variable`.

#### `ADD variable,value`<br>ADD variable,increment,min TO max`

Add a `value` to a `variable`.

Optionally make it wrap around between `min` and `max`.

#### `minimal=MIN(a,b)`<br>`maximal=MAX(a,b)`

Return the `minimal` or `maximal` value between `a` and `b`.

#### `clamped=CLAMP(value,min,max)`

Return the `value` `clamped` between `min` and `max`.

#### `length=LEN(x,y)`

Return the `length` of a `x`,`y` vector.

#### `interpolation=EASE(function,mode,value)`

This returns the `interpolation` of `value` between 0.0 and 1.0, using the 🌐[specified easing](https://easings.net/) `function` and `mode`.

|     | function |
| ---:| -------- |
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
| ---:| ----- |
|  -1 | in    |
|   0 | inout |
|  +1 | out   |

#### `random=RND`

Return a `random` floating-point number (0..1).

#### `random=RND(max)`

Return a `random` integer number (0..max).

#### `RANDOMIZE seed`

Initialize the random generator with a different `seed` (0..16777216).

TODO: link to floating point precision loss

Setting a specific `seed` will guarantee to return the same sequence of random numbers.

    randomize 123456
    for i=0 to 5
        print rnd(100)
    next i

    print

    randomize 123456
    for i=0 to 5
        print rnd(100)
    next i

#### `random=RND address<br>random=RND(max,address)<br>RANDOMIZE seed[,address]`

Similar to the original functions with a addional `address` parameter, where the current generator state will be stored.

This is usefull if you want to keep multiple generator that still need to be determistic.

The generator state consume 16 Bytes of memory.

### String API

#### `lenght=LEN(text)`

Return the `lenght` in ascii7 character of a `text` string.

#### `ascii7=ASC(character)`

Converts the first `character` of a string to an `ascii7` code.

TODO: link to ascii7 table

#### `character=CHR$(ascii7)`

Converts an `ascii7` code to a one `character` string.

#### `text=STR$(number)`

Converts a `number` to a `text` string.

    print str$(123.456)

#### `number=VAL(text)`

Converts a `text` string that starts with digits (and maybe one point `.`) into a `number`.

    print val("3.5abc")

#### `hexadecimal=HEX$(number,[digits])`<br>`binary=BIN$(number,[digits])`

Converts a `number` to an `hexadecimal` or `binary` string with an optional minimal number of `digits`.

#### `leading$=LEFT$(text,length)`<br>`trailing$=RIGHT$(text,length)`

Return the `length` number of characters from the beginning or the end of the `text` string.

    shadok: data "gabuzomeu"
    print left$(shadok,4)
    print right$(shadok,5)

#### `LEFT$(text,length)=replacement$`<br>`RIGHT$(text,length)=replacement$`

Overwrites the `length` number of characters at the beginning or the end of the `text` string by the `length` first characters of the `replacement$` string.

#### `extract$=MID(text,position,length)`

Return the `length` number of characters starting at `position` (1..) from the `text` string.

#### `mid$(text,position,length)=replacement$`

Overwrites the `length` number of characters starting at `position` (1..) from the `text` string by the `length` first characters of the `replacement$` string.

    six$="hexakosioihexekontahexaphobie"
    mid$(six$,1,4)="-66-"
    mid$(six$,11,4)="-66-"
    mid$(six$,20,4)="-66-"
    print six$

### Sound API

TODO: Link to how sound work

#### `PLAY voice,pitch[,length] [SOUND sound]`

Play a note at the `pitch` on the `voice` with an optional `length` and `sound`.

Check the [pitch references](#pitch-values>) to learn which pitch conrespond to which notes.

`length` use 1/60 of seconds as units with the maximum being 255, so 4.25 seconds. A length of 0 means that the sound will not stop until another sound is played on the same `voice`.

`sound` is a number representing a group of settings that define the sound's characteristics. It requires the usage of the [`SOUND SOURCE`](<manual#`SOUND SOURCE [address]`>) command.

#### `STOP`<br>`STOP [voice]`

Stops the sound and track on the `voice` or all the voices if omitted.

Release duration of envelope are not stop but fade out instead.

#### `VOLUME voice,[volume],[mix]`

Sets the `volume` and `mix` of `voice`.

| parameter | value and range                                |
|----------:|------------------------------------------------|
|  `volume` | `0` .. `15`                                    |
|     `mix` | `0` Muted<br>`1` Left<br>`2` Right<br>`3` Both |

All parameters can be omitted to keep their current settings.

#### `SOUND voice,[wave],[width],[length]`

Sets the sound's characteristics for the `voice`.

`wave` control the waveform. With a waveform of Pulse, the `width` control the pulse width. A value of 8 means a square wave.

`length` control the length of the sound. A length of 0 means that the sound will not stop until another sound is played on the same `voice`. This value can be overriden by the parameter `length` of the [`PLAY` command](#play-voice-pitch-length-sound-sound).

| parameter | value and range                                        |
|----------:|--------------------------------------------------------|
|    `wave` | `0` Sawtooth<br>`1` Triangle<br>`2` Pulse<br>`3` Noise |
|   `width` | `0` .. `15`                                            |
|  `length` | `0` Infinite<br>1 16.67ms .. `255` 4.25s               |

Omitted parameters will keep their previous values.

#### `ENVELOPE voice,[attack],[decay],[sustain],[release]`

Set the volume envelope for the `voice`.

Allow to change the `attack`, `decay` and `release` duration.

`sustain` control the volume after the decay and before the release.

| parameter | range               |
|----------:|---------------------|
|  `attack` | `0` 2ms .. `15` 12s |
|   `decay` | `0` 2ms .. `15` 12s |
| `release` | `0` 2ms .. `15` 12s |
| `sustain` | `0` .. `15`         |

All parameters can be omitted to keep their current settings.

#### `LFO voice,[rate],[frequency],[volume],[width]`

Set the Low Frequency Oscillator (LFO) for the `voice`.

Allow to change the `rate`, the `frequency`, the `volume` and the `width` for Pulse waveform.

|   parameter | range                   |
|------------:|-------------------------|
|      `rate` | `0` 0.12Hz .. `15` 18Hz |
| `frequency` | `0` .. `15`             |
|    `volume` | `0` .. `15`             |
|     `width` | `0` .. `15`             |

All parameters can be omitted to keep their current settings.

#### `LFO WAVE voice,[wave],[invert],[env],[trigger]`

Set options for the a second Low Frequency Oscillator (LFO) for the `voice`.

Allow to change the `wave` and enable or disable `invert`, `env` and `trigger`.

With `invert` off, the wave is added (+) to the output signal. With `invert` on, the wave is substracted (-).

When `env` on, this second LFO will be played once. Will implictly set `trigger` on.

With `trigger` on, this second LFO is restarted at every `PLAY`, otherwise the LFO is continously applied.

| parameter | value                                                    |
|----------:|----------------------------------------------------------|
|    `wave` | `0` Triangle<br>`1` Sawtooth<br>`2` Square<br>`3` Random |
|  `invert` | `0` Off<br>`1` On                                        |
|     `env` | `0` Off<br>`1` On                                        |
| `trigger` | `0` Off<br>`1` On                                        |

All parameters can be omitted to keep their current settings.

#### `SOUND SOURCE [address]`

Set the `address` on memory to use as source for `MUSIC`, `TRACK` and `PLAY` commands. The [official sound data format]() TODO: link is used to decode the informations.

This will not affect already started playback.

If not specified the default address is taken by internally executing `=ROM(15)`

#### `MUSIC [pattern]`

Starts playing at the `pattern`. If omitted, it starts at pattern `0`.

This will consider that the data respects the [official soud data format]() TODO: link.

#### `=MUSIC(what)`

Question the current playback.

| `what` |                                       |
|-------:|---------------------------------------|
|      0 | The current pattern                   |
|      1 | The current row                       |
|      2 | The current tick                      |
|      3 | The current speed<br>`0` when stopped |

#### `TRACK track,voice`

Starts playing the `track` on `voice`.

This will consider that the data respects the [official soud data format]() TODO: link.

### Memory API

#### `POKE address,value`

Write an 8bits `value` (0..255) in memory at `address`.

The `address` must be writable. TODO: link

Example that change the background color:

    c=0
    do
        poke $ff00,C
        add c,1,0 to 63
        wait 5
    loop

#### `=PEEK(address)`

Read and return a 8bits value (0..255) from memory at `address`.

Example that print the last pressed ASCII key code:

    do
        keyboard on
        print str$(peek($ff84))
        wait 5
    loop

#### `POKEW address,value`

Write a 16bits `value` (-32768..32767) in memory at `address`.

#### `=PEEKW(address)`

Read and return a 16bits value (-32768..32767) from memory at `address`.

Example that print the width and height of the visible pixels:

    print "width",peekw($ff78),shown.w
    print "height",peekw($ff7a),shown.h

#### `POKEL address,value`

Write a 32bits `value` value (-16777216..16777216) in memory at `address`.

TODO: link to floating-point precision loss

#### `=PEEKL(address)`

Read and return a 32bits value (-16777216..16777216) from memory at `address`.

TODO: link to floating-point precision loss

#### `address=ROM(file)`

Return the `address` in memory of the virtual `file`.

#### `size=SIZE(file)`

Return the `size` in bytes of the virtual `file`.

#### `COPY source,count TO destination`

Copies `count` bytes from `source` address from memory to `destination` address in memory.

The `source` and `destination` CAN overlap.

#### `FILL address,count[,value]`

Sets `count` bytes from `address` in memory with the `value` or 0 when not specified.

#### `ROL address,places`<br>`ROR address,places`

Rotates the bits of the byte stored at `address` by a number of `places` to the left or the right.

    poke $9000,%00001111
    again:
    print right$("0000000"+bin$(peek($9000)),8)
    wait tap
    rol $9000,2
    goto again

#### `DMA COPY [ROM]`

Performs a fast memory copy that can only be done during interrupt calls: `VBL/RASTER/EMITTER/PARTICLE`.

It uses the following registers as data:

|    addr | size    | purpose             |
| -------:| ------- | ------------------- |
| `$FFA0` | 2 Bytes | Source address      |
| `$FFA2` | 2 Bytes | Bytes count         |
| `$FFA4` | 2 Bytes | Destination address |

To copy from an address greater or equal than `$10000`, the optional `[ROM]` argument MUST be used.

### File API

Give access to the [virtual file embeded](#virtual-file) in the cartridge.

#### `FILES`

Enable the access to the files, allowing to use the other commands of the file API.

Getting access to files allow to create editor program, like characters, map or custom editor. The official GFX and SFX use this command.

When the `FILES` command is called, it will map the data stored in the fantasy cartridge to virtual files indexed from 0 to 15.

#### `comment$=FILE$(file)`

Return the `comment$` string of the `file`.

The command [`FILES` MUST be called](#files) before.

#### `size=FSIZE(file)`

Return the `size` in bytes stored in the `file`.

The command [`FILES` MUST be called](#files) before.

#### `LOAD file,address[,limit[,offset]]`

Load the virtual `file` at `address`. Optionnaly `limit` the number of bytes read and start at `offset`.

The command [`FILES` MUST be called](#files) before.

#### `SAVE file,comment$,address,size`

Save `size` bytes from `address` to the `file` and **erase previous data**. The `comment$` is just a reminder of what is stored.

The command [`FILES` MUST be called](#files) before.

### Other API

#### `TRACE expression[,expression...]`

Output the evaluated `expression` on the [overlay](#overlay).

#### `MESSAGE text`

Output `text` in the bottom-left corner of the fantasy screen, in the [overlay](#overlay).

#### `SYSTEM setting,value`

Sets the `value` to system `setting`.

| setting | purpose                     | values |
| -------:|:--------------------------- |:------ |
|       0 | energy saving mode          | 0 or 1 |
|       1 | color 0 opacity for layer 0 | 0 or 1 |
|       2 | color 0 opacity for layer 1 | 0 or 1 |
|       3 | color 0 opacity for layer 2 | 0 or 1 |
|       4 | color 0 opacity for layer 3 | 0 or 1 |
|       5 | double size for layer 0     | 0 or 1 |
|       6 | double size for layer 1     | 0 or 1 |
|       7 | double size for layer 2     | 0 or 1 |
|       8 | double size for layer       | 0 or 1 |

Enabling the _energy saving mode_ setting will reduce the refresh rate whenever there is no user input. The CPU cycles are not affected.

Enabling the _color 0 opacity_ will make the color 0 of each palette non transparent.

> It makes no sens for layer 0.

Enabling the _double size_ will make the background layer rendered at twice the size. The scrolling values are not affected.

#### `HAPTIC pattern`

Trigger an haptic feedback on the device make it vibrate using a `pattern` (0..9).

| pattern | feeling   |
| -------:| --------- |
|       0 |           |
|       1 | tok-tirti |
|       2 | tok-tik   |
|       3 | took-ti   |
|       4 | took      |
|       5 | tik       |
|       6 | tok       |
|       7 | ti        |
|       8 | tf        |
|       9 | t         |

Not supported on all devices.

#### `COMPAT`

Enables compatibility mode:

- Forces the rendering process to keep the original device screen.
- Reverts the `RND` command and `=RND()` function to their original behavior.
- Maybe other things I can't remember.

This does not guarantee full compatibility but can help with some aspects. For instance:
- Commands and functions that have been removed will still be unavailable.
- Double cell support for background is not emulated.

> This command will be removed at some point.

#### `PAUSE`

Halt the execution of bring the debugger console.

Check the [debugger instructions](#debugger-instructions) to learn what you can to do with it.

## Debugger instructions

#### dbg: `PAUSE`

Enter the debugger. It bring a console where user can enter debugger specific commands. The scope used in the debugger is the same as the one in the program where the `PAUSE` appear.

#### dbg: a variable name

By typing the name of a variable, the debugger will print it's value. The variable use the same syntax as inside a program: `$` for string, `()` for array.

The variable must be accessible throughout the scope of where the `PAUSE` has been used to enter the debugger. Global variables are still available everywhere.

#### dbg: a variable name `=` new value

Allow to change the value of a variable.

Number literal use the same syntax as inside a program, it support integer, float, `$` hexadecimal and `%` binary.

String literal should use the same syntax as inside a program: `"`.

#### dbg: an address

By typing a memory address, the debugger will print it's value as if it was read by `peek()`.

The address can be indiquated using the `$` hexadecimal or by any other valid numeric literal.

#### dbg: an address `=` a value

Will try to modify the value store inside a memory address.

    `$FF00=3`

#### dbg: `CLS`

Clear the debugger console.

#### dbg: `WAIT`

Resume execution until a `WAIT` command is found in the program, the scope may change.

#### dbg: `DIM [filter] [pagination]`

Print the list of accessible variable at the current scope.

Allow to limit the output to the variables that matchs the `[filter]`.

Allow to output more variables using the `[pagination]` with an index that start at `0` zero.

#### dbg: `TRACE`

Print the current call stack in order: label and procedure names.

## References

This anex provides technical informations about how things work under the hood.

### 64 Colors

<style>
.famicube{display:flex;flex-wrap:wrap}
.famicube div{font-family:monospace;padding:0.8em 1em;width:2em}
.famicube div:nth-child(1){background:#000000;color:#fff}
.famicube div:nth-child(2){background:#e03c28;color:#000}
.famicube div:nth-child(3){background:#ffffff;color:#000}
.famicube div:nth-child(4){background:#d7d7d7;color:#000}
.famicube div:nth-child(5){background:#a8a8a8;color:#000}
.famicube div:nth-child(6){background:#7b7b7b;color:#fff}
.famicube div:nth-child(7){background:#343434;color:#fff}
.famicube div:nth-child(8){background:#151515;color:#fff}
.famicube div:nth-child(9){background:#0d2030;color:#fff}
.famicube div:nth-child(10){background:#415d66;color:#fff}
.famicube div:nth-child(11){background:#71a6a1;color:#000}
.famicube div:nth-child(12){background:#bdffca;color:#000}
.famicube div:nth-child(13){background:#25e2cd;color:#000}
.famicube div:nth-child(14){background:#0a98ac;color:#fff}
.famicube div:nth-child(15){background:#005280;color:#fff}
.famicube div:nth-child(16){background:#00604b;color:#fff}
.famicube div:nth-child(17){background:#20b562;color:#000}
.famicube div:nth-child(18){background:#58d332;color:#000}
.famicube div:nth-child(19){background:#139d08;color:#fff}
.famicube div:nth-child(20){background:#004e00;color:#fff}
.famicube div:nth-child(21){background:#172808;color:#fff}
.famicube div:nth-child(22){background:#376d03;color:#fff}
.famicube div:nth-child(23){background:#6ab417;color:#000}
.famicube div:nth-child(24){background:#8cd612;color:#000}
.famicube div:nth-child(25){background:#beeb71;color:#000}
.famicube div:nth-child(26){background:#eeffa9;color:#000}
.famicube div:nth-child(27){background:#b6c121;color:#000}
.famicube div:nth-child(28){background:#939717;color:#fff}
.famicube div:nth-child(29){background:#cc8f15;color:#000}
.famicube div:nth-child(30){background:#ffbb31;color:#000}
.famicube div:nth-child(31){background:#ffe737;color:#000}
.famicube div:nth-child(32){background:#f68f37;color:#000}
.famicube div:nth-child(33){background:#ad4e1a;color:#fff}
.famicube div:nth-child(34){background:#231712;color:#fff}
.famicube div:nth-child(35){background:#5c3c0d;color:#fff}
.famicube div:nth-child(36){background:#ae6c37;color:#000}
.famicube div:nth-child(37){background:#c59782;color:#000}
.famicube div:nth-child(38){background:#e2d7b5;color:#000}
.famicube div:nth-child(39){background:#4f1507;color:#fff}
.famicube div:nth-child(40){background:#823c3d;color:#fff}
.famicube div:nth-child(41){background:#da655e;color:#000}
.famicube div:nth-child(42){background:#e18289;color:#000}
.famicube div:nth-child(43){background:#f5b784;color:#000}
.famicube div:nth-child(44){background:#ffe9c5;color:#000}
.famicube div:nth-child(45){background:#ff82ce;color:#000}
.famicube div:nth-child(46){background:#cf3c71;color:#fff}
.famicube div:nth-child(47){background:#871646;color:#fff}
.famicube div:nth-child(48){background:#a328b3;color:#fff}
.famicube div:nth-child(49){background:#cc69e4;color:#000}
.famicube div:nth-child(50){background:#d59cfc;color:#000}
.famicube div:nth-child(51){background:#fec9ed;color:#000}
.famicube div:nth-child(52){background:#e2c9ff;color:#000}
.famicube div:nth-child(53){background:#a675fe;color:#000}
.famicube div:nth-child(54){background:#6a31ca;color:#fff}
.famicube div:nth-child(55){background:#5a1991;color:#fff}
.famicube div:nth-child(56){background:#211640;color:#fff}
.famicube div:nth-child(57){background:#3d34a5;color:#fff}
.famicube div:nth-child(58){background:#6264dc;color:#000}
.famicube div:nth-child(59){background:#9ba0ef;color:#000}
.famicube div:nth-child(60){background:#98dcff;color:#000}
.famicube div:nth-child(61){background:#5ba8ff;color:#000}
.famicube div:nth-child(62){background:#0a89ff;color:#fff}
.famicube div:nth-child(63){background:#024aca;color:#fff}
.famicube div:nth-child(64){background:#00177d;color:#fff}
</style>
<div class="famicube">
    <div>00</div><div>01</div><div>02</div><div>03</div><div>04</div><div>05</div><div>06</div><div>07</div>
    <div>08</div><div>09</div><div>10</div><div>11</div><div>12</div><div>13</div><div>14</div><div>15</div>
    <div>16</div><div>17</div><div>18</div><div>19</div><div>20</div><div>21</div><div>22</div><div>23</div>
    <div>24</div><div>25</div><div>26</div><div>27</div><div>28</div><div>29</div><div>30</div><div>31</div>
    <div>32</div><div>33</div><div>34</div><div>35</div><div>36</div><div>37</div><div>38</div><div>39</div>
    <div>40</div><div>41</div><div>42</div><div>43</div><div>44</div><div>45</div><div>46</div><div>47</div>
    <div>48</div><div>49</div><div>50</div><div>51</div><div>52</div><div>53</div><div>54</div><div>55</div>
    <div>56</div><div>57</div><div>58</div><div>59</div><div>60</div><div>61</div><div>62</div><div>63</div>
</div>

### Memory mapping

| address  | size        | purpose            |
| --------:| ----------- | ------------------ |
| `$00000` | 8 Kibibyte  | Layer 0 data       |
| `$02000` | 8 Kibibyte  | Layer 1 data       |
| `$04000` | 8 Kibibyte  | Layer 2 data       |
| `$06000` | 8 Kibibyte  | Layer 3 data       |
| `$08000` | 4 Kibibyte  | Character data     |
| `$09000` | 20 Kibibyte | Working RAM        |
| `$0E000` | 6 Kibibyte  | Persisent RAM      |
| `$0FB00` | 1020 Bytes  | Sprite registers   |
| `$0FF00` | 32 Bytes    | Color registers    |
| `$0FF20` | 10 Bytes    | Video registers    |
| `$0FF40` | 48 Bytes    | Audio registers    |
| `$0FF70` | 28 Bytes    | I/O registers      |
| `$0FFA0` | 6 Bytes     | DMA registers      |
| `$0FFA6` | 10 Bytes    | Internal registers |
| `$10000` | 64 Kibibyte | Cartridge ROM      |

TODO: Add particle/emitter registers

### TODO: Color data

### Background data

The 4 background layers use the following format:
- 64x64 cells
- 2 bytes per cell

For each cell:

| address | purpose                   |
| -------:| ------------------------- |
|  addr+0 | character number (0..255) |
|  addr+2 | cell attributes           |

### Cell attributes

|    bit mask | purpose                |
| -----------:| ---------------------- |
| `%00000111` | palette number (0..7)  |
| `%00001000` | horizontal flip (0..1) |
| `%00010000` | vertical flip (0..1)   |
| `%00100000` | priority flag (0..1)   |

_the last 2 bits are unused_

### Character data

The 256 characters use the following format:
- 8x8 pixels
- 2 bits per pixel
- 16 bytes per character
- 🌐[bit plane encoded](https://en.wikipedia.org/wiki/Bit_plane)

The pixels are encoded bit per bit, from left to right, then top to bottom, one plane at a time.
The 1st 8 bytes store the low bit for all the 8x8 pixels. The 2nd 8 bytes store the high bit.

### Background source format data

When using [`BG SOURCE address[,width,height]`](#bg-source-address-width-height) without specifing `width` and `height`, A specific header is added before the regular [background data](#background-data):

| address | size   | purpose         |
| -------:| ------ | --------------- |
|  addr+0 | 1 Byte | always zero     |
|  addr+1 | 1 Byte | always zero     |
|  addr+2 | 1 Byte | width in cell   |
|  addr+3 | 1 Byte | height in cell  |
|  addr+4 | ...    | background data |

### Sound source format data

When using [`SOUND SOURCE`](<manual#`SOUND SOURCE [address]`>), LowResRMX will use the following data format to store sounds, patterns and tracks.

TODO: continue

|  address | purpose          |
| --------:| ---------------- |
|   addr+0 | 16 sound presets |
| addr+128 | 64 patterns      |
| addr+384 | 64 tracks        |

For each sound preset:

| address | size | purpose                                 |
| -------:| ---- | --------------------------------------- |
|  addr+0 | 1    | [Attributes](<manual#Attributes bits:>) |
|  addr+1 | 1    | Length                                  |
|  addr+2 | 2    | [Envelope](<manual#Envelope bits:>)     |
|  addr+4 | 1    | LFO attributes                          |
|  addr+5 | 2    | LFO settings                            |
|  addr+7 | 1    | Not used                                |

#### Attributes bits:

| bits | purpose                                                        |
| ----:| -------------------------------------------------------------- |
| 0..3 | Pulse width                                                    |
| 4..5 | Wave<br>`0` Sawtooth<br>`1` Triangle<br>`2` Pulse<br>`3` Noise |
|    6 | Timeout enabled                                                |

#### Envelope bits:

|   bits | purpose         |
| ------:| --------------- |
|   0..3 | Attack duration |
|   4..7 | Decay duration  |
|  8..11 | Sustain volume  |
| 12..15 | Decay volume    |


TODO: every attributes and settings

### Pitch values

| alpha | sylla | pitch per octave        |
| -----:| -----:| ----------------------- |
|     C |    Do | 01 13 25 37 49 61 73 85 |
|    C# |   #Do | 02 14 26 38 50 62 74 86 |
|     D |    Re | 03 15 27 39 51 63 75 87 |
|    D# |   #Re | 04 16 28 40 52 64 76 88 |
|     E |    Mi | 05 17 29 41 53 65 77 89 |
|     F |    Fa | 06 18 30 42 54 66 78 90 |
|    F# |   #Fa | 07 19 31 43 55 67 79 91 |
|     G |   Sol | 08 20 32 44 56 68 80 92 |
|    G# |  #Sol | 09 21 33 45 57 69 81 93 |
|     A |    La | 10 22 34 46 58 70 82 94 |
|    A# |   #La | 11 23 35 47 59 71 83 95 |
|     B |    Si | 12 24 36 48 60 72 84 96 |

### ASCII table

LowResRMX understand ASCII7 characters (0..127).

The [text API](#text-api) can only print a fraction of it and remap it. The [`FONT` command](#font-first) is used to indiquate which characters are use to print the ASCII characters.

| ascii code | usage                         |
| ----------:| ------------------------------|
|      0..31 | not used                      |
|     32..95 | used by [`FONT`](#font-first) |
|    95..255 | not used                      |

| code | hexa | character or purpose |
| ----:| ----:| -------------------- |
|   17 |  $11 | right arrow          |
|   18 |  $12 | left arrow           |
|   19 |  $13 | down arrow           |
|   20 |  $14 | up arrow             |
|   32 |  $20 | whitespace           |
|   33 |  $21 | "!"                  |
|   34 |  $22 | '"'                  |
|   35 |  $23 | "#"                  |
|   36 |  $24 | "$"                  |
|   37 |  $25 | "%"                  |
|   38 |  $26 | "&"                  |
|   39 |  $27 | "'"                  |
|   40 |  $28 | "("                  |
|   41 |  $29 | ")"                  |
|   42 |  $2A | "*"                  |
|   43 |  $2B | "+"                  |
|   44 |  $2C | ","                  |
|   45 |  $2D | "-"                  |
|   46 |  $2E | "."                  |
|   47 |  $2F | "/"                  |
|   48 |  $30 | "0"                  |
|   49 |  $31 | "1"                  |
|   50 |  $32 | "2"                  |
|   51 |  $33 | "3"                  |
|   52 |  $34 | "4"                  |
|   53 |  $35 | "5"                  |
|   54 |  $36 | "6"                  |
|   55 |  $37 | "7"                  |
|   56 |  $38 | "8"                  |
|   57 |  $39 | "9"                  |
|   58 |  $3A | ":"                  |
|   59 |  $3B | ";"                  |
|   60 |  $3C | "<"                  |
|   61 |  $3D | "="                  |
|   62 |  $3E | ">"                  |
|   63 |  $3F | "?"                  |
|   64 |  $40 | "@"                  |
|   65 |  $41 | "A"                  |
|   66 |  $42 | "B"                  |
|   67 |  $43 | "C"                  |
|   68 |  $44 | "D"                  |
|   69 |  $45 | "E"                  |
|   70 |  $46 | "F"                  |
|   71 |  $47 | "G"                  |
|   72 |  $48 | "H"                  |
|   73 |  $49 | "I"                  |
|   74 |  $4A | "J"                  |
|   75 |  $4B | "K"                  |
|   76 |  $4C | "L"                  |
|   77 |  $4D | "M"                  |
|   78 |  $4E | "N"                  |
|   79 |  $4F | "O"                  |
|   80 |  $50 | "P"                  |
|   81 |  $51 | "Q"                  |
|   82 |  $52 | "R"                  |
|   83 |  $53 | "S"                  |
|   84 |  $54 | "T"                  |
|   85 |  $55 | "U"                  |
|   86 |  $56 | "V"                  |
|   87 |  $57 | "W"                  |
|   88 |  $58 | "X"                  |
|   89 |  $59 | "Y"                  |
|   90 |  $5A | "Z"                  |
|   91 |  $5B | "["                  |
|   92 |  $5C | "\"                  |
|   93 |  $5D | "]"                  |
|   94 |  $5E | "^"                  |
|   95 |  $5F | "_"                  |
|   96 |  $60 | "`"                  |
|   97 |  $61 | "a"                  |
|   98 |  $62 | "b"                  |
|   99 |  $63 | "c"                  |
|  100 |  $64 | "d"                  |
|  101 |  $65 | "e"                  |
|  102 |  $66 | "f"                  |
|  103 |  $67 | "g"                  |
|  104 |  $68 | "h"                  |
|  105 |  $69 | "i"                  |
|  106 |  $6A | "j"                  |
|  107 |  $6B | "k"                  |
|  108 |  $6C | "l"                  |
|  109 |  $6D | "m"                  |
|  110 |  $6E | "n"                  |
|  111 |  $6F | "o"                  |
|  112 |  $70 | "p"                  |
|  113 |  $71 | "q"                  |
|  114 |  $72 | "r"                  |
|  115 |  $73 | "s"                  |
|  116 |  $74 | "t"                  |
|  117 |  $75 | "u"                  |
|  118 |  $76 | "v"                  |
|  119 |  $77 | "w"                  |
|  120 |  $78 | "x"                  |
|  121 |  $79 | "y"                  |
|  122 |  $7A | "z"                  |

### Registers

Registers are one or more bytes mapped in memory thats as an internal usage.

#### Character Registers

Each one of the 256 character occupies 16 bytes.

| address | size     | purpose         |
| -------:| -------- | --------------- |
| `$8000` | 16 Bytes | 1st character   |
| `$8010` | 16 Bytes | 2nd character   |
| `$8020` | 16 Bytes | 3rd character   |
|       … | 16 Bytes | …               |
| `$8FE0` | 16 Bytes | 255th character |
| `$8FF0` | 16 Bytes | 256th character |

#### Sprite Registers

There are 170 sprites, each occupies 6 bytes:

| address | size    | purpose      |
| -------:| ------- | ------------ |
| `$FB00` | 6 Bytes | 1st sprite   |
| `$FB06` | 6 Bytes | 2nd sprite   |
| `$FB0C` | 6 Bytes | 3rd sprite   |
|       … |         | …            |
| `$FEF6` | 6 Bytes | 170th sprite |
| `$FEFC` | 4 Bytes | Not used     |

For each sprite:

| offset | size    | purpose              |
| ------:| ------- | -------------------- |
|     +0 | 2 Bytes | Position on X axis   |
|     +2 | 2 Bytes | Position on Y axis   |
|     +4 | 1 Byte  | Character number     |
|     +5 | 1 Byte  | Character attributes |

Both position on x and y axis use sub-pixels values. To advance by 1 pixel, the values should get increased by 16.
Also, they are both offseted by 32 pixels. To place a sprite in the 0x0 coordinates, the values should be 512x512.

#### Character attributes

| bits | purpose         |
| ----:| --------------- |
| 0..2 | Palette number  |
|    3 | Horizontal flip |
|    4 | Vertical flip   |
|    5 | Priority        |
| 6..7 | Size            |

Sprite size:

|   binary | purpose                        |
| --------:| ------------------------------ |
|    `%00` | 8x8 pixels or 1x1 character    |
|    `%01` | 16x16 pixels or 2x2 characters |
|    `%10` | 24x24 pixels or 3x3 characters |
|    `%11` | 32x32 pixels or 4x4 characters |

#### Color registers

There are 8 palettes with 4 colors each:

|    addr | size     | purpose         |
| -------:| -------- | --------------- |
| `$FF00` | 32 Bytes | Color registers |

For each palette:

| offset | size   | purpose         |
| ------:| ------ | --------------- |
|     +0 | 1 Byte | 1st color value |
|     +1 | 1 Byte | 2nd color value |
|     +2 | 1 Byte | 3rd color value |
|     +3 | 1 Byte | 4th color value |

#### Video registers

|    addr | size     | purpose                     |
| -------:| -------- | --------------------------- |
| `$FF20` | 2 Bytes  | Background layer 0 scroll X |
| `$FF22` | 2 Bytes  | Background layer 0 scroll Y |
| `$FF24` | 2 Bytes  | Background layer 1 scroll X |
| `$FF26` | 2 Bytes  | Background layer 1 scroll Y |
| `$FF28` | 2 Bytes  | Background layer 2 scroll X |
| `$FF2A` | 2 Bytes  | Background layer 2 scroll Y |
| `$FF2C` | 2 Bytes  | Background layer 3 scroll X |
| `$FF2E` | 2 Bytes  | Background layer 3 scroll Y |
| `$FF30` | 2 Bytes  | Raster line number          |
| `$FF32` | 1 Byte   | Display attributes          |
| `$FF33` | 12 Bytes | Not used                    |

#### Display attributes

| bits | purpose                    |
| ----:| -------------------------- |
|    0 | Sprites enabled            |
|    1 | Background layer 0 enabled |
|    2 | Background layer 1 enabled |
|    3 | Background layer 2 enabled |
|    4 | Background layer 3 enabled |

#### TODO: Audio registers

|    addr | size     | purpose         |
| -------:| -------- | --------------- |
| `$FF40` | 48 Bytes | Audio registers |

#### I/O registers

|    addr | size    | purpose                        |
| -------:| ------- | ------------------------------ |
| `$FF70` | 4 Bytes | Last touch position X          |
| `$FF74` | 4 Bytes | Last touch position Y          |
| `$FF78` | 2 Bytes | Pixels shown in width          |
| `$FF7a` | 2 Bytes | Pixels shown in height         |
| `$FF7c` | 2 Bytes | Pixels outside the safe zone   |
| `$FF7e` | 2 Bytes | Pixels outside the safe zone   |
| `$FF80` | 2 Bytes | Pixels outside the safe zone   |
| `$FF82` | 2 Bytes | Pixels outside the safe zone   |
| `$FF84` | 1 Byte  | ASCII code of last pressed key |
| `$FF85` | 1 Byte  | Other I/O status bits          |

Last touch position X and Y are stored as float and currently LowResRMX do not have a way to peek float from memory, use [`TOUCH.X` and `TOUCH.Y`](#xtouchxytouchy) functions instead.

Pixels shown represent the number of fantasy pixels that is visible by the user according to their device screen ratio. Use the practicle [`SHOWN.W` and `SHOWN.H`](#widthshownxheightshownh) functions.

Pixels outside the safe zone represent the number of fantasy pixels that are visible but should be considered unsafe for touch input as they are outside the safe area. Use the easy memorable [`SAFE.L`, `SAFE.T`, `SAFE.R` and `SAFE.B`](#leftsafeltopsafetrightsaferbottomsafeb) functions.

#### Other I/O status bits

| bits | purpose                                   |
| ----:| ----------------------------------------- |
|    0 | Pause currently active                    |
|    1 | Fantasy screen currently touched          |
|    2 | Device virtual keyboard currently visible |

#### DMA registers

|    addr | size    | purpose                 |
| -------:| ------- | ----------------------- |
| `$FFA0` | 2 Bytes | Source address          |
| `$FFA2` | 2 Bytes | Number of bytes to copy |
| `$FFA4` | 2 Bytes | Destination address     |

### Cycles

The fantasy hardware simulates CPU cycles. A cycle a fixed time duration that can be used to compute things. In LowResRMX each instructions cost a number of cycles that follow a set of rules.

1. 1 cycle per instruction,
2. 1 cycle for reading the value of a variable,
3. 1 cycle per operator in expression evaluation,
4. 1 cycle per array item creation,
5. 1 cycle per function execution,
6. 1 cycle per literals evaluation,
7. 1 cycle per label read as value,
8. 1 cycle per command execution,
9. 8 cycles for copying sound data for each sound play,
10. 1 cycle per character for string creation, modification, concatenation,
11. 1 cycle per byte for memory copy or modification (except `DMA COPY`),
12. almost 1/32 cycle per byte copied using `DMA COPY`,
13. 2 cycles per cells modification including text.

Limit of cycles a program can execute:
- 52668 per frames

During interrupt, addional limits are also applied:
- 3420 for VBL interrupt
- 204 per line for RASTER interrupt
- 51 per particle for PARTICLE interrupt
- 102 per emitter for EMITTER interrupt

### Number limits

LowResRMX internally store numbers as 🌐[floating point](https://en.wikipedia.org/wiki/IEEE_754) on 32 bits.

This enough to store integers values (-16777216..16777216) with a 1 to 1 precision.

With lower or bigger numbers, precision will decrease, making those number equal.

    if 16777216=16777217 then
        print "yes"
    else
        print "no"
    end if

With float numbers, the precision max depends on whatever the programs want. It is not clear for me.

    print 4.000000299=4
    print 4.000000300<>4

    print 1.0000000599=1
    print 1.0000000600<>1

    print 0.000000235=0
    print 0.000000239<>0

### Language limits

1. 16384 max tokens per program.

    A token is one identifier, one number or one symblols.

2. 2048 max symbols per program.

    A symbol is an identifier that was defined by the user: a variable, procedure or label name.

3. 20 max characters per identifier

4. 128 max jump stack.

    The jump stack is increased at each `GOSUB` but also internally by the commands `IF`, `ELSE`, `FOR`, `DO`, `WHILE` and `UNTIL`.

    The jump stack is decreased at each `RETURN` and internally by the same commands as above.

5. 256 max labels declared.

6. 256 max procedures declared.

7. 256 max number or string variables.

8. 256 max number or string array variables.

9. 4 max dimensions per array.

10. 32768 max items per array in all dimensions.
