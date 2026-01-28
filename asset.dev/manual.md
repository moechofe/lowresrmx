TODO: layer registers?

# Retro Game Creator - Documentation

> This is a fork of the excellent fantasy console Lowres NX by Timo "Inutilis" Kloss.<br>
> This is a heavily modified version and way more complicated to use, I recommend you to stay on the original app:<br>
> 🌐[LowRes NX Coder on App Store](https://apps.apple.com/app/lowres-nx-coder/id1318884577).

## What is Retro Game Creator?

Create games and play them on the go using an iPhone or iPad. Retro Game Creator is a virtual game console and game development environment that includes all the tools needed to start and finish a project.

It includes a project manager, a code editor, and various tools to edit game assets: sprites, palettes, backgrounds, instruments, and music.

Retro Game Creator simulates a virtual retro game console directly inside your device, allowing users to create, play, and share their games.

It emulates old-school 8-bit/16-bit console hardware by replicating their limitations: a low number of colors, limited memory, and slow processing power.

## Difference

The goal of the app is to provide a development environment that I can use during my commutes on the subway, and later, to be able to publish a real app on the stores.

The original LowRes NX, despite being an excellent development environment, lacks some capabilities needed for use on a handheld device.

> If you already know how to use Lowres NX, you can read the list of differences below.<br>
> For new users, I recommend jumping to the [How does it work](#how-does-it-work) section.

**Background:**

- [4 background layers](#backgrounds) instead of 2, with 64x64 cells instead of 32x32.

- Support for large 16x16 pixel cells has been removed.

    Removed syntaxes: <del>`CELL SIZE`</del>.

- The [`FLIP` command now takes optional arguments](#-flip-horizontal-vertical-).

- New settings with the [`SYSTEM` command to render a layer at double size](#-system-setting-value-).

- Internal scroll offset is no longer capped at 512, but at 0xFFFF.

**Screen:**

- A new [fantasy screen](#screen) with a maximum resolution of 216x384 pixels.

- New functions to [retrieve the visible fantasy display size](#-width-shown-w-br-height-shown-h-).

- New functions to [query the safe area offset](#-left-safe-l-br-top-safe-t-br-right-safe-r-br-bottom-safe-b-).

- Removed command: <del>`DISPLAY`</del>.

**Colors:**

- The 64 available colors [do not follow the original EGA style](#64-colors) of LowRes NX.

    > This makes using the color chooser from the original "Gfx Designer" in LowRes NX a bit more challenging.

- New settings with the [`SYSTEM` command to make color 0 for a layer opaque](#-system-setting-value-) instead of transparent.

**Sprites:**

- 170 sprites instead of 64. Sprite numbers range from 0 to 169.

- Positions support sub-pixels with 1/16 pixel precision.

        SPRITE 123,SPRITE.X(123)+0.25,

**Input/Output:**

- [`=TOUCH.X/Y` coordinates return floating-point](#-x-touch-x-br-y-touch-y-) values instead of integers.

- The virtual controller and the ability to use a Bluetooth controller have been removed.

    Removed gamepad related functions and commands: <del>`=BUTTON`</del>, <del>`=UP`</del>, <del>`=DOWN`</del>, <del>`=LEFT`</del>, <del>`=RIGHT`</del>, <del>`GAMEPAD n`</del>.

    Removed useless commands: <del>`TOUCHSCREEN`</del>, <del>`KEYBOARD OPTIONAL`</del>

- When the device's virtual keyboard is hidden by the user, the corresponding I/O [Registers](#registers) flag is updated (see [Other I/O status bits](#other-i-o-status-bits)).

- New syntax to [`WAIT` until device screen is tapped](#-wait-tap-).

- New function that returns [the height taken by the device virtual `=KEYBOARD`](#-height-keyboard-) in pixels when it's visible.

- New [`HAPTIC` command to trigger haptic feedback](#-haptic-pattern-) on the device.

**Control flow:**

- New program control flow: [`ON GOTO`](#-on-goto-br-on-gosub-), [`ON GOSUB`](#-on-goto-br-on-gosub-) and [`ON RESTORE`](#-on-restore-).

**Data:**

- New keyword to [`SKIP` read data](#-skip-number-).

- It's now possible to [access data directly using its label](#labels-jumps-and-embedded-data) without the need to [`READ`](#-read-variable-variable-) them.

**Math:**

- Trigonometric functions [`=COS`, `=SIN`](#-cosine-cos-number-br-sine-sin-number-) and [`ATAN`](#-arc-atan-x-y-) internally replace π with 0.5.

    > Pico-8 uses the same approach.

    - An angle of 0.0 or 1.0 represents the right direction.
    - An angle of 0.5 represents the left direction.
    - An angle of 0.25 represents the top direction.
    - An angle of -0.25 represents the bottom direction.

- Removed trigonometric functions: <del>`=ACOS`</del>, <del>`=ASIN`</del>, <del>`=HCOS`</del>, <del>`=HSIN`</del>, <del>`=HTAN`</del>.

- New function [`=CEIL` to return the ceiling value](#-ceil-ceil-number-) of a numerical value.

- New function [`=FLOOR` to return the flooring value](#-floor-floor-number-br-floo-int-number-) of a numerical value.

- New function to [`=CLAMP` a numerical value](#-clamped-clamp-value-min-max-).

- New syntax to compute [vector length](#-length-len-x-y-).

- New function [`EASE` to compute easing function](#-interpolation-ease-function-mode-value-).

- New random generator using 🌐[PCG](https://www.pcg-random.org/). It produces better randomness.

- New syntax for [`=RND` that save the state](#-random-rnd-) of random generator.

**Text:**

- New command to expose the existing [overlay `MESSAGE` API](#-message-text-).

- New behavior for the [`PRINT` command to word wrap before breaking words](#-print-expression-).

    If there is not enough space to print a word inside the window, a new line is inserted to avoid breaking the word in two.

**Other:**

- New [`COMPAT` command to support programs from the original LowRes NX](#-compat-).

- Reuse the [`PAUSE` command to access the internal debugger](#-pause-
).

- Removed commands and functions and commands: <del>`PAUSE ON/OFF`</del>, <del>`PAUSE=`</del>.

- Removed reserved keywords: <del>`ANIM`</del>, <del>`CLOSE`</del>, <del>`DECLARE`</del>, <del>`DEF`</del>, <del>`FLASH`</del>, <del>`FN`</del>, <del>`FUNCTION`</del>, <del>`LBOUND`</del>, <del>`OPEN`</del>, <del>`OUTPUT`</del>, <del>`SHARED`</del>, <del>`STATIC`</del>, <del>`TEMPO`</del>, <del>`VOICE`</del>, <del>`WRITE`</del>.

**Memory:**

- An almost compatible memory mapping.

    The total addressable memory has increased from 64 Kibit to 128 Kibit.

    Lowres NX:

| address | size      | purpose          |
| -------:| --------- | ---------------- |
|   $0000 | 32 Kibit  | Cartridge ROM    |
|   $8000 | 4 Kibit   | Character Data   |
|   $9000 | 2 Kibit   | BG0 Data         |
|   $9800 | 2 Kibit   | BG1 Data         |
|   $A000 | 16 Kibit  | Working RAM      |
|   $E000 | 4 Kibit   | Persistent RAM   |
|   $FE00 | 256 Bytes | Sprite Registers |
|   $FF00 | 32 Bytes  | Color Registers  |
|   $FF20 |           | Video Registers  |
|   $FF40 |           | Audio Registers  |
|   $FF70 |           | I/O Registers    |

Retro Game Creator:

| address | size       | purpose          |
| -------:| ---------- | ---------------- |
|   $0000 | 8 Kibit    | BG0 data         |
|   $2000 | 8 Kibit    | BG1 data         |
|   $4000 | 8 Kibit    | BG2 data         |
|   $6000 | 8 Kibit    | BG3 data         |
|   $8000 | 4 Kibit    | Character Data   |
|   $9000 | 20 Kibit   | Working RAM      |
|   $E000 | 6 Kibit    | Persistent RAM   |
|   $FB00 | 1020 Bytes | Sprite registers |
|   $FF00 | 32 Bytes   | Color registers  |
|   $FF20 | 10 Bytes   | Video registers  |
|   $FF40 | 48 Bytes   | Audio registers  |
|   $FF70 | 40 Bytes   | I/O registers    |
|   $FFA0 | 6 Bytes    | DMA registers    |
|  $10000 | 64 Kibit   | Cartridge ROM    |

- New registers and [`DMA COPY` command for fast memory copying](#-dma-copy-rom-):

**The BASIC language:**

- A colon symbol (:) can be used to [separate multiple instructions](#instructions-separator) on the same line.

- Program code is not forced to be uppercase anymore.

**The iOS app:**

- The size of the program thumbnails are 128x128 pixels.

**The fantasy hardware:**

- The number of tokens, symbols a program can have has been increased.

- The number of CPU-cycles a program can execute per frame, per vbl and per raster has been increased to accommodate with the bigger screen size.

**Particles:**

> **Work in progress:** The particle system is under development and everything is subject to change.

The particles library reuses sprites to make them appear, disappear, change their character data, and move them across the screen.

---

`PARTICLE first, count AT address`

Declare the use of `count` sprites starting from `first` and store the internal data at `address`.

Each sprite will consume 6 bytes of memory. For each sprite:

| address | size    | purpose         |
| -------:| ------- | --------------- |
|      +0 | 2 Bytes | Speed on x axis |
|      +2 | 2 Bytes | Speed on y axis |
|      +4 | 1 Byte  | Appearence      |
|      +5 | 1 Byte  | Current frame   |
|      +4 | 2 Bytes | Lifetime        |

	PARTICLE 1,20 AT $9000
	'Will use sprite 1 to 20 included.
	'Store internal data from $9000 to $9078 excluded.

---

`PARTICLE appearance DATA label`

Declare a `label` that contains data for the particle `appearance`.

`appearance` is a number between 0 and 23.

The associated data MUST contain a list of character numbers that will be used to draw the particle. Using a negative number will loop back to the previous data value.

A common example is to use zero (0) and minus one (-1) for the last two data values, which allows the particle to disappear until the sprite is reused.

	my_appearance:
	DATA 1,2,3,4,0,-1
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

	SUB mypart(spr,addr)
		sy=PEEKW(addr+2)
		POKEW addr+2,sy-5
		'change the speed in y axis
	END SUB

The number of available cycles will be specified in a future version.

---

`EMITTER count AT address`

Declare the use of `count` emitters and store the internal data at `address`.

`count` is a number between 0 and 15.

Each emitter will consume 6 bytes of memory. For each emitter:

| address | size    | purpose                 |
| -------:| ------- | ----------------------- |
|      +0 | 2 Bytes | Position on x axis      |
|      +2 | 2 Bytes | Position on y axis      |
|      +4 | 1 Byte  | Delay before emit       |
|      +5 | 1 Byte  | Number of times to emit |

---

`EMITTER emitter DATA label`

Declare a `label` that contains DATA for the `emitter`.

The associated data MUST follow this list:
1. `appearance` index of `appearance`, see: `PARTICLE appearance DATA label`.
2. `shape` (documentation pending),
3. `outer` ring radius,
4. `inner` ring radius,
5. `arc` (documentation pending),
6. `rotation` (documentation pending),
7. initial particle `speed` on x axis,
8. initial particle `speed` on y axis,
9. `explosivity` (documentation pending),
10. `count` of the number of particles to spawn at this frame,
11. number of frames to `delay` before re-emission,
12. how many times to `repeat` the emission.

When a particle is spawned, a random position is computed between the `outer` and `inner` ring.

---

`EMITTER emitter ON x, y`

Tell the `emitter` to emit particles starting at position `x` and `y`.

It will automatically end when `repeat` reaches 0.

---

`EMITTER emitter OFF`

Manually stop the `emitter`.

## How does it work

Retro Game Creator does not have a framebuffer where users paint pixels like in Pico-8. Instead, it simulates a 🌐[NES/Gameboy PPU chip](https://www.nesdev.org/wiki/PPU), which traverses its memory to generate output pixels based on characters, backgrounds, sprites, palettes and register information.

### Screen

The fantasy screen has a maximum resolution of 216x384 pixels.

It features perfectly square pixels, a fixed portrait orientation, and always fills the entire device screen.

Depending on the device's screen ratio, the number of visible virtual pixels may vary from 216x288 on 4:3 devices to 177x384 on 19.5:9 devices.

That's why Retro Game Creator provides an API to request the [actual visible dimension of the screen using the `=SHOWN.W` and `=SHOWN.H` functions](#-width-shown-w-br-height-shown-h-).

Additionally, the device can have a notch and bottom inset full-width buttons. Retro Game Creator provides an API to request the offset from the visible dimension to the [safe area using `=SAFE.L`, `=SAFE.R`, `=SAFE.T` and `=SAFE.B` functions](#-left-safe-l-br-top-safe-t-br-right-safe-r-br-bottom-safe-b-).

### Colors

Retro Game Creator provides the user with a fixed set of [64 colors](#64-colors).

The colors are grouped into sets of 4 called palettes, with 8 palettes available for users to colorize their creations.

The 1st color of the 1st palette is used as the background color for the whole program. For the following palettes (2nd to 8th), the 1st color is transparent.

This can be altered by the [`SYSTEM` command](#-system-setting-value-).

From now on, the terms:
- **a palette** apply to one of the 8 palettes with 4 colors inside,
- **a color** means one of the 4 colors within a palette or one of the 64 available colors.

Sauce: 🌐[FAMICUBE palette created by Arne](https://lospec.com/palette-list/famicube).

### Characters

The 256 characters are [blocks of 8x8 pixels](#character-data) assigned to numbers from 0 to 255.
When a number is used to draw in a background layer or sprite, the rendering process draws the corresponding 8x8 pixel block.

From now on, the term:
- **a character** is a block of 8x8 pixels and takes 16 bytes.

### Backgrounds

Retro Game Creator has 4 [background layers](#background-data), each with an independent:
- X and Y position,
- and 64x64 character cells.

The 4 layers are drawn in order from 4th to 1st. Cells of the 1st background will be visible above the others unless the priority flag is used.

Each cell has its own:
- character number,
- palette,
- horizontal and vertical flip,
- priority to alter which layer's cell is drawn on top of another and on top of sprites.

Layers are not framebuffers; you cannot easily paint pixels directly. Instead, every cell references a character number, and those hold the pixel color information.

Each layer can be shifted using the position on the X and Y axes. When reaching the edges, the cells will wrap around and remain visible.
Put another way, when the rendering process tries to draw the 65th cell of a row, because it does not exist, it will draw the 1st cell instead. This can be used to achieve infinite scrolling.

As mentioned before, the 1st color of each palette is transparent except for the 1st color of the 1st palette, which determines the background color of the whole fantasy screen.

This can be altered by the `SYSTEM` command.

From now on, the terms:
- **a layer** is a background layer of 64x64 cells,
- **a cell** is a square of a layer that shows one character, so 8x8 pixel.

Check [background data](#background-data) and the [background API](#background-api) for more information.

### Sprites

Retro Game Creator offers 170 sprites, each with:
- X and Y position,
- a dimension of 8x8, 16x16, 24x24 or 32x32 pixels,
- a character number for the top left one,
- a palette,
- horizontal and vertical flip,
- and priority to show the sprite on top of bg cells with priorities.

The 170 sprites are drawn in order from 170th (bottom) to 1st (top).
The sprite numbers range starts at 0, so the last one is 169.

Sprites are groups of characters ranging from 1x1 to 4x4. The difference with backgrounds is that you only choose the top-left character. The other character numbers are dependent on the top-left one. To compute them, add 1 from the first one to reach the character to the right, and add 16 from the first one to get the one below.

Example with a 2x2 sprite with the first character to be 1:

|    |    |
| -- | -- |
|  1 |  2 |
| 16 | 17 |

A sprite can be freely placed anywhere on the screen with 1/16 sub-pixel precision. This means you can add 0.5 to its position even if there will be no visual change.

Check [sprites registers](#sprite-registers) and the [sprite API](#sprite-api) for more information.

### Rendering

The fantasy screen is rendered line by line; this is called a raster.

During raster, the pixels from background layers and sprites are drawn according to priority flags and the palette is applied. All impacted parameters are not fixed, and a sub-routine can be used between each line to alter some parameters and create interesting visual effects, see `ON RASTER CALL` and `=RASTER`.

Check the [sprite API](#sprite-api) and the [background API](#background-api).

### CPU & cycles

Retro Game Creator simulates a fantasy console with limitations, one of them being the number of instructions the CPU can execute before the next frame MUST be drawn and presented to the player.

This is achieved by a set of different rules applied to an internal CPU cycle counter. When the maximum is reached, the next frame is drawn no matter what. This can have the following effects:

1. The execution of the main program is halted and will continue after the frame is drawn. The program will run slower.

2. If the maximum cycles are reached during a raster interrupt, the next line is not drawn.

Check the [cycles reference](#cycles) for more information.

### Cartridge

A Retro Game Creator cartridge is a text file containing the program code and 16 virtual files after the text.

The virtual files data are encoded in hexadecimal and SHOULD respect a specific format.

> Manually editing the virtual files data is NOT RECOMMENDED as it can lead to unexpected behavior. Instead, use the provided tools and APIs to manipulate the virtual files.

### Virtual file

There are 16 virtual files identified by a number `id` and can contain a `comment$`.

    #id:comment$
    data

- `id` is a number between 0 and 15.
- `comment$` is a string **without double quotes** of 32 characters max.
- `data` is a list of hexadecimal literals.

When the program is started, the content of the files is loaded and [mapped to the memory](#memory-mapping).

The memory address for the files starts at $10000, and the best way to know this address is to use the `=ROM` function.

Check the [file API](#file-api).

### Memory

The fantasy hardware simulates a [memory mapping](#memory-mapping), making access to the cartridge ROM and registers accessible using an address ($0000..$1FFFF).

Each address contains a value of one byte (0..255). Some of those addresses are readable, writable, both or none.

Check the [memory API](#memory-api) for more information.

### Persistent Memory

The fantasy hardware simulates persistent memory allowing users to store some bytes on the device memory, making them persistent even after stopping the program or quitting the app.

Check the [memory API](#memory-api) and [memory mapping](#memory-mapping) for more information.

### Sound

The fantasy hardware simulates 4 voices to play sound effects and music using an internal tracks player.

Check the [sound API](#sound-api) for more information.

### Overlay

When the program is running, there is an option in the device application that enables showing the screen overlay.

This screen shows the simulated CPU usage by counting how many [fantasy CPU cycles](#cycles) have been consumed already.

It also shows any message that has been printed on the overlay using the `TRACE`.

### Keyboard

The fantasy hardware has access to the device virtual keyboard. It allows users to type while running a program.

Check the [input API](#input-api) for more information.

## Program Language

The programming language follows the path of the original Lowres NX. It's a 🌐[BASIC-type language](https://en.wikipedia.org/wiki/BASIC) and here's how to use it.

### Program

A program is a list of so-called instructions executed by the fantasy hardware one by one from top to bottom. The simplest and most iconic program could be:

    PRINT "hello world!"

Users are invited to create a new program, type the instruction above and run it.

Keywords and values are separated by spaces and instructions by new lines.

    PRINT "hello..."
    PRINT "second instruction"

> The program does not care about upper or lower case, use the one you prefer.

### Instructions

Instructions tell the fantasy hardware to do something.

- Assigning a [literal](#literals) or an [expression](#expressions) to a [variable](#variables-and-assignation).
- Executing a [command](#commands) or a [function](#functions).
- Calling a [procedure](#procedures) with or without [arguments](#parameters-arguments-and-scopes).
- Jumping to a [label](#labels-jumps-and-embedded-data) or returning from the [stack](#stack-sub-routine-and-return).

### Identifiers

1. An identifier is a word defined by the user and is used to declare variables, labels or procedures (more on those terms later), e.g.: hero, enemies, HP, score, spawn_monsters, updateScreen, CollectCoins, level12…

2. An identifier can also be a built-in function name provided by Retro Game Creator as part of the [API](#api-instructions).

Valid characters for identifiers are: ASCII letters, digits and the underscore (_). However, they cannot start with a digit and they cannot have more than 21 characters.

**Important:** Not all identifiers are valid because some of them are reserved by the language itself. Check the [list of reserved keywords](#reserved-keywords) and learn them to avoid common mistakes.

### Variables and assignation

A variable is a value that is retained in the fantasy hardware memory as long as the program is running. Variables can be used to store positions, scores, HP and all sorts of information. The purpose of a variable is to store a value to retrieve it later.

Example: a score counter that starts at zero and increments every time a plumber hero jumps on enemies' heads. An appropriate name for this value is score and it can be declared and assigned like this:

    score=0

And to increment it:

    score=score+1

As you can see, variables are declared using an identifier. That's why users SHOULD choose descriptive names that help them remember what's inside.

As a real example, this is a program that increments and prints a score every time the player taps the device screen.

    score=0
    DO
        PRINT "score",score
        WAIT TAP
        score=score+1
    LOOP

Variable can also store strings:

    name$="Untel"
    print name$

Variable syntax:

    identifier[$][(expression)]

This allows variables to have 4 different forms:

- my_number
- my_string$
- my_number_array()
- my_string_array$()

Assignation syntax:

    variable=expression

Read the chapter about [expression](#expressions) to learn about the dedicate syntaxe.

### Literals

Identifiers can store two types of values: numbers and strings.

Numbers can be integers or decimals, e.g.: 123, -45, 0.01, 12345.6789

Strings can only contain ASCII7 characters: letters, digits, some punctuation and some control characters: "hello WORLD! 123"

Learn more about the technical information about [number type](#number-limits) and [ASCII7 characters](#ascii-table).

### Arrays

Arrays are lists of values grouped into one variable:

    DIM player_score(1)
    player_score(0)=123
    player_score(1)=456

Arrays are zero-based indexed.

Arrays can also store strings:

    DIM player_name$(1)
    player_name$(0)="Untel"
    player_name$(1)="John Doe"

See `DIM`, `DIM GLOBAL` and `=UBOUND` for more information.

### Labels, jumps and embedded data

Another feature that uses identifiers are labels. They are used to mark a position in the program to go back to it later. Users need to understand that programs are executed line by line from top to bottom and labels are one solution to go back to the top of the program or anywhere else (almost).

Here is an example that demonstrates how to recreate the program above using labels:

    score=0
    start:
        PRINT "score",score
        WAIT TAP
        score=score+1
    GOTO start

A label is declared by an identifier followed by a colon : (here "start"), and the `GOTO` command will make the program execution "jump" to the previous label "start", creating an infinite loop.

---

About using label identifiers as [rvalue](#lvalue-and-rvalue): This feature allows using label names that lead to [constant string values](#-data-constant-constant-) directly instead of having to store the value in a variable.

Example, instead of writing:

    DATA "chocolate"
    READ like$
    PRINT "I like",like$

You can write:

    like: DATA "chocolate"
    PRINT "I like",like

And it works in all [expressions](#expressions) that involve [literal strings](#literals) or [string variables](#variables-and-assignation).

    test1: DATA "entanglem"
    PRINT test1+LEFT$(test1,3)
    test2: DATA 1234.5
    PRINT test2*2

### Stack, sub-routine and return

Using the `GOTO` command to jump to another part of the program is very useful to organize the code and create loops. Sometimes this is not enough, introducing the `GOSUB` command.

The stack is a place inside the fantasy hardware that tracks where a program was before jumping to a label, allowing you to `RETURN` to the previous location.

This allows reaching the same label from different parts of the code, while being able to return to it later.

Example of a program that reaches the same label from two places.

    one:
        PRINT "one"
        GOSUB common
    two:
        PRINT "two"
        GOSUB common
        GOTO one

    common:
        WAIT TAP
        RETURN

### Procedures

The last usage of identifiers are procedures. They are meant to be reusable sub-parts of a program.

Here's a quick example:

    SUB addition(a,b,c)
        c=a+b
    END SUB

By running this program, nothing will happen because procedures do not get executed automatically. Instead the user MUST use a dedicated command for it.

    result=0
    CALL addition(120,3,result)
    PRINT "result",result

By adding this piece of code after the previous one and running the program, the solution of the operation 120+3 will be printed on the screen: "result 123".

The advantage of calling procedures is being able to do it again and again.

    CALL addition(45,-78,result)
    PRINT "result",result

This time, it will print "result -33".

The difference with [sub-routines](#stack-sub-routine-and-return) is the [scope](#parameters-arguments-and-scopes).

### Parameters, arguments and scopes

Another concept that comes with procedures are parameters and arguments, but first, users need to learn a thing about variable scope.

By default, when declaring a variable in a program, its scope is limited and the variable is unknown inside a procedure.

    score=0
    SUB print_score
        🐞print "score",score
    END SUB
    CALL print_score

This program will produce the error: "variable not initialized". The program can't access the variable score inside the print_score procedure.

One option is to pass the variable to the procedure. Two changes MUST be made for that.

1. Users need to add a parameter to the procedure declaration:

        SUB print_score(s)
            print "score",s
        END SUB

2. Users also need to add an argument to the procedure call:

        CALL print_score(score)

When the procedure print_score is called, the argument score is passed to the procedure inside the s parameter.

Corrected program:

        score=0
        SUB print_score(s)
            PRINT "score",s
        END SUB
        CALL print_score(score)

Another option is to rely on [global scope](#local-and-global-scope).

### Passed by reference

Arguments are passed by reference. This means that a variable passed to a procedure and being modified by this procedure will stay modified when exiting the procedure.

    SUB modify_it(a)
        a=123
    END SUB
    my_value=0
    CALL modify_it(my_value)
    PRINT my_value

It will print 123.

To avoid this behavior, encapsulate the argument in round brackets (identifier).

    SUB modify_it(a)
        a=123
    END SUB
    my_value=0
    CALL modify_it((my_value))
    PRINT my_value

It will print 0.

### Local and global scope

When variables are initialized or declared in the main program body, they are local to the program body.

When variables are initialized or declared inside a procedure body, they are local to this procedure body.

It is possible to declare a variable global so it can be accessed in both scopes.

    GLOBAL score
    SUB frag
        score=score+1
    END SUB
    DO
        CLS
        PRINT "score:",score
        WAIT TAP
        CALL frag
    LOOP

Global variables MUST be declared before trying to access them.

The syntax to declare a global array is a bit different:

    DIM GLOBAL enemies

See `GLOBAL` and `DIM GLOBAL` for more information.

### Comments

Comments are pieces of text that are not executed. They are useful for taking notes of what a particular piece of code is doing.

    'get touch position in cells coordinates
    tx=TOUCH.X\8

> Avoid 🌐[Captain Obvious](https://en.wikipedia.org/wiki/Captain_Obvious) comments!

### Grammar

As said earlier, a program is a list of instructions read and executed one by one. To make it easier to read, developers use a set of spaces, indentations, new lines, comments, procedures…

Here is a set of rules on how it works:

- An instruction cannot be split on two lines.

    This will not work:

        🐞a=
        123

- Labels are not instructions; users can place instructions on the same line.

    ✅ This will work:

        test: PRINT 123
        WAIT TAP
        GOTO test

- The only way to squeeze two instructions on one line is to use the : colon separator.

    ✅ This will work:

        x=TOUCH.X : y=TOUCH.Y

    > I recommend placing spaces before and after the `:` colon until I fix the parsing issue.

- `IF...THEN` uses a different syntax for one-line instructions, and : colon as instruction separator will not work correctly.

    This will not work as intended:

        DO
            x=0 : y=0
            IF TAP THEN x=TOUCH.X : y=TOUCH.Y
            PRINT x,y
            WAIT TAP
        LOOP

    `y=TOUCH.Y` will always be executed.

- Comments can be placed after the : colon instruction separator.

    ✅ This will work:

        a=123 :'Default value


- Indentation has no meaning; users can place it however they want.

    ✅ This will work:

        test:
                PRINT       "far"
        PRINT "near"


- Check the [list of reserved keywords](#reserved-keywords), as users cannot use them for identifiers.

### Expressions

Expressions are used to compute values, be assigned to identifiers or passed to functions as arguments. An expression can be one of:

- A [literal value](#literals):

        123
        "gabu"

- A [variable identifier](#variables-and-assignation):

        myVar$
        myArray(0)

- A [function call](#functions):

        cos(1.314)
        LEFT$(name$,8)

- An [unary, binary or group operator](#operators):

        42+24
        $FF00+A*2
        count>=0
        NOT dead
        (2+3)*4

- A [label identifier](#labels-jumps-and-embedded-data):

        mylabel

### Operators

Operators are used inside [expressions](#expressions) to compute or alter their values.

**Arithmetic operators:**

Operate on two numeric values to produce a new one.

| symbol | example | purpose          |
| ------:| ------- | ---------------- |
|      - | -42     | negation         |
|      ^ | x^3     | exponentiation   |
|      * | 2*y     | multiplication   |
|      / | x/2     | division         |
|      \ | x\2     | integer division |
|    mod | x mod 2 | modulo           |
|      + | c+2     | addition         |
|      - | 100-d   | subtraction      |

The priority of execution respects the mathematical rules.

**Group operator:**

Rounded parentheses () are used to counter the operator priority.

**Comparison operator:**

Used to compare two numeric or string values and produce -1 if the test succeeds or 0 otherwise. Generally used as [expressions](#expressions) in [conditional flow control](#control-flow).

| symbol | example | purpose          |
| ------:| ------- | ---------------- |
|      = | a=10    | equal            |
|     <> | a<>100  | not equal        |
|      > | b>c     | greater          |
|      < | 5<x     | less             |
|     >= | X>=20   | greater or equal |
|     <= | X<=30   | less or equal    |

**Bitwise operator:**

Used to manipulate each bit of numeric values.

| symbol | example                    | purpose                                                     |
| ------:| -------------------------- | ----------------------------------------------------------- |
|    NOT | not (x=15)<br>not 0        | Bits that are 0 become 1,<br>and those that are 1 become 0. |
|    AND | a=1 and b=12<br>170 and 15 | If both bits are 1, the<br>result is 1, 0 otherwise.        |
|     OR | x=10 or y=0<br>128 or 2    | The result is 0 if both<br>bits are 0, 1 otherwise.         |
|    XOR | a xor b                    | The result is 1 if only<br>one bit is 1, 0 otherwise.       |

**Concatenation operator:**

Copy a string at the end of another string.

| symbol | example  | purpose                           |
| ------:| -------- | --------------------------------- |
|      + | "ab"+"c" | Concatenate two strings into one. |

### Commands

Retro Game Creator provide a bunch of built-in commands to manipulate the fantasy device hardware such as the graphic or sound. You'll find them by consulting the [list of API instructions](#api-instructions).

To execute a command, use it's identifier followed by a list of coma-separated arguments or other reserved keywords. The exact syntax depends on the command.

    SPRITE OFF 0 to 169
    SPRITE 0,40,60,1

### Functions

Retro Game Creator provide a bunch of built-in function to manipulate the fantasy device hardware such as the graphic or sound. You'll find them by consulting the [list of API instructions](#api-instructions).

To execute a function, use it's identifier followed by a list of coma-separated arguments surrounded by rounded parenthesis.

    PRINT MID$("test",3,1) :'print s

A function will always return something and MUST be used as [rvalue](#lvalue-and-rvalue) inside an [expressions](#expressions).

### Lvalue and Rvalue

The equal = sign is used both as variable assignement and comparison operator, and It can lead to some confusion. Here is a tip to help you:

- If the line start by the variable name, it's [a variable assignation](#variables-and-assignation).

- In all other cases, the variable is used as [a part of an expression](#expressions).

An lvalue appears on the left side of an assignment, and an rvalue appears on the right side of an assignment. Also, rvalue is used as an argument of [functions](#functions) or [procedures](#procedures).

|                               expression | type of value    |
| ----------------------------------------:| ---------------- |
|   [variable](#variables-and-assignation) | lvalue or rvalue |
|                     [literal](#literals) | rvalue           |
|              [function call](#functions) | rvalue           |
|               [any operator](#operators) | rvalue           |
| [label](#labels-jumps-and-embedded-data) | rvalue           |

### Instructions separator

An instruction generally end by a new line \n.

It's possible to put multiple instructions using a colon : on one line but with limitation:

1. Can be miss-interpreted as a label declaration. So the best way is to add a space between the idenfier and the colon.

    This will not work:

        test: DATA 123
        🐞RESTORE test: WAIT VBL

    ✅ This will work:

        test: DATA 123
        RESTORE test : WAIT VBL

2. Does not work with the one line syntaxe of [the `IF/THEN` command](#-if-then-else-if-else-end-if-br-if-then-).

        IF expression THEN instruction1 : instruction2

    `instruction2` will always be executed.

### Reserved keywords

Some identifier cannot be used by the user for variables, proceduce or label name because they are keywords reserved by the language and it's API:

`ABS`, `ADD`, `AND`, `ASC`, `ATAN`, `ATTR`, `AT`, `BG`, `=BIN$`, `CALL`, `=CEIL`, `=CELL.A`, `=CELL.C`, `CELL`, `CHAR`, `=CHR$`, `=CLAMP`, `CLS`, `CLW`, `=COLOR`, `COMPAT`, `COPY`, `=COS`, `CURSOR.X`, `CURSOR.Y`, `DATA`, `DEC`, `DIM`, `DMA`, `DO`, `EASE`, `ELSE`, `EMITTER`, `END`, `ENVELOPE`, `EXIT`, `EXP`, `=FILE$`, `FILES`, `FILL`, `FLIP`, `FLOOR`, `FONT`, `FOR`, `FSIZE`, `GLOBAL`, `GOSUB`, `GOTO`, `HAPTIC`, `=HEX$`, `HIT`, `IF`, `INC`, `=INKEY$`, `INPUT`, `=INSTR`, `INT`, `KEYBOARD`, `LEFT$`, `LEN`, `LET`, `LFO.A`, `LFO`, `LOAD`, `LOCATE`, `LOG`, `LOOP`, `MAX`, `MCELL.A`, `MCELL.C`, `MCELL`, `MESSAGE`, `MID$`, `MIN`, `MOD`, `MUSIC`, `NEXT`, `NOT`, `NUMBER`, `OFF`, `ON`, `OR`, `PALETTE`, `PAL`, `PARTICLE`, `PAUSE`, `PEEKL`, `PEEKW`, `PEEK`, `PI`, `PLAY`, `POKEL`, `POKEW`, `POKE`, `PRINT`, `PRIO`, `RANDOMIZE`, `RASTER`, `READ`, `REPEAT`, `RESTORE`, `RETURN`, `RIGHT$`, `RND`, `ROL`, `ROM`, `ROR`, `SAFE.B`, `SAFE.L`, `SAFE.R`, `SAFE.T`, `SAVE`, `SCROLL.X`, `SCROLL.Y`, `SCROLL`, `SGN`, `SHOWN.H`, `SHOWN.W`, `SIN`, `SIZE`, `SKIP`, `SOUND`, `SOURCE`, `SPRITE.A`, `SPRITE.C`, `SPRITE.X`, `SPRITE.Y`, `SPRITE`, `SQR`, `STEP`, `STOP`, `STR$`, `SUB`, `SWAP`, `SYSTEM`, `TAN`, `TAP`, `TEXT`, `THEN`, `TIMER`, `TINT`, `TOUCH.X`, `TOUCH.Y`, `TOUCH`, `TO`, `TRACE`, `TRACK`, `UBOUND`, `UNTIL`, `VAL`, `VBL`, `VIEW`, `VOLUME`, `WAIT`, `WAVE`, `WEND`, `WHILE`, `WINDOW`, `XOR`.

## BASIC instructions

To be able to use the language as a tool to make games, Retro Game Creator provides instructions to:

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

    IF expression THEN
        instruction...
    [ELSE IF expression THEN
        instruction...]
    [ELSE IF...]
    [ELSE
        instruction]
    END IF

Will execute the list of instruction... if the above `expression` is true.
An [expression](#expressions) is evaluated as `true` if the result of the expression is different from 0.

It's possible to have multiple `ELSE IF` blocks but a maximum of one `ELSE` block is authorized.

Real example of a game that asks the player to guess a number:

    again: PRINT "guess the number"
    answer=RND(100)
    retry: INPUT "1-100:";guess
    IF guess<1 OR guess>100 THEN
        PRINT "a number between 1 and 100"
        GOTO retry
    ELSE IF guess=answer then
        PRINT "you got it!"
        PRINT
        GOTO again
    ELSE IF guess<answer THEN
        PRINT "too low"
        GOTO retry
    ELSE
        PRINT "too high"
        GOTO retry
    END IF

---

    IF condition THEN instruction

This form only allow one `instruction`, but it's also shorter.

Real example of the same game with less lines:

    again: PRINT "guess the number"
    answer=RND(100)
    retry: INPUT "1-100:";guess
    IF guess=answer THEN
        PRINT "you got it!"
        PRINT
        GOTO again
    END IF
    IF guess<1 OR guess>100 THEN PRINT "a number between 1 and 100"
    IF guess<answer THEN PRINT "too low"
    IF guess>answer THEN PRINT "too high"
    GOTO retry

#### `DO/LOOP/EXIT`

    DO
        instruction...
        [EXIT/GOTO]
    LOOP

Continuously execute the list of instruction....

Possible way to get out of the loop is using `EXIT` or `GOTO`.

Real example of non stop moving square that bounce on the device screen:

    x=SHOWN.W\2
    y=SHOWN.H\2
    sx=1
    sy=1
    DO
        IF x=0 OR x=shown.w-8 THEN sx=-sx
        IF y=0 OR y=shown.h-8 THEN sy=-sy
        ADD x,sx
        ADD y,sy
        SPRITE 0,x,y,1
        WAIT VBL
        IF TAP THEN EXIT
    LOOP
    PRINT "done"

    #2:main characters
    00000000000000000000000000000000
    0000000000000000FFFFFFFFFFFFFFFF

`EXIT` command will exit one level of DO/LOOP.

#### `REPEAT/UNTIL/EXIT`

    REPEAT
        instruction...
        [EXIT/GOTO]
    UNTIL expression

Repeat the list of `instruction...` until the `expression` ahead became true.
An [expression](#expressions) is evaluated as `true` if the result of the expression is different from 0.

Possible way to get out of the loop is using `EXIT` or `GOTO`.

It different from WHILE/WEND because `expression` is evaluated after the list of instruction... is executed.

    stop_now=-1
    REPEAT
        PRINT "do it anyway"
    UNTIL stop_now

`EXIT` command will exit one level of REPEAT/UNTIL.

#### `WHILE/WEND/EXIT`

    WHILE expression
        instruction...
        [EXIT/GOTO]
    WEND

Repeat the list of instruction... while the `expression` above is true.
An [expression](#expressions) is evaluated as `true` if the result of the expression is different from 0.

Possible way to get out of the loop is using `EXIT` or `GOTO`.

It different from REPEAT/UNTIL because `expression` is evaluated before the instruction... list is executed.

    count=0
    WHILE count
        PRINT "not executed"
    WEND

`EXIT` instruction will exit one level of WHILE/WEND.

#### `FOR/TO/STEP/NEXT/EXIT`

    FOR variable=begin TO ended [STEP incr]
        instruction...
        [EXIT/GOTO]
    NEXT identifier

Repeat the instruction... list while varying the value of the `variable` starting at `begin` [expression](#expressions) until it reach `ended` expression included.
The increment can be changed using `incr`, allowing to iterates in reverse.

Possible way to get out of the loop is using `EXIT` or `GOTO`.

Real example that print numbers from 1 to 9 in ascending and descending order.

    FOR i=1 TO 9
        PRINT i;
    NEXT i
    PRINT
    FOR i=9 TO 1 STEP -1
        PRINT i;
    NEXT i

`EXIT` command will exit one level of FOR/TO/NEXT.

#### `GOTO`

    GOTO label

Make the execution of the program to jump at the specific `label`.

With `label` being declared somewhere in the program.

Real example of a tool that flip a coin:

    again:
        PRINT
        PRINT "tap to flip a coin"
        WAIT TAP
        coin=RND(2)
        IF coin=1 THEN GOTO heads
        IF coin=2 THEN GOTO tails
        GOTO edge
    heads:
        PRINT "heads"
        GOTO again
    tails:
        PRINT "tails"
        GOTO again
    edge:
        PRINT "edge of the coin!"
        GOTO again

#### `GOSUB/RETURN`

    GOSUB label
    ...
    RETURN

Store the current location of the program execution [on top of the stack](#stack-subroutine-and-return), and jump at the specific `label`.

Expect to found a `RETURN` command to go back where it was right after the `GOSUB`.

The return location is store in a stack allow user to jump to a sub-routine and return from it.

> Best practice: always have one `RETURN` for each `GOSUB`.

A common usage of sub-routine is to reuse a piece of code multiple times instead of rewrite it again.

> Users SHOULD place all sub-routines in one place near the end of the program to ensure they are not executed unintentionally.

Real example. A score is incremented using time and tap:

    score=0
    GOSUB update
    t=timer
    DO
        IF timer-t>30 THEN GOSUB increase
        IF TAP THEN GOSUB increase
        WAIT VBL
    LOOP
    increase:
        INC score
        t=TIMER
    update:
        TEXT 2,2,"score:"+STR$(score)
        RETURN

This example has one `RETURN`, two labels and three `GOSUB`. The best practice mentioned above is not followed here, but here is the explanation.

The important thing is not the number of `RETURN` statements in the code but the number of times the command is executed. Users need to understand that one `GOSUB` execution will [increase by one the size of the stack](#stack-subroutine-and-return), and one execution of `RETURN` will reduce the same stack by exactly one. If the stack size is empty every time the `WAIT VBL` instruction is executed, it's a good sign. It means that the stack will not overflow.

Users can print the current stack using the [debugger `TRACE` command](#dbg-trace-).

#### `ON GOTO`<br>`ON GOSUB`

    ON value GOTO label0[,label1...]

Jump to one of the listed `label` according to a `value`.

`ON GOSUB` Will store the current program location on top of the stack before jumping, allowing to `RETURN` to this location later.

Will read the `value` and jump to:
- the first label if `value` equal 0,
- the second label if `value` equal 1,
- ...

Real example:

    again:
        WAIT 30
        ON RND(1) GOTO zero,one
    zero:
        PRINT "zero"
        GOTO again
    one:
        PRINT "one"
        GOTO again

#### `SUB/END SUB/EXIT SUB`

Syntax for a [procedure definition](#procedures):

    SUB procedure [(parameters...)]
        instruction...
        [EXIT SUB]
    END SUB

This will define a `procedure` and can optionnaly received a list of `parameters`.

An empty list of `parameters` is not valid. Simply remove the parentheses ().

Syntax of the `parameters` list:

    (identifier[$][()], identifier[$][()]...)

Hm, it's a mess! Let me explain this. A parameter is always an `identifier` and can be followed by the type of value it contains. The list of possibilities are:

- a number: my_num
- a string: my_str$
- an array of number: my_num_array()
- an array of string: my_str_array$()

Procedures are isolated small programs that can be executed using the `CALL` command. They are isolated because [variables declared inside are local](#local-and-global-scope).

> Body of sub-routines are only executed through the `CALL` command, so it is safe to place them at the beginning of the program.

Real example, a game where the player needs to enter a sequence of digits while the computer tries to prevent it:

    'perturb player input
    SUB perturb
        if cursor.x<10 then print str$(rnd(9));
    END SUB

    'handle player input
    SUB handle(c$)
        a=asc(c$)
        IF a=8 THEN CALL delete
        IF a<48 or a>57 THEN EXIT SUB
        IF rnd<0.3 THEN CALL perturb
        IF cursor.x<10 THEN PRINT c$;
    END SUB

    'delete last character
    SUB delete
        IF cursor.x=0 THEN EXIT SUB
        LOCATE cursor.x-1,cursor.y
        TEXT cursor.x,cursor.y," "
    END SUB

    'check if input is correct
    SUB check(r$)
        r$="       "
        TEXT 0,2,"       "
        IF cursor.x<10 THEN EXIT SUB
        ok=-1
        FOR i=0 TO 9
            IF CELL.C(i,1)-208<>i THEN ok=0
        NEXT i
        r$="failure"
        IF ok THEN r$="success"
    END SUB

    'main game
    PRINT "write number form 0 to 9"
    result$="       "
    KEYBOARD ON
    RANDOMIZE TIMER
    DO
        k$=INKEY$
        IF k$<>"" THEN CALL handle(k$)
        WAIT VBL
        CALL check(result$)
        TEXT 0,2,result$
    LOOP

`EXIT SUB` command will exit the `SUB`.

> Using `GOTO` inside a procedure is NOT RECOMMENDED, especially if the destination target is outside of the body. It will permanently increase the stack size without any possibility to reduce it.

> Using `GOSUB` inside a procedure that jumps to a label outside of the body is NOT RECOMMENDED because the scope of the procedure will be used to execute the instructions.

#### `CALL`

Syntax for a [procedure call](#procedures):

    CALL procedure [(parameters)]

This will call the `procedure` and optionnally pass a list of `parameters`.

Syntax of the `parameters` list:

    (identifier[$][([items...])], identifier[$][([items...])]...)

Hm, it's a mess! Let me explain this. A parameter is always an `identifier` and can be followed by the type of value it contains. If the value is an array, an `expression` can be added to get one `item` instead of the whole array.

Syntax of the `items` list:

    expression, expresion...

This allow to get any items in the array according to its number of dimensions.

 The list of possibilities are:

- a number: my_num
- a string: my_str$
- an array of number: my_num_array()
- an array of string: my_str_array$()
- one number from an array: my_num_array(1,2)
- one string from an array: my_str_array$(1,2)

### Embedded data

Retro Game Creator provides two ways to store data or assets inside a program.

1. Use the combination of `DATA` and `READ` to store readable number and string and access it when you need it.

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

#### `DATA constant [,constant...]`

A list of `constant` values (numbers or strings) that can be accessed using the `READ` command.

#### `READ variable [,variable...]`

Read a list of values inside `variable` that was declared using `DATA`.

#### `RESTORE [label]`

Move the read pointer to a specified `label`. The declared constant values that appear after the label will be read next. When ommited, restore at the beginning of the program.

#### `SKIP number`

Allow to skip a `number` of constant values by moving the read pointer.

    DATA "failure"
    DATA "success"
    SKIP 1
    READ word$
    PRINT word$

#### `ON RESTORE`

    ON value RESTORE label0, [label1...]

Move the read pointer to one of the listed `label` according to a `value`.

### Variables and scopes

#### `GLOBAL`

    GLOBAL identifier...

Declare a list of number or string variables to be globally [accessible in all scopes](#local-and-global-scope).

`GLOBAL` are illegal inside a subroutine body.

#### `DIM`

    DIM identifier(highest) [,identifier(highest)]...

Will declare one or more arrays with `highest`+1 number of elements.

Arrays of numbers or strings MUST be declared before reading or writing them.

    DIM scores(1),names$(1)
    FOR i=0 TO 1
        PRINT names$(i);":",scores(i)
    NEXT i

#### `DIM GLOBAL`

    DIM GLOBAL identifier(highest) [,identifier(highest)]...

Similar to `DIM` but will [declare the arrays](#arrays) [globally accessible](#local-and-global-scope).

`DIM GLOBAL` are illegal inside a SUB/END SUB subroutine body.

#### `=UBOUND`

    highest=UBOUND(identifier[,dimension])

Return the `highest` index of the array variable `identifier` at specified `dimension`.

#### `SWAP a, b`

Swap the values of the variable `a` and `b`. They have to share same type.

## API instructions

_« An 🌐[application programming interface (API)](https://en.wikipedia.org/wiki/API) is a connection between computers or between computer programs. »_

Retro Game Creator provide a bunch of built-in [commands](#commands) and [functions](#functions) to communicates with the differents features provided by the fantasy console: input, graphics, sound, memory and more.

### Sprite API

Sprites are limited in numbers and SHOULD be used to show moving objects above background layers. It is not an obligation and interesting effects can be achieved by breaking these rules.

Check [how sprites works](#sprites) and [sprites registers](#sprite-registers).

#### `SPRITE sprite, [x], [y], [character]`

Sets the position `x` and `y` in pixel coordinates and `character` number of the `sprite`.

`x, y` and `character` can be ommited to keep their current value.

An example that show a smiley sprite moving in circle:

    i=0
    DO
        ADD i,1,0 TO 99
        SPRITE 0,COS(i/100)*40+60,SIN(i/100)*40+60,1
        WAIT VBL
    LOOP

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    007EFFFFEDFFFF7E0000003636000000

Omitted parameters will keep their previous values.

#### `SPRITE sprite [PAL palette] [FLIP horizontal, vertical] [PRIO priority] [SIZE size]`

Sets one or more attributes for the `sprite`:

- `PAL palette` Change the `palette` (0..7),
- `FLIP horizontal, vertical` Flip the sprite on `horizontal` and `vertical` axis,
- `PRIO priority` change the `priority` (0..1),
- `SIZE s` Change the size, a.k.a.: the number of characters width and height.

Omitted parameters will keep their previous values.

Example of a sprite that get flipped according to it's position on the screen:

    DO
        SPRITE 0,TOUCH.X,TOUCH.Y,1
        SPRITE 0 PAL 6
        SPRITE 0 FLIP TOUCH.X>SHOWN.W/2,TOUCH.Y>SHOWN.H/2
        WAIT VBL
    LOOP

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    40A0D0E8F4D0E818C06030180C305808

#### `x =SPRITE.X(sprite)`<br>`y =SPRITE.Y(sprite)`

Return the position `x` or `y` of the `sprite` (0..169) in pixels.

Example of a sprite smoothly following the finger touch:

    sprite 0,shown.w,shown.h,1
    do
        x=SPRITE.X(0)+(TOUCH.X-SPRITE.X(0))/4
        y=SPRITE.Y(0)+(TOUCH.Y-SPRITE.Y(0))/4
        SPRITE 0,x,y,
        WAIT VBL
    LOOP

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    007E7E66667E7E00FF81BDA5A5BD81FF

#### `character =SPRITE.C(sprite)`

Return the first `character` number of the `sprite` (0..169).

#### `SPRITE.A sprite, attributes`

Sets all `attributes` at once for the `sprite`.

Check [Character attributes](#character-attributes).

#### `attributes =SPRITE.A(sprite)`

Return the whole `attributes` flags of the `sprite` (0..169).

Example: that show how to read sprite attributes:

    SPRITE 0,80,80,1
    SPRITE.A 0,255
    DO
        TEXT 4,4,"palette: 00000"+BIN$(SPRITE.A(0) AND %111)
        TEXT 4,5,"flip:    000"+BIN$(SPRITE.A(0) AND %11000)
        TEXT 4,6,"size:    0"+BIN$(SPRITE.A(0) AND %1100000)
        WAIT TAP
        SPRITE.A 0,RND(255)
    LOOP

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    40A0D0E8F4D0E818C06030180C305808

#### `SPRITE OFF`

Hides all sprites.

#### `SPRITE OFF sprite`

Hide one `sprite`.

Example that hide a sprite at each tap:

    FOR i=0 TO 159
        SPRITE I,RND(SHOWN.W-8),RND(SHOWN.H-8),1
    NEXT i
    i=0
    WHILE i<159
        WAIT TAP
        SPRITE OFF I
        ADD I,1
    WEND

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    0000000000000000FFFFFFFFFFFFFFFF

#### `SPRITE OFF sprite1 TO sprite2`

Hides all sprites from range `sprite1` to `sprite2` included.

#### `collides =SPRITE HIT(sprite)`

Return if the `sprite` (0..169) `collides` (0/-1) with another sprite. Collision detection is done by checking overlapping pixels that are not transparent (a.k.a.: not using the color 0).

Example of a ball that bounce on the wall and fall into holes:

    again:
    FOR i=1 TO 20
        SPRITE I,RND(SHOWN.W)-8,RND(SHOWN.H)-8,2
    NEXT i
    x=0
    y=0
    sx=1
    sy=1
    DO
        SPRITE 0,x,y,1
        IF SPRITE HIT(0) THEN
            WAIT 30
            GOTO again
        END IF
        WAIT VBL
        x=SPRITE.X(0)+sx
        y=SPRITE.Y(0)+sy
        IF x<0 or x>SHOWN.W-8 THEN sx=-sx
        IF y<0 or y>SHOWN.H-8 THEN sy=-sy
    LOOP

    #1:MAIN PALETTES
    0405

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    003C7E7A72623C00000030240C1C0000
    000000010101021C003C7E7F7F7F3E1C

Use `=HIT` to get which sprite is colliding with the tested `sprite`.

#### `collides =SPRITE HIT(sprite, sprite1)`

Return if the `sprite` (0..169) `collides` (0/-1) with the `sprite1`.

Example of two owerlapping sprites:

    SPRITE 20,46,46,1
    DO
    SPRITE 10,40+((TIMER/60) MOD 2)*4,40,1
    IF SPRITE HIT(10,20) THEN
        TEXT 8,5,"hit"
    ELSE
        TEXT 8,5,"   "
    END IF
    WAIT VBL
    LOOP

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    FFFFFFFFF0F0F0F00000000000000000

Use `=HIT` to get which sprite is colliding with the tested `sprite`.

#### `collides =SPRITE.HIT(sprite, sprite1 to sprite2)`

Return if the `sprite` (0..169) `collides` with any other from the range `sprite1` to `sprite2` included (0..169).

Use `=HIT` to get which sprite is colliding with the tested `sprite`.

#### `sprite =HIT`

Return the `sprite` which collided, resulting of the last `=SPRITE.HIT()` function call (any of the three form).

Example of a ball that bounce on the wall and destroy the obstable:

    FOR i=1 TO 20
        SPRITE i,RND(SHOWN.W)-8,RND(SHOWN.H)-8,2
    NEXT i
    again:
    x=0
    y=0
    sx=1
    sy=1
    DO
        SPRITE 0,x,y,1
        IF SPRITE HIT(0,1 TO 20) THEN
            WAIT 15
            SPRITE OFF HIT
            GOTO again
        END IF
        WAIT VBL
        x=SPRITE.X(0)+sx
        y=SPRITE.Y(0)+sy
        IF x<0 OR x>SHOWN.W-8 THEN sx=-sx
        IF y<0 OR y>SHOWN.H-8 THEN sy=-sy
    LOOP

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

#### `SCROLL layer, [x], [y]`

Set the scroll offset on `x` and `y` axis of the `layer` in pixels.

    TEXT 10,10,"hello!"
    BG 1
    TEXT 10,10,"hello!"
    SCROLL 0,0,-4

Omitted parameters will keep their previous values.

#### `x =SCROLL.X(layer)`<br>`y =SCROLL.Y(layer)`

Return the scroll offset on `x` and `y` axis of the `layer` in pixels.

Example of getting the scroll offset of the background layer:

    FOR x=0 TO 63
    FOR y=0 TO 63
        CELL x,y,RND(3)
    NEXT y
    NEXT x
    do
        x=SCROLL.X(0)+(TOUCH.X-SCROLL.X(0))/4
        y=SCROLL.Y(0)+(TOUCH.Y-SCROLL.Y(0))/4
        SCROLL 0,x,y
        WAIT VBL
    LOOP

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    FFFFFFFFFFFFFFFF0000000000000000
    0000000000000000FFFFFFFFFFFFFFFF
    FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

#### `BG layer`

Change the current `layer` (0..3) for further cells draw operations.

    BG 0
    TEXT 10,10,"\"
    BG 1
    TEXT 10,10,"/"

#### `PAL palette`

Change the current `palette` for further cells draw operations.

    PRINT "hello",
    PAL 1
    PRINT "world"

#### `FLIP [horizontal], [vertical]`

Change the `horizontal` and `vertical` flip attributes for further cells draw operations, considering 0 as not flipped and something else as flipped.

    FLIP 1,0
    PRINT "dlrow olleh"

Omitted parameters will keep their previous values.

#### `PRIO priority`

Change the `priority` for further cells draw operations.

    SPRITE 0,78,78,1
    PRIO 1
    TEXT 10,10,"s"

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    003C7E6A7E7E3C000000183C00000000

#### `ATTR attributes`

Sets the palette, flip and priority `attributes` all at once for further cells draw operations.

Check the [cell attributes reference](#cell-attributes).

#### `CELL x, y, [character]`

Draw to the `x, y` cell of the current layer with the `character` using the current attributes.

By omiting the `character` argument, the command will only alter the attributes: palette, flip and priority.

Use `BG`, `PAL`, `FLIP`, `PRIO` and `ATTR` to alter the [current attributes](#cells-attributes).

E.g. draw a face:

    CELL 10,10,1

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    003C7E6A7E7E3C000000183C00000000

E.g. draw an inverted blue r letter:

    CELL 10,10,242
    FLIP 1,0
    PAL 1
    CELL 10,10,

#### `character =CELL.C(x, y)`

Return the `character` of the `x, y` cell from the current layer.

Example of reading the ASCII code of a character:

    BG 2
    FOR x=0 to SHOWN.W\8
    FOR y=0 to SHOWN.H\8
        CELL x,y,192+RND(63)
    NEXT y
    NEXT x
    BG 1
    BG COPY 0,0,6,4 TO 4,4

    DO
        BG 2
        c=CELL.C(TOUCH.X\8,TOUCH.Y\8)
        BG 0
        TEXT 5,5,"$"+RIGHT$("0"+HEX$(C-192),2)
        WAIT VBL
    LOOP

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
    0000000000000000FFFFFFFFFFFFFFFF

    #3:MAIN BG
    00000604010001000100010001000000
    01000100010001000100020001000100
    01000100010002000000020002000200
    02000200

#### `attributes =CELL.A(x, y)`

Return the `attributes` of the `x, y` call from current layer.

Example that display cell's attributes:

    BG 2
    FOR x=0 to SHOWN.W\8
    FOR y=0 to SHOWN.H\8
        ATTR RND(255)
        CELL X,Y,192+RND(63)
    NEXT y
    NEXT X
    BG 1
    BG FILL 5,5 TO 13,9 CHAR 2
    BG FILL 4,4 TO 12,8 CHAR 1
    BG 0
    PAL 0
    PRIO 1
    FLIP 0,0
    TEXT 5,5,"Pal:"
    TEXT 5,6,"Flip:"
    TEXT 5,7,"Prio:"
    DO
        BG 2
        a=CELL.A(TOUCH.X\8,TOUCH.Y\8)
        BG 0
        TEXT 9,5,RIGHT$("00"+BIN$(A AND %111),3)
        TEXT 10,6,RIGHT$("0"+BIN$((A\8) AND %11),2)
        TEXT 10,7,RIGHT$("0"+BIN$((A\32) AND %11),2)
        WAIT VBL
    LOOP

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
    0000000000000000FFFFFFFFFFFFFFFF

Check the [cell attributes reference](#cell-attributes).

#### `BG FILL x1, y1 TO x2, y2 CHAR character`

Fills all cells from `x1, y1` coordinates to `x2, y2` with `character` and the current attributes.

    BG FILL 1,1 TO 5,5 CHAR 1
    BG FILL 2,2 TO 4,4 CHAR 2

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    FFFFFFFFFFFFFFFF0000000000000000
    0000000000000000FFFFFFFFFFFFFFFF

#### `TINT y, y [PAL palette] [FLIP horizontal, vertical] [PRIO priority]`

Sets to the cell `x, y` (0..63) of the current layer, one or more attributes:

- `PAL palette` Change the `palette` (0..3),
- `FLIP horizontal, vertical` Flip the sprite on `horizontal` and `vertical` axis,
- `PRIO priority` change the `priority` (0..1),

Omitted parameters will keep their previous values.

    PRINT "hello!"
    TINT 5,0 PAL 7

#### `BG TINT y1, y1 TO x2, y2 [PAL palette] [FLIP horizontal, vertical] [PRIO priority]`

Sets to the cells from the rectangle `x1, y1` to `x2, y2` (0..63), one or more attributes:

- `PAL palette` Change the `palette` (0..3),
- `FLIP horizontal, vertical` Flip the sprite on `horizontal` and `vertical` axis,
- `PRIO priority` change the `priority` (0..1),

Omitted parameters will keep their previous values.

#### `BG SCROLL x1, y1 to x2, y2 step x3, y3`

Move the cell's attributes and character from the rectangle `x1, y1` to `x2, y2` (0..63) of the current layer by `x3`,`y3` in cell coordinates.

This feature is internally used to scroll text when it reach the bottom of the window.

#### `BG SOURCE address [,width, height]`

Set the memory `address` to use as source for BG COPY x1,y1,width,height TO x2,y2 operations.

When `width` and `height` are specified, they are used as indiquating the number of cells in the source.

If they are not specified, the [official background data format](#background-source-data) are used to retrieve the width and the height.

> When the program is stared the default address is taken by internally executing `BG SOURCE ROM(3)`.

Example that generate background using random numbers:

    a=$9000
    FOR r=0 TO 63
    FOR c=0 TO 63
        POKE A,1
        INC a
        POKE a,RND(1)*8
        INC a
    NEXT c
    NEXT r

    bG SOURCE $9000,64,64
    BG COPY 0,0,64,64 TO 0,0

    Do
    x=0
    ADD x,SIN(TIMER/600)*COS(TIMER/6000)*200
    ADD x,SIN(TIMER/500)*50

    y=0
    ADD y,COS(TIMER/400)*SIN(TIMER/4000)*200
    ADD y,COS(TIMER/700)*50

    SCROLL 0,X,Y
    WAIT VBL
    LOOP

#### `BG COPY x1, y1, width, height TO x2, y2`

Copy from the rectangle `x1, y1` (0..63) with `width, height` (0..63) the cell's attributes and character numbers from background source specified previously using `BG SOURCE` to `x2, y2` (0..63) of the current background layer.

#### `MCELL x, y, character`

Draw to the `x, y` cell to the background source specified previously using `BG SOURCE` with the `character` using the current attributes.

Similar to `CELL` but modify the source in memory instead of the current layer.

The source MUST point to writable memory. It will NOT work with `BG SOURCE ROM(3)`.

Example that continously draw to the background source:

    TEXT 4,4,"touch to copy"
    BG 1
    BG SOURCE $A000,64,64

    DO
        PAL RND(7)
        MCELL RND(CEIL(SHOWN.W/8)),RND(CEIL(SHOWN.H/8)),RND(1)
        IF TOUCH THEN
            BG COPY 0,0,CEIL(SHOWN.W/8),CEIL(SHOWN.H/8) TO 0,0
        END IF
        WAIT VBL
    LOOP

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    FFFFFFFFFFFFFFFF0000000000000000

#### `character =MCELL.C(x, y)`

Return the `character` of the `x, y` cell from the background source.

Similar to `=CELL.C` but modify the source in memory instead of the current layer.

#### `attributes =MCELL.A(x, y)`

Return the `attributes` of the `x, y` call from the background source.

Similar to `=CELL.A` but modify the source in memory instead of the current layer.

#### `TEXT x, y, text$`

Will print `text$` on the current layer starting at `x, y` cell using the current background attributes on the current layer.

  DATA "ga","bu","zo","meu"
  FOR i=0 TO 3
    PAL I
    READ c$
    TEXT 8+i*2,10,c$
  NEXT i

#### `NUMBER x, y, number, count`

Will print the latest `count` digits of the `number` on the current layer starting at `x, y` cell using the current background attributes on the current layer.

Similar to `TEXT` but dedicated to print number instead of text.

    score=123
    NUMBER 10,10,score,6

#### `FONT first`

Sets the `first` character number that will defines the range of ASCII characters used for `TEXT`, `NUMBER`] and `PRINT` commands.

The default value is 192, which points to where the default font is loaded at the beginning of the program if the characters are not used. It MAY be overridden by `LOAD` operation.

### Text API

Commands dedicated to draw text on layers using characters data. 64 characters can be reserved to print characters on screen. By default those characters number are (192..255) but can be changed using the `FONT`.

#### `WINDOW x, y, width, height, layer`

Sets the current window at `x, y` with `width, height` and on `layer` where text will be drawn.

    WINDOW 4,8,SHOWN.W\8-8,20,0
    PRINT "Oh my god! There's a tremendous amount of work to do. Can I do a little bit of it today."

By default, the window is sets inside the safe area delimited by the safe functions.

#### `CLW`

Clears the window by replacing all the cells by the character 0. It also reset the cursor position.

#### `LOCATE x, y`

Move the cursor location at `x, y` in cells coordinates inside the window.

    FOR i=1 TO 9
        LOCATE I,I
        PRINT str$(i);
    NEXT i

#### `x =CURSOR.X`<br>`y =CURSOR.Y`

Return the cursor location `x, y` in cells coordinates inside the window.

#### `PRINT expression...`

Outputs string `expression...` onto the current window.

`expression...` can be one or more of:
- a string or numeric literal,
- a variable identifier,
- a label that point to a string data.

`expression...` items can be seperated by:
- a coma (,) to add a space between items,
- a semicolon (;) to not add space e

Example with different expressions:

    PRINT "literal"
    v$="variable"
    PRINT v$
    txt: DATA "data"
    PRINT txt

Example with different separators:

    PRINT "glu";"ed"
    PRINT "sepa","rated"

    DO
        WAIT 2
        PRINT ".";
    LOOp

#### `PRINT`

Outputs a new line.

### Input API

Retro Game Creator support touch input with float-point precision (but not multi-touches), and limited keyboard input.

User can also rely on the OS virtual keyboard to capture typed characters.

#### `touched =TOUCH`

Return -1 if the device fantasy screen is currently `touched`.

Example that print a text only when device screen is touched:

    DO
        CLS
        IF TOUCH THEN TEXT 4,4,"touched"
        WAIT VBL
    LOOP

#### `touched =TAP`

Return -1 for exactly one frame, if the device fantasy screen is `touched`.

Example of a flappy letter going down by gravity and up when tapping on the screen:

    SPRITE 0,0,SHOWN.H/2,226
    gravity=3
    DO
        WAIT VBL
        x=SPRITE.X(0)+1
        IF x+8>SHOWN.W THEN X=0
        IF TAP THEN gravity=-9
        y=CLAMP(SPRITE.Y(0)+GRAVITY,0,SHOWN.H-8)
        gravity=MIN(3,gravity+1)
        SPRITE 0,x,y,
    LOOP

#### `x =TOUCH.X`<br>`y =TOUCH.Y`

Returns the last pixel position `x, y` touched. It returns a floating-point number, with a 1/16 pixel precisions.

    DO
        CLS
        PRINT TOUCH.X;",";TOUCH.Y
        WAIT VBL
    LOOP

#### `KEYBOARD ON`<br>`KEYBOARD OFF`

Show or hide the device virtual keyboard. Because user can hide the keybord using a dedicated key, do not assume the keyboard is visible. Use `KEYBOARD ON` inside a loop or rely on `=KEYBOARD` to detect when the keyboard is shown or not.

#### `height =KEYBOARD`

Return the `height` of the virtual screen that has been occluded by the keyboard.

#### `INPUT [prompt;] variable`<br>`INPUT [prompt;] variable$`

Wait for the user to type a text or a number and store it to the `variable` or `variable$`.

Optionally, a `prompt` can be printed on the screen before the user input.

    a=RND(9)+10
    b=RND(9)
    PRINT "Captcha:"
    PRINT STR$(A);"+";STR$(B)
    DO
        INPUT "?";answer
        IF answer=a+b THEN GOTO pass
    LOOP
    pass: PRINT "you may pass"

#### `pressed$ =INKEY$`

Returns only once the last `pressed$` key. It's a string containing one ASCII character supported by Retro Game Creator. If no key was pressed or if the pressed key is not supported, it returns an empty string. See [ASCII table](#ascii-table).

Example that output the pressed key:

    DO
        KEYBOARD ON
        k$=INKEY$
        IF k$<>"" THEN
            PRINT k$;
        END IF
        WAIT VBL
    LOOP

#### `frames =TIMER`

Returns the number of `frames` since Retro Game Creator was launched. The value wraps to 0 when 5184000 is reached, which is about 24 hours.

#### `WAIT TAP`

Will stop execution of the program until a touch is made.

While waiting for a tap, interrupt sub-routines for VBL/RASTER/PARTICLE/EMITTER are still executed.

### Display API

#### `PALETTE palette, [c0], [c1], [c2], [c3]`

Sets the four colors on the height available `palette` (0..7). The color 0 of the palette 0 is generally used as backdrop color. (It can be change using the `SYSTEM` command.) `c0`, `c1`, `c2`, `c3` can accept a numeric value between 0 and 63, omit them to keep the current value. Consult the [64 colors reference](#64-colors) to choose the color you want.

#### `available =COLOR(palette, color)`

Returns one of the `available` [64 colors](#64-colors) associated to the pair `palette` (0..7), `color` (0..3).

#### `width =SHOWN.W`<br>`height =SHOWN.H`

Returns the `width` and `height` in pixels of the visible area of the fantasy screen.

#### `left =SAFE.L`<br>`top =SAFE.T`<br>`right =SAFE.R`<br>`bottom =SAFE.B`

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

It is RECOMMENDED to place at least one `WAIT VBL` inside every loop that waits for player input or presents something to the player.

Example that show the effect:

    SPRITE 0,SHOWN.W/2-4,SHOWN.H/2-4,240
    LOCATE 2,2
    PRINT "touch to wait vbl"
    DO
        x=SPRITE.X(0)
        y=SPRITE.Y(0)
        ADD X,1,-8 TO SHOWN.W
        ADD Y,1,-8 TO SHOWN.H
        SPRITE 0,x,y,
        IF TOUCH THEN WAIT VBL
    LOOP

> `WAIT VBL` is equal to `WAIT 1`.

#### `ON VBL CALL procedure`

Before the execution of the code for the next frame, will execute the `procedure`.

The code MUST be short, read more about [CPU cycles](#cycles).

This can be used to execute code at every frame, independently of which part of the code is generating frames. Here is an example that demonstrate:

    SUB v
        x=SPRITE.X(0)
        ADD x,1,-8 TO SHOWN.W
        y=SPRITE.Y(0)
        ADD y,1,-8 TO SHOWN.H
        SPRITE 0,x,y,1
    END SUB
    ON VBL CALL v

    one:
    PALETTE 0,43,,,
    WAIT TAP
    GOTO two

    two:
    PALETTE 0,59,,,
    WAIT TAP
    GOTO one

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    0000000000000000FFFFFFFFFFFFFFFF

#### `ON VBL OFF`

Will stop the execution of a sub-routine during VBL interrupt.

#### `ON RASTER CALL procedure`

Before the rendering of the next line of the screen, will execute the `procedure`. This is useful in conjunction with `=RASTER`.

The code MUST be shortest, read more about [CPU cycles](#cycles).

This can be used to change colors or scroll background layers, example:

    TEXT 3,8,"tap to toggle"
    CELL 10,10,1

    SUB R
    IF RASTER>=80 THEN
        SCROLL 0,0,80-RASTER
        PALETTE 0,,,RASTER MOD 64,
    ELSE IF RASTER=0 THEN
        SCROLL 0,0,0
        PALETTE 0,,,0,
    END IF
    END SUB

    DO
        WAIT TAP
        ON RASTER CALL R
        WAIT TAP
        ON RASTER OFF
        SCROLL 0,0,0
        PALETTE 0,,,0,
    LOOP

#### `line =RASTER`

Return the fantasy screen `line` number currently rendered.

#### `ON RASTER OFF`

Will stop the execution of a sub-routine during RASTER interrupt.

### Math API

#### `pi =PI`

Return the constant π.

#### `cosine =COS(number)`<br>`sine =SIN(number)`

Return the `cosine` or `sine` (0..1) value of a `number`.

`=COS()` and `=SIN()` take (0..1) instead of (0..π*2), and `=SIN()` is inverted.

#### `arc =ATAN(x, y)`

Return the `arc` tangent value of a `x, y` vector.

`ATAN()` returns an value between (-0.5..0.5) making it suitable for use with `COS()` and `SIN()`.

Example that show how to compute an angle from a position, and a position from an angle:

    'get the center position of the screen
    cx=SHOWN.W/2-4
    cy=SHOWN.H/2-4
    SPRITE 50,cx,cy,1

    'initial position from the center
    dx=40
    dy=20
    SPRITE 40,cx+dx,cy+dy,1
    SPRITE 40 PAL 1
    SPRITE 30 PAL 2

    DO
        'new position from touch
        dx=TOUCH.X-cx
        dy=TOUCH.Y-cy
        SPRITE 40,cx+dx,cy+dy,1

        'get angle from vector
        a=ATAN(dx,dy)

        'get position from angle
        x=COS(a)*30
        y=SIN(a)*30

        SPRITE 30,cx+x,cy+y,1
        WAIT VBL
    LOOP

    #2:MAIN CHARACTERS
    00000000000000000000000000000000
    3C7EFFFFFFFF7E3C0000000000000000

#### `absolute =ABS(number)`

Return the `absolute` value of a `number`.

#### `exponential =EXP(number)`

Return the `exponential` value of a `number`.

#### `logarithm =LOG(number)`

Return the `logarithm` value of a `number`.

#### `square =SQR(number)`

Return the `square` root value of a `number`.

#### `sign =SGN(number)`

Return the `sign` (+1/-1) of a `number`.

#### `floor =FLOOR(number)`<br>`floo =INT(number)`

Return the `floor` value of a `number`.

#### `ceil =CEIL(number)`

Return the `ceil` value of a `number`.

#### `INC variable`<br>`DEC variable`

Increment or decrement by 1 the value of a `variable`.

#### `ADD variable, value`<br>`ADD variable, increment, min TO max`

Add a `value` to a `variable`.

Optionally make it wrap around between `min` and `max`.

#### `minimal =MIN(a, b)`<br>`maximal =MAX(a, b)`

Return the `minimal` or `maximal` value between `a` and `b`.

#### `clamped =CLAMP(value, min, max)`

Return the `value` `clamped` between `min` and `max`.

#### `length =LEN(x, y)`

Return the `length` of a `x, y` vector.

#### `interpolation =EASE(function, mode, value)`

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

#### `random =RND`

Return a `random` floating-point number (0..1).

#### `random =RND(max)`

Return a `random` integer number (0..max).

#### `RANDOMIZE seed`

Initialize the random generator with a different `seed` (0..16777216).

TODO: link to floating point precision loss

Setting a specific `seed` will guarantee to return the same sequence of random numbers.

    RANDOMIZE 123456
    FOR I=0 TO 5
        PRINT RND(100)
    NEXT I

    PRINT

    RANDOMIZE 123456
    FOR I=0 TO 5
        PRINT RND(100)
    NEXT I

#### `random =RND address`<br>`random =RND(max, address)`<br>`RANDOMIZE seed [,address]`

Similar to the original functions with a additional `address` parameter, where the current generator state will be stored.

This is useful if you want to keep multiple generator that still need to be deterministic.

The generator state consume 16 Bytes of memory.

### String API

#### `length =LEN(text)`

Return the `length` in ascii7 character of a `text` string.

#### `ascii7 =ASC(character)`

Converts the first `character` of a string to an `ascii7` code. See [ASCII table](#ascii-table).

#### `character =CHR$(ascii7)`

Converts an `ascii7` code to a one `character` string.

#### `text =STR$(number)`

Converts a `number` to a `text` string.

    print str$(123.456)

#### `number =VAL(text)`

Converts a `text` string that starts with digits (and optionally one decimal point .) into a `number`.

    print val("3.5abc")

#### `hexadecimal =HEX$(number, [digits])`<br>`binary =BIN$(number, [digits])`

Converts a `number` to an `hexadecimal` or `binary` string with an optional minimal number of `digits`.

#### `leading$ =LEFT$(text, length)`<br>`trailing$ =RIGHT$(text, length)`

Return the `length` number of characters from the beginning or the end of the `text` string.

    shadok: DATA "gabuzomeu"
    PRINT LEFT$(shadok,4)
    PRINT RIGHT$(shadok,5)

#### `LEFT$(text, length) =replacement$`<br>`RIGHT$(text, length) =replacement$`

Overwrites the `length` number of characters at the beginning or the end of the `text` string by the `length` first characters of the `replacement$` string.

#### `extract$ =MID(text, position, length)`

Return the `length` number of characters starting at `position` (1..) from the `text` string.

#### `MID$(text, position, length) =replacement$`

Overwrites the `length` number of characters starting at `position` (1..) from the `text` string by the `length` first characters of the `replacement$` string.

    six$="hexakosioihexekontahexaphobie"
    MID$(six$,1,4)="-66-"
    MID$(six$,11,4)="-66-"
    MID$(six$,20,4)="-66-"
    PRINT six$

#### `found =INSTR(text, occurence [,position])`

Search for the first `occurence` inside `text` and return the `found` position. Return 0 if not found. Optionally, start the search at `position`.

### Sound API

The fantasy hardware simulates 4 voices to play sound effects and music. See [Sound](#sound) for an overview.

#### `PLAY voice, pitch [,length] [SOUND sound]`

Play a note at the `pitch` on the `voice` with an optional `length` and `sound`.

Check the [pitch references](#pitch-values) to learn which pitch correspond to which notes.

`length` use 1/60 of seconds as units with the maximum being 255, so 4.25 seconds. A length of 0 means that the sound will not stop until another sound is played on the same `voice`.

`sound` is a number representing a group of settings that define the sound's characteristics. It requires the usage of the [`SOUND SOURCE`](<manual#[`SOUND SOURCE [address]`](#sound-source-address)>) command.

#### `STOP`<br>`STOP [voice]`

Stops the sound and track on the `voice` or all the voices if omitted.

Release duration of envelope are not stop but fade out instead.

#### `VOLUME voice, [volume], [mix]`

Sets the `volume` and `mix` of `voice`.

| parameter | value and range                        |
|----------:|----------------------------------------|
|  `volume` | 0 .. 15                                |
|     `mix` | 0 Muted<br>1 Left<br>2 Right<br>3 Both |

All parameters can be omitted to keep their current settings.

#### `SOUND voice, [wave], [width], [length]`

Sets the sound's characteristics for the `voice`.

`wave` control the waveform. With a waveform of Pulse, the `width` control the pulse width. A value of 8 means a square wave.

`length` control the length of the sound. A length of 0 means that the sound will not stop until another sound is played on the same `voice`. This value can be overriden by the parameter `length` of the [`PLAY` command](#play-voice-pitch-length-sound-sound).

| parameter | value and range                                |
|----------:|------------------------------------------------|
|    `wave` | 0 Sawtooth<br>1 Triangle<br>2 Pulse<br>3 Noise |
|   `width` | 0 .. 15                                        |
|  `length` | 0 Infinite<br>1 16.67ms .. 255 4.25s           |

Omitted parameters will keep their previous values.

#### `ENVELOPE voice, [attack], [decay], [sustain], [release]`

Set the volume envelope for the `voice`.

Allow to change the `attack`, `decay` and `release` duration.

`sustain` control the volume after the decay and before the release.

| parameter | range           |
|----------:|-----------------|
|  `attack` | 0 2ms .. 15 12s |
|   `decay` | 0 2ms .. 15 12s |
| `release` | 0 2ms .. 15 12s |
| `sustain` | 0 .. 15         |

All parameters can be omitted to keep their current settings.

#### `LFO voice, [rate], [frequency], [volume], [width]`

Set the Low Frequency Oscillator (LFO) for the `voice`.

Allow to change the `rate`, the `frequency`, the `volume` and the `width` for Pulse waveform.

|   parameter | range               |
|------------:|---------------------|
|      `rate` | 0 0.12Hz .. 15 18Hz |
| `frequency` | 0 .. 15             |
|    `volume` | 0 .. 15             |
|     `width` | 0 .. 15             |

All parameters can be omitted to keep their current settings.

#### `LFO WAVE voice, [wave], [invert], [env], [trigger]`

Set options for the a second Low Frequency Oscillator (LFO) for the `voice`.

Allow to change the `wave` and enable or disable `invert`, `env` and `trigger`.

With `invert` off, the wave is added (+) to the output signal. With `invert` on, the wave is substracted (-).

When `env` on, this second LFO will be played once. Will implictly set `trigger` on.

With `trigger` on, this second LFO is restarted at every `PLAY`, otherwise the LFO is continously applied.

| parameter | value                                            |
|----------:|--------------------------------------------------|
|    `wave` | 0 Triangle<br>1 Sawtooth<br>2 Square<br>3 Random |
|  `invert` | 0 Off<br>1 On                                    |
|     `env` | 0 Off<br>1 On                                    |
| `trigger` | 0 Off<br>1 On                                    |

All parameters can be omitted to keep their current settings.

#### `SOUND SOURCE [address]`

Set the `address` on memory to use as source for `MUSIC`, `TRACK` and `PLAY` commands. The [sound source format data](#sound-source-format-data) is used to decode the information.

This will not affect already started playback.

If not specified the default address is taken by internally executing `=ROM(15)`

#### `MUSIC [pattern]`

Starts playing at the `pattern`. If omitted, it starts at pattern 0.

This will consider that the data respects the [sound source format data](#sound-source-format-data).

#### `value =MUSIC(what)`

Question the current playback.

| what | value                               |
|-----:|-------------------------------------|
|    0 | The current pattern                 |
|    1 | The current row                     |
|    2 | The current tick                    |
|    3 | The current speed<br>0 when stopped |

#### `TRACK track, voice`

Starts playing the `track` on `voice`.

This will consider that the data respects the [sound source format data](#sound-source-format-data).

### Memory API

#### `POKE address, value`

Write an 8bits `value` (0..255) in memory at `address`.

The `address` MUST be writable. See [memory mapping](#memory-mapping) for details.

Example that change the background color:

    c=0
    DO
        POKE $FF00,C
        ADD c,1,0 TO 63
        WAIT 5
    LOOP

#### `value =PEEK(address)`

Read and return a 8bits `value` (0..255) from memory at `address`.

Example that print the last pressed ASCII key code:

    DO
        KEYBOARD ON
        PRINT STR$(PEEK($FF84))
        WAIT 5
    LOOp

#### `POKEW address, value`

Write a 16bits `value` (-32768..32767) in memory at `address`.

#### `value =PEEKW(address)`

Read and return a 16bits `value` (-32768..32767) from memory at `address`.

Example that print the width and height of the visible pixels:

    PRINT "width",PEEKW($FF78),SHOWN.W
    PRINT "height",PEEKW($FF7A),SHOWN.H

#### `POKEL address, value`

Write a 32bits `value` value (-16777216..16777216) in memory at `address`.

Values are stored as floating-point. See [number limits](#number-limits) for precision details.

#### `value =PEEKL(address)`

Read and return a 32bits `value` (-16777216..16777216) from memory at `address`.

Values are stored as floating-point. See [number limits](#number-limits) for precision details.

#### `address =ROM(file)`

Return the `address` in memory of the virtual `file`.

#### `size =SIZE(file)`

Return the `size` in bytes of the virtual `file`.

#### `COPY source, count TO destination`

Copies `count` bytes from `source` address from memory to `destination` address in memory.

The `source` and `destination` CAN overlap.

#### `FILL address, count [,value]`

Sets `count` bytes from `address` in memory with the `value` or 0 when not specified.

#### `ROL address, places`<br>`ROR address, places`

Rotates the bits of the byte stored at `address` by a number of `places` to the left or the right.

    POKE $9000,%00001111
    again:
    PRINT RIGHT$("0000000"+BIN$(PEEK($9000)),8)
    WAIT TAP
    ROL $9000,2
    GOTO AGAIN

#### `DMA COPY [ROM]`

Performs a fast memory copy that can only be done during interrupt calls: VBL/RASTER/EMITTER/PARTICLE.

It uses the following registers as data:

| address | size    | purpose             |
| -------:| ------- | ------------------- |
|   $FFA0 | 2 Bytes | Source address      |
|   $FFA2 | 2 Bytes | Bytes count         |
|   $FFA4 | 2 Bytes | Destination address |

To copy from an address greater or equal than `$10000`, the OPTIONAL `[ROM]` argument MUST be used.

### File API

Give access to the [virtual file embedded](#virtual-file) in the cartridge.

#### `FILES`

Enable the access to the files, allowing to use the other commands of the file API.

Getting access to files allow to create editor program, like characters, map or custom editor. The official GFX and SFX use this command.

When the `FILES` command is called, it will map the data stored in the fantasy cartridge to virtual files indexed from 0 to 15.

#### `comment$ =FILE$(file)`

Return the `comment$` string of the `file`.

The command `FILES` MUST be called before.

#### `size =FSIZE(file)`

Return the `size` in bytes stored in the `file`.

The command `FILES` MUST be called before.

#### `LOAD file, address [,limit [,offset]]`

Load the virtual `file` at `address`. Optionnaly `limit` the number of bytes read and start at `offset`.

The command `FILES` MUST be called before.

#### `SAVE file, comment$, address, size`

Save `size` bytes from `address` to the `file` and **erase previous data**. The `comment$` is just a reminder of what is stored.

The command `FILES` MUST be called before.

### Other API

#### `TRACE expression [,expression...]`

Output the evaluated `expression` on the [overlay](#overlay).

#### `MESSAGE text`

Output `text` in the bottom-left corner of the fantasy screen, in the [overlay](#overlay).

#### `SYSTEM setting, value`

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

By typing the name of a variable, the debugger will print it's value. The variable use the same syntax as inside a program: `$` for string, () for array.

The variable MUST be accessible throughout the scope of where the `PAUSE` has been used to enter the debugger. Global variables are still available everywhere.

#### dbg: a variable name `=` new value

Allow to change the value of a variable.

Number literal use the same syntax as inside a program, it support integer, float, hexadecimal and binary.

String literal SHOULD use the same syntax as inside a program: ".

#### dbg: an address

By typing a memory address, the debugger will print it's value as if it was read by `=PEEK()`.

The address can be indicated using the hexadecimal or by any other valid numeric literal.

#### dbg: an address `=` a value

Will try to modify the value store inside a memory address.

    $FF00=3

#### dbg: `CLS`

Clear the debugger console.

#### dbg: `WAIT`

Resume execution until a `WAIT` command is found in the program, the scope may change.

#### dbg: `DIM [filter] [pagination]`

Print the list of accessible variable at the current scope.

Allow to limit the output to the variables that matchs the `filter`.

Allow to output more variables using the `pagination` with an index that start at 0 zero.

#### dbg: `TRACE`

Print the current call stack in order: label and procedure names.

## References

This annex provides technical information about how things work under the hood.

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

| address | size        | purpose            |
| -------:| ----------- | ------------------ |
|  $00000 | 8 Kibibyte  | Layer 0 data       |
|  $02000 | 8 Kibibyte  | Layer 1 data       |
|  $04000 | 8 Kibibyte  | Layer 2 data       |
|  $06000 | 8 Kibibyte  | Layer 3 data       |
|  $08000 | 4 Kibibyte  | Character data     |
|  $09000 | 20 Kibibyte | Working RAM        |
|  $0E000 | 6 Kibibyte  | Persistent RAM     |
|  $0FB00 | 1020 Bytes  | Sprite registers   |
|  $0FF00 | 32 Bytes    | Color registers    |
|  $0FF20 | 10 Bytes    | Video registers    |
|  $0FF40 | 48 Bytes    | Audio registers    |
|  $0FF70 | 28 Bytes    | I/O registers      |
|  $0FFA0 | 6 Bytes     | DMA registers      |
|  $0FFA6 | 10 Bytes    | Internal registers |
|  $10000 | 64 Kibibyte | Cartridge ROM      |

TODO: Add particle/emitter registers

### TODO: Color data

### Background data

The 4 background layers use the following format:
- 64x64 cells
- 2 bytes per cell

For each cell:

| address | purpose                   |
| -------:| ------------------------- |
|      +0 | character number (0..255) |
|      +2 | cell attributes           |

### Cell attributes

|  bit mask | purpose                |
| ---------:| ---------------------- |
| %00000111 | palette number (0..7)  |
| %00001000 | horizontal flip (0..1) |
| %00010000 | vertical flip (0..1)   |
| %00100000 | priority flag (0..1)   |

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

When using `BG SOURCE` without specifying `width` and `height`, a specific header is added before the regular [background data](#background-data):

| address | size   | purpose         |
| -------:| ------ | --------------- |
|      +0 | 1 Byte | always zero     |
|      +1 | 1 Byte | always zero     |
|      +2 | 1 Byte | width in cell   |
|      +3 | 1 Byte | height in cell  |
|      +4 | ...    | background data |

### Sound source format data

When using `SOUND SOURCE`, Retro Game Creator will use the following data format to store sounds, patterns and tracks.

TODO: continue

|  address | purpose          |
| --------:| ---------------- |
|       +0 | 16 sound presets |
|     +128 | 64 patterns      |
|     +384 | 64 tracks        |

For each sound preset:

| address | size | purpose                                 |
| -------:| ---- | --------------------------------------- |
|      +0 | 1    | [Attributes](<manual#Attributes bits:>) |
|      +1 | 1    | Length                                  |
|      +2 | 2    | [Envelope](<manual#Envelope bits:>)     |
|      +4 | 1    | LFO attributes                          |
|      +5 | 2    | LFO settings                            |
|      +7 | 1    | Not used                                |

#### Attributes bits:

| bits | purpose                                                        |
| ----:| -------------------------------------------------------------- |
| 0..3 | Pulse width                                                    |
| 4..5 | Wave<br>0 Sawtooth<br>1 Triangle<br>2 Pulse<br>3 Noise |
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

Retro Game Creator understand ASCII7 characters (0..127).

The [text API](#text-api) can only print a fraction of it and remap it. The `FONT` is used to indicate which characters are use to print the ASCII characters.

| ascii code | usage                         |
| ----------:| ------------------------------|
|      0..31 | not used                      |
|     32..95 | used by [`FONT`](#font-first) |
|    95..255 | not used                      |

<br>

| code | hexa | character   |
| ----:| ----:| ----------- |
|   17 |  $11 | right arrow |
|   18 |  $12 | left arrow  |
|   19 |  $13 | down arrow  |
|   20 |  $14 | up arrow    |
|   32 |  $20 | whitespace  |
|   33 |  $21 | "!"         |
|   34 |  $22 | '"'         |
|   35 |  $23 | "#"         |
|   36 |  $24 | "$"         |
|   37 |  $25 | "%"         |
|   38 |  $26 | "&"         |
|   39 |  $27 | "'"         |
|   40 |  $28 | "("         |
|   41 |  $29 | ")"         |
|   42 |  $2A | "*"         |
|   43 |  $2B | "+"         |
|   44 |  $2C | ","         |
|   45 |  $2D | "-"         |
|   46 |  $2E | "."         |
|   47 |  $2F | "/"         |
|   48 |  $30 | "0"         |
|   49 |  $31 | "1"         |
|   50 |  $32 | "2"         |
|   51 |  $33 | "3"         |
|   52 |  $34 | "4"         |
|   53 |  $35 | "5"         |
|   54 |  $36 | "6"         |
|   55 |  $37 | "7"         |
|   56 |  $38 | "8"         |
|   57 |  $39 | "9"         |
|   58 |  $3A | ":"         |
|   59 |  $3B | ";"         |
|   60 |  $3C | "<"         |
|   61 |  $3D | "="         |
|   62 |  $3E | ">"         |
|   63 |  $3F | "?"         |
|   64 |  $40 | "@"         |

| code | hexa | character   |
| ----:| ----:| ----------- |
|   65 |  $41 | "A"         |
|   66 |  $42 | "B"         |
|   67 |  $43 | "C"         |
|   68 |  $44 | "D"         |
|   69 |  $45 | "E"         |
|   70 |  $46 | "F"         |
|   71 |  $47 | "G"         |
|   72 |  $48 | "H"         |
|   73 |  $49 | "I"         |
|   74 |  $4A | "J"         |
|   75 |  $4B | "K"         |
|   76 |  $4C | "L"         |
|   77 |  $4D | "M"         |
|   78 |  $4E | "N"         |
|   79 |  $4F | "O"         |
|   80 |  $50 | "P"         |
|   81 |  $51 | "Q"         |
|   82 |  $52 | "R"         |
|   83 |  $53 | "S"         |
|   84 |  $54 | "T"         |
|   85 |  $55 | "U"         |
|   86 |  $56 | "V"         |
|   87 |  $57 | "W"         |
|   88 |  $58 | "X"         |
|   89 |  $59 | "Y"         |
|   90 |  $5A | "Z"         |

| code | hexa | character   |
| ----:| ----:| ----------- |
|   91 |  $5B | "["         |
|   92 |  $5C | "\"         |
|   93 |  $5D | "]"         |
|   94 |  $5E | "^"         |
|   95 |  $5F | "_"         |
|   96 |  $60 | "`"         |
|   97 |  $61 | "a"         |
|   98 |  $62 | "b"         |
|   99 |  $63 | "c"         |
|  100 |  $64 | "d"         |
|  101 |  $65 | "e"         |
|  102 |  $66 | "f"         |
|  103 |  $67 | "g"         |
|  104 |  $68 | "h"         |
|  105 |  $69 | "i"         |
|  106 |  $6A | "j"         |
|  107 |  $6B | "k"         |
|  108 |  $6C | "l"         |
|  109 |  $6D | "m"         |
|  110 |  $6E | "n"         |
|  111 |  $6F | "o"         |
|  112 |  $70 | "p"         |
|  113 |  $71 | "q"         |
|  114 |  $72 | "r"         |
|  115 |  $73 | "s"         |
|  116 |  $74 | "t"         |
|  117 |  $75 | "u"         |
|  118 |  $76 | "v"         |
|  119 |  $77 | "w"         |
|  120 |  $78 | "x"         |
|  121 |  $79 | "y"         |
|  122 |  $7A | "z"         |

### Registers

Registers are one or more bytes mapped in memory thats as an internal usage.

#### Character Registers

Each one of the 256 character occupies 16 bytes.

| address | size     | purpose         |
| -------:| -------- | --------------- |
|   $8000 | 16 Bytes | 1st character   |
|   $8010 | 16 Bytes | 2nd character   |
|   $8020 | 16 Bytes | 3rd character   |
|       … | 16 Bytes | …               |
|   $8FE0 | 16 Bytes | 255th character |
|   $8FF0 | 16 Bytes | 256th character |

#### Sprite Registers

There are 170 sprites, each occupies 6 bytes:

| address | size    | purpose      |
| -------:| ------- | ------------ |
|   $FB00 | 6 Bytes | 1st sprite   |
|   $FB06 | 6 Bytes | 2nd sprite   |
|   $FB0C | 6 Bytes | 3rd sprite   |
|       … |         | …            |
|   $FEF6 | 6 Bytes | 170th sprite |
|   $FEFC | 4 Bytes | Not used     |

For each sprite:

| offset | size    | purpose              |
| ------:| ------- | -------------------- |
|     +0 | 2 Bytes | Position on X axis   |
|     +2 | 2 Bytes | Position on Y axis   |
|     +4 | 1 Byte  | Character number     |
|     +5 | 1 Byte  | Character attributes |

Both position on x and y axis use sub-pixels values. To advance by 1 pixel, the values SHOULD get increased by 16.
Also, they are both offseted by 32 pixels. To place a sprite in the 0x0 coordinates, the values SHOULD be 512x512.

#### Character attributes

| bits | purpose         |
| ----:| --------------- |
| 0..2 | Palette number  |
|    3 | Horizontal flip |
|    4 | Vertical flip   |
|    5 | Priority        |
| 6..7 | Size            |

Sprite size:

| binary | purpose                        |
| ------:| ------------------------------ |
|    %00 | 8x8 pixels or 1x1 character    |
|    %01 | 16x16 pixels or 2x2 characters |
|    %10 | 24x24 pixels or 3x3 characters |
|    %11 | 32x32 pixels or 4x4 characters |

#### Color registers

There are 8 palettes with 4 colors each:

| address | size     | purpose         |
| -------:| -------- | --------------- |
|   $FF00 | 32 Bytes | Color registers |

For each palette:

| offset | size   | purpose         |
| ------:| ------ | --------------- |
|     +0 | 1 Byte | 1st color value |
|     +1 | 1 Byte | 2nd color value |
|     +2 | 1 Byte | 3rd color value |
|     +3 | 1 Byte | 4th color value |

#### Video registers

| address | size     | purpose                     |
| -------:| -------- | --------------------------- |
|   $FF20 | 2 Bytes  | Background layer 0 scroll X |
|   $FF22 | 2 Bytes  | Background layer 0 scroll Y |
|   $FF24 | 2 Bytes  | Background layer 1 scroll X |
|   $FF26 | 2 Bytes  | Background layer 1 scroll Y |
|   $FF28 | 2 Bytes  | Background layer 2 scroll X |
|   $FF2A | 2 Bytes  | Background layer 2 scroll Y |
|   $FF2C | 2 Bytes  | Background layer 3 scroll X |
|   $FF2E | 2 Bytes  | Background layer 3 scroll Y |
|   $FF30 | 2 Bytes  | Raster line number          |
|   $FF32 | 1 Byte   | Display attributes          |
|   $FF33 | 12 Bytes | Not used                    |

#### Display attributes

| bits | purpose                    |
| ----:| -------------------------- |
|    0 | Sprites enabled            |
|    1 | Background layer 0 enabled |
|    2 | Background layer 1 enabled |
|    3 | Background layer 2 enabled |
|    4 | Background layer 3 enabled |

#### TODO: Audio registers

| address | size     | purpose         |
| -------:| -------- | --------------- |
|   $FF40 | 48 Bytes | Audio registers |

#### I/O registers

| address | size    | purpose                        |
| -------:| ------- | ------------------------------ |
|   $FF70 | 4 Bytes | Last touch position X          |
|   $FF74 | 4 Bytes | Last touch position Y          |
|   $FF78 | 2 Bytes | Pixels shown in width          |
|   $FF7a | 2 Bytes | Pixels shown in height         |
|   $FF7c | 2 Bytes | Pixels outside the safe zone   |
|   $FF7e | 2 Bytes | Pixels outside the safe zone   |
|   $FF80 | 2 Bytes | Pixels outside the safe zone   |
|   $FF82 | 2 Bytes | Pixels outside the safe zone   |
|   $FF84 | 1 Byte  | ASCII code of last pressed key |
|   $FF85 | 1 Byte  | Other I/O status bits          |

Last touch position X and Y are stored as float and currently Retro Game Creator do not have a way to peek float from memory, use `TOUCH.X` and `TOUCH.Y` functions instead.

Pixels shown represent the number of fantasy pixels that is visible by the user according to their device screen ratio. Use the practical `SHOWN.W` and `SHOWN.H` functions.

Pixels outside the safe zone represent the number of fantasy pixels that are visible but SHOULD be considered unsafe for touch input as they are outside the safe area. Use the easy memorable `SAFE.L`, `SAFE.T`, `SAFE.R` and `SAFE.B` functions.

#### Other I/O status bits

| bits | purpose                                   |
| ----:| ----------------------------------------- |
|    0 | Pause currently active                    |
|    1 | Fantasy screen currently touched          |
|    2 | Device virtual keyboard currently visible |

#### DMA registers

|  address | size    | purpose                |
| -------:| ------- | ----------------------- |
|   $FFA0 | 2 Bytes | Source address          |
|   $FFA2 | 2 Bytes | Number of bytes to copy |
|   $FFA4 | 2 Bytes | Destination address     |

### Cycles

The fantasy hardware simulates CPU cycles. A cycle a fixed time duration that can be used to compute things. In Retro Game Creator each instructions cost a number of cycles that follow a set of rules.

- 1 cycle per instruction,
- 1 cycle for reading the value of a variable,
- 1 cycle per operator in expression evaluation,
- 1 cycle per array item creation,
- 1 cycle per function execution,
- 1 cycle per literals evaluation,
- 1 cycle per label read as value,
- 1 cycle per command execution,
- 8 cycles for copying sound data for each sound play,
- 1 cycle per character for string creation, modification, concatenation,
- 1 cycle per byte for memory copy or modification (except `DMA COPY`,
- almost 1/32 cycle per byte copied using `DMA COPY`,
- 2 cycles per cells modification including text.

Limit of cycles a program can execute:
- 52668 per frames

During interrupt, additional limits are also applied:
- 3420 for VBL interrupt
- 204 per line for RASTER interrupt
- 51 per particle for PARTICLE interrupt
- 102 per emitter for EMITTER interrupt

### Number limits

Retro Game Creator internally store numbers as 🌐[floating point](https://en.wikipedia.org/wiki/IEEE_754) on 32 bits.

This enough to store integers values (-16777216..16777216) with a 1 to 1 precision.

With lower or bigger numbers, precision will decrease, making those number equal.

    if 16777216=16777217 then
        print "yes"
    else
        print "no"
    end if

With floating-point numbers, the maximum precision varies depending on the magnitude of the number.

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

## Index

**`ABS`**:
- [`absolute =ABS(number)`](#-absolute-abs-number-)

**`ADD`**:
- [`ADD variable, value`<br>`ADD variable, increment, min TO max`](#-add-variable-value-br-add-variable-increment-min-to-max-)

**`AND`**:
- [`AND`](#operators)

**`ASC`**:
- [`ascii7 =ASC(character)`](#-ascii7-asc-character-)

**`AT`**:
- [`AT`](#operators)

**`ATAN`**:
- [`arc =ATAN(x, y)`](#-arc-atan-x-y-)

**`ATTR`**:
- [`ATTR attributes`](#-attr-attributes-)

**`BG`**:
- [`BG layer`](#-bg-layer-)
- [`BG FILL x1, y1 TO x2, y2 CHAR character`](#-bg-fill-x1-y1-to-x2-y2-char-character-)
- [`BG TINT y1, y1 TO x2, y2 [PAL palette] [FLIP horizontal, vertical] [PRIO priority]`](#-bg-tint-y1-y1-to-x2-y2-pal-palette-flip-horizontal-vertical-prio-priority-)
- [`BG SCROLL x1, y1 to x2, y2 step x3, y3`](#-bg-scroll-x1-y1-to-x2-y2-step-x3-y3-)
- [`BG SOURCE address [,width, height]`](#-bg-source-address-width-height-)
- [`BG COPY x1, y1, width, height TO x2, y2`](#-bg-copy-x1-y1-width-height-to-x2-y2-)
- [`BG VIEW ON`<br>`BG VIEW OFF`](#-bg-view-on-br-bg-view-off-)

**`BIN`**:
- [`hexadecimal =HEX$(number, [digits])`<br>`binary =BIN$(number, [digits])`](#-hexadecimal-hex-number-digits-br-binary-bin-number-digits-)

**`CALL`**:
- [`CALL`](#-call-)
- [`ON VBL CALL procedure`](#-on-vbl-call-procedure-)
- [`ON RASTER CALL procedure`](#-on-raster-call-procedure-)

**`CEIL`**:
- [`ceil =CEIL(number)`](#-ceil-ceil-number-)

**`CELL`**:
- [`CELL x, y, [character]`](#-cell-x-y-character-)
- [`character =CELL.C(x, y)`](#-character-cell-c-x-y-)
- [`attributes =CELL.A(x, y)`](#-attributes-cell-a-x-y-)

**`CELL.A`**:
- [`attributes =CELL.A(x, y)`](#-attributes-cell-a-x-y-)

**`CELL.C`**:
- [`character =CELL.C(x, y)`](#-character-cell-c-x-y-)

**`CHAR`**:
- [`BG FILL x1, y1 TO x2, y2 CHAR character`](#-bg-fill-x1-y1-to-x2-y2-char-character-)

**`CHR`**:
- [`character =CHR$(ascii7)`](#-character-chr-ascii7-)

**`CLAMP`**:
- [`clamped =CLAMP(value, min, max)`](#-clamped-clamp-value-min-max-)

**`CLS`**:
- [`CLS`](#-cls-)
- [`CLS layer`](#-cls-layer-)

**`CLW`**:
- [`CLW`](#-clw-)

**`COLOR`**:
- [`available =COLOR(palette, color)`](#-available-color-palette-color-)

**`COMPAT`**:
- [`COMPAT`](#-compat-)

**`COPY`**:
- [`BG COPY x1, y1, width, height TO x2, y2`](#-bg-copy-x1-y1-width-height-to-x2-y2-)
- [`COPY source, count TO destination`](#-copy-source-count-to-destination-)
- [`DMA COPY [ROM]`](#-dma-copy-rom-)

**`COS`**:
- [`cosine =COS(number)`<br>`sine =SIN(number)`](#-cosine-cos-number-br-sine-sin-number-)

**`CURSOR.X`**:
- [`x =CURSOR.X`<br>`y =CURSOR.Y`](#-x-cursor-x-br-y-cursor-y-)

**`CURSOR.Y`**:
- [`x =CURSOR.X`<br>`y =CURSOR.Y`](#-x-cursor-x-br-y-cursor-y-)

**`DATA`**:
- [`DATA constant [,constant...]`](#-data-constant-constant-)

**`DEC`**:
- [`INC variable`<br>`DEC variable`](#-inc-variable-br-dec-variable-)

**`DIM`**:
- [`DIM`](#-dim-)
- [`DIM GLOBAL`](#-dim-global-)

**`DMA`**:
- [`DMA COPY [ROM]`](#-dma-copy-rom-)

**`DO`**:
- [`DO/LOOP/EXIT`](#-do-loop-exit-)

**`EASE`**:
- [`interpolation =EASE(function, mode, value)`](#-interpolation-ease-function-mode-value-)

**`ELSE`**:
- [`IF/THEN/ELSE IF/ELSE/END IF`<br>`IF/THEN`](#-if-then-else-if-else-end-if-br-if-then-)

**`EMITTER`**:
- [`EMITTER`](#operators)

**`END`**:
- [`END`](#-end-)
- [`IF/THEN/ELSE IF/ELSE/END IF`<br>`IF/THEN`](#-if-then-else-if-else-end-if-br-if-then-)
- [`SUB/END SUB/EXIT SUB`](#-sub-end-sub-exit-sub-)

**`ENVELOPE`**:
- [`ENVELOPE voice, [attack], [decay], [sustain], [release]`](#-envelope-voice-attack-decay-sustain-release-)

**`EXIT`**:
- [`DO/LOOP/EXIT`](#-do-loop-exit-)
- [`REPEAT/UNTIL/EXIT`](#-repeat-until-exit-)
- [`WHILE/WEND/EXIT`](#-while-wend-exit-)
- [`FOR/TO/STEP/NEXT/EXIT`](#-for-to-step-next-exit-)
- [`SUB/END SUB/EXIT SUB`](#-sub-end-sub-exit-sub-)

**`EXP`**:
- [`exponential =EXP(number)`](#-exponential-exp-number-)

**`FILE`**:
- [`comment$ =FILE$(file)`](#-comment-file-file-)

**`FILES`**:
- [`FILES`](#-files-)

**`FILL`**:
- [`BG FILL x1, y1 TO x2, y2 CHAR character`](#-bg-fill-x1-y1-to-x2-y2-char-character-)
- [`FILL address, count [,value]`](#-fill-address-count-value-)

**`FLIP`**:
- [`SPRITE sprite [PAL palette] [FLIP horizontal, vertical] [PRIO priority] [SIZE size]`](#-sprite-sprite-pal-palette-flip-horizontal-vertical-prio-priority-size-size-)
- [`FLIP [horizontal], [vertical]`](#-flip-horizontal-vertical-)
- [`TINT y, y [PAL palette] [FLIP horizontal, vertical] [PRIO priority]`](#-tint-y-y-pal-palette-flip-horizontal-vertical-prio-priority-)
- [`BG TINT y1, y1 TO x2, y2 [PAL palette] [FLIP horizontal, vertical] [PRIO priority]`](#-bg-tint-y1-y1-to-x2-y2-pal-palette-flip-horizontal-vertical-prio-priority-)

**`FLOOR`**:
- [`floor =FLOOR(number)`<br>`floo =INT(number)`](#-floor-floor-number-br-floo-int-number-)

**`FONT`**:
- [`FONT first`](#-font-first-)

**`FOR`**:
- [`FOR/TO/STEP/NEXT/EXIT`](#-for-to-step-next-exit-)

**`FSIZE`**:
- [`size =FSIZE(file)`](#-size-fsize-file-)

**`GLOBAL`**:
- [`GLOBAL`](#-global-)
- [`DIM GLOBAL`](#-dim-global-)

**`GOSUB`**:
- [`GOSUB/RETURN`](#-gosub-return-)
- [`ON GOTO`<br>`ON GOSUB`](#-on-goto-br-on-gosub-)

**`GOTO`**:
- [`GOTO`](#-goto-)
- [`ON GOTO`<br>`ON GOSUB`](#-on-goto-br-on-gosub-)

**`HAPTIC`**:
- [`HAPTIC pattern`](#-haptic-pattern-)

**`HEX`**:
- [`hexadecimal =HEX$(number, [digits])`<br>`binary =BIN$(number, [digits])`](#-hexadecimal-hex-number-digits-br-binary-bin-number-digits-)

**`HIT`**:
- [`collides =SPRITE HIT(sprite)`](#-collides-sprite-hit-sprite-)
- [`collides =SPRITE HIT(sprite, sprite1)`](#-collides-sprite-hit-sprite-sprite1-)
- [`collides =SPRITE.HIT(sprite, sprite1 to sprite2)`](#-collides-sprite-hit-sprite-sprite1-to-sprite2-)
- [`sprite =HIT`](#-sprite-hit-)

**`IF`**:
- [`IF/THEN/ELSE IF/ELSE/END IF`<br>`IF/THEN`](#-if-then-else-if-else-end-if-br-if-then-)

**`INC`**:
- [`INC variable`<br>`DEC variable`](#-inc-variable-br-dec-variable-)

**`INKEY`**:
- [`pressed$ =INKEY$`](#-pressed-inkey-)

**`INPUT`**:
- [`INPUT [prompt;] variable`<br>`INPUT [prompt;] variable$`](#-input-prompt-variable-br-input-prompt-variable-)

**`INSTR`**:
- [`found =INSTR(text, occurence [,position])`](#-found-instr-text-occurence-position-)

**`INT`**:
- [`floor =FLOOR(number)`<br>`floo =INT(number)`](#-floor-floor-number-br-floo-int-number-)

**`KEYBOARD`**:
- [`KEYBOARD ON`<br>`KEYBOARD OFF`](#-keyboard-on-br-keyboard-off-)
- [`height =KEYBOARD`](#-height-keyboard-)

**`LEFT`**:
- [`leading$ =LEFT$(text, length)`<br>`trailing$ =RIGHT$(text, length)`](#-leading-left-text-length-br-trailing-right-text-length-)
- [`LEFT$(text, length) =replacement$`<br>`RIGHT$(text, length) =replacement$`](#-left-text-length-replacement-br-right-text-length-replacement-)

**`LEN`**:
- [`length =LEN(x, y)`](#-length-len-x-y-)
- [`length =LEN(text)`](#-length-len-text-)

**`LET`**:
- [`LET`](#operators)

**`LFO`**:
- [`LFO voice, [rate], [frequency], [volume], [width]`](#-lfo-voice-rate-frequency-volume-width-)
- [`LFO WAVE voice, [wave], [invert], [env], [trigger]`](#-lfo-wave-voice-wave-invert-env-trigger-)

**`LFO.A`**:
- [`LFO.A`](#operators)

**`LOAD`**:
- [`LOAD file, address [,limit [,offset]]`](#-load-file-address-limit-offset-)

**`LOCATE`**:
- [`LOCATE x, y`](#-locate-x-y-)

**`LOG`**:
- [`logarithm =LOG(number)`](#-logarithm-log-number-)

**`LOOP`**:
- [`DO/LOOP/EXIT`](#-do-loop-exit-)

**`MAX`**:
- [`minimal =MIN(a, b)`<br>`maximal =MAX(a, b)`](#-minimal-min-a-b-br-maximal-max-a-b-)

**`MCELL`**:
- [`MCELL x, y, character`](#-mcell-x-y-character-)
- [`character =MCELL.C(x, y)`](#-character-mcell-c-x-y-)
- [`attributes =MCELL.A(x, y)`](#-attributes-mcell-a-x-y-)

**`MCELL.A`**:
- [`attributes =MCELL.A(x, y)`](#-attributes-mcell-a-x-y-)

**`MCELL.C`**:
- [`character =MCELL.C(x, y)`](#-character-mcell-c-x-y-)

**`MESSAGE`**:
- [`MESSAGE text`](#-message-text-)

**`MID`**:
- [`extract$ =MID(text, position, length)`](#-extract-mid-text-position-length-)
- [`MID$(text, position, length) =replacement$`](#-mid-text-position-length-replacement-)

**`MIN`**:
- [`minimal =MIN(a, b)`<br>`maximal =MAX(a, b)`](#-minimal-min-a-b-br-maximal-max-a-b-)

**`MOD`**:
- [`MOD`](#operators)

**`MUSIC`**:
- [`MUSIC [pattern]`](#-music-pattern-)
- [`value =MUSIC(what)`](#-value-music-what-)

**`NEXT`**:
- [`FOR/TO/STEP/NEXT/EXIT`](#-for-to-step-next-exit-)

**`NOT`**:
- [`NOT`](#operators)

**`NUMBER`**:
- [`NUMBER x, y, number, count`](#-number-x-y-number-count-)

**`OFF`**:
- [`SPRITE OFF`](#-sprite-off-)
- [`SPRITE OFF sprite`](#-sprite-off-sprite-)
- [`SPRITE OFF sprite1 TO sprite2`](#-sprite-off-sprite1-to-sprite2-)
- [`KEYBOARD ON`<br>`KEYBOARD OFF`](#-keyboard-on-br-keyboard-off-)
- [`SPRITE VIEW ON`<br>`SPRITE VIEW OFF`](#-sprite-view-on-br-sprite-view-off-)
- [`BG VIEW ON`<br>`BG VIEW OFF`](#-bg-view-on-br-bg-view-off-)
- [`ON VBL OFF`](#-on-vbl-off-)
- [`ON RASTER OFF`](#-on-raster-off-)

**`ON`**:
- [`ON GOTO`<br>`ON GOSUB`](#-on-goto-br-on-gosub-)
- [`ON RESTORE`](#-on-restore-)
- [`KEYBOARD ON`<br>`KEYBOARD OFF`](#-keyboard-on-br-keyboard-off-)
- [`SPRITE VIEW ON`<br>`SPRITE VIEW OFF`](#-sprite-view-on-br-sprite-view-off-)
- [`BG VIEW ON`<br>`BG VIEW OFF`](#-bg-view-on-br-bg-view-off-)
- [`ON VBL CALL procedure`](#-on-vbl-call-procedure-)
- [`ON VBL OFF`](#-on-vbl-off-)
- [`ON RASTER CALL procedure`](#-on-raster-call-procedure-)
- [`ON RASTER OFF`](#-on-raster-off-)

**`OR`**:
- [`OR`](#operators)

**`PAL`**:
- [`SPRITE sprite [PAL palette] [FLIP horizontal, vertical] [PRIO priority] [SIZE size]`](#-sprite-sprite-pal-palette-flip-horizontal-vertical-prio-priority-size-size-)
- [`PAL palette`](#-pal-palette-)
- [`TINT y, y [PAL palette] [FLIP horizontal, vertical] [PRIO priority]`](#-tint-y-y-pal-palette-flip-horizontal-vertical-prio-priority-)
- [`BG TINT y1, y1 TO x2, y2 [PAL palette] [FLIP horizontal, vertical] [PRIO priority]`](#-bg-tint-y1-y1-to-x2-y2-pal-palette-flip-horizontal-vertical-prio-priority-)

**`PALETTE`**:
- [`PALETTE palette, [c0], [c1], [c2], [c3]`](#-palette-palette-c0-c1-c2-c3-)

**`PARTICLE`**:
- [`PARTICLE`](#operators)

**`PAUSE`**:
- [`PAUSE`](#-pause-)

**`PEEK`**:
- [`value =PEEK(address)`](#-value-peek-address-)

**`PEEKL`**:
- [`value =PEEKL(address)`](#-value-peekl-address-)

**`PEEKW`**:
- [`value =PEEKW(address)`](#-value-peekw-address-)

**`PI`**:
- [`pi =PI`](#-pi-pi-)

**`PLAY`**:
- [`PLAY voice, pitch [,length] [SOUND sound]`](#-play-voice-pitch-length-sound-sound-)

**`POKE`**:
- [`POKE address, value`](#-poke-address-value-)

**`POKEL`**:
- [`POKEL address, value`](#-pokel-address-value-)

**`POKEW`**:
- [`POKEW address, value`](#-pokew-address-value-)

**`PRINT`**:
- [`PRINT expression...`](#-print-expression-)
- [`PRINT`](#-print-)

**`PRIO`**:
- [`SPRITE sprite [PAL palette] [FLIP horizontal, vertical] [PRIO priority] [SIZE size]`](#-sprite-sprite-pal-palette-flip-horizontal-vertical-prio-priority-size-size-)
- [`PRIO priority`](#-prio-priority-)
- [`TINT y, y [PAL palette] [FLIP horizontal, vertical] [PRIO priority]`](#-tint-y-y-pal-palette-flip-horizontal-vertical-prio-priority-)
- [`BG TINT y1, y1 TO x2, y2 [PAL palette] [FLIP horizontal, vertical] [PRIO priority]`](#-bg-tint-y1-y1-to-x2-y2-pal-palette-flip-horizontal-vertical-prio-priority-)

**`RANDOMIZE`**:
- [`RANDOMIZE seed`](#-randomize-seed-)
- [`random =RND address`<br>`random =RND(max, address)`<br>`RANDOMIZE seed [,address]`](#-random-rnd-address-br-random-rnd-max-address-br-randomize-seed-address-)

**`RASTER`**:
- [`ON RASTER CALL procedure`](#-on-raster-call-procedure-)
- [`line =RASTER`](#-line-raster-)
- [`ON RASTER OFF`](#-on-raster-off-)

**`READ`**:
- [`READ variable [,variable...]`](#-read-variable-variable-)

**`REPEAT`**:
- [`REPEAT/UNTIL/EXIT`](#-repeat-until-exit-)

**`RESTORE`**:
- [`RESTORE [label]`](#-restore-label-)
- [`ON RESTORE`](#-on-restore-)

**`RETURN`**:
- [`GOSUB/RETURN`](#-gosub-return-)

**`RIGHT`**:
- [`leading$ =LEFT$(text, length)`<br>`trailing$ =RIGHT$(text, length)`](#-leading-left-text-length-br-trailing-right-text-length-)
- [`LEFT$(text, length) =replacement$`<br>`RIGHT$(text, length) =replacement$`](#-left-text-length-replacement-br-right-text-length-replacement-)

**`RND`**:
- [`random =RND`](#-random-rnd-)
- [`random =RND(max)`](#-random-rnd-max-)
- [`random =RND address`<br>`random =RND(max, address)`<br>`RANDOMIZE seed [,address]`](#-random-rnd-address-br-random-rnd-max-address-br-randomize-seed-address-)

**`ROL`**:
- [`ROL address, places`<br>`ROR address, places`](#-rol-address-places-br-ror-address-places-)

**`ROM`**:
- [`address =ROM(file)`](#-address-rom-file-)
- [`DMA COPY [ROM]`](#-dma-copy-rom-)

**`ROR`**:
- [`ROL address, places`<br>`ROR address, places`](#-rol-address-places-br-ror-address-places-)

**`SAFE.B`**:
- [`left =SAFE.L`<br>`top =SAFE.T`<br>`right =SAFE.R`<br>`bottom =SAFE.B`](#-left-safe-l-br-top-safe-t-br-right-safe-r-br-bottom-safe-b-)

**`SAFE.L`**:
- [`left =SAFE.L`<br>`top =SAFE.T`<br>`right =SAFE.R`<br>`bottom =SAFE.B`](#-left-safe-l-br-top-safe-t-br-right-safe-r-br-bottom-safe-b-)

**`SAFE.R`**:
- [`left =SAFE.L`<br>`top =SAFE.T`<br>`right =SAFE.R`<br>`bottom =SAFE.B`](#-left-safe-l-br-top-safe-t-br-right-safe-r-br-bottom-safe-b-)

**`SAFE.T`**:
- [`left =SAFE.L`<br>`top =SAFE.T`<br>`right =SAFE.R`<br>`bottom =SAFE.B`](#-left-safe-l-br-top-safe-t-br-right-safe-r-br-bottom-safe-b-)

**`SAVE`**:
- [`SAVE file, comment$,    address, size`](#-save-file-comment-address-size-)

**`SCROLL`**:
- [`SCROLL layer, [x], [y]`](#-scroll-layer-x-y-)
- [`x =SCROLL.X(layer)`<br>`y =SCROLL.Y(layer)`](#-x-scroll-x-layer-br-y-scroll-y-layer-)
- [`BG SCROLL x1, y1 to x2, y2 step x3, y3`](#-bg-scroll-x1-y1-to-x2-y2-step-x3-y3-)

**`SCROLL.X`**:
- [`x =SCROLL.X(layer)`<br>`y =SCROLL.Y(layer)`](#-x-scroll-x-layer-br-y-scroll-y-layer-)

**`SCROLL.Y`**:
- [`x =SCROLL.X(layer)`<br>`y =SCROLL.Y(layer)`](#-x-scroll-x-layer-br-y-scroll-y-layer-)

**`SGN`**:
- [`sign =SGN(number)`](#-sign-sgn-number-)

**`SHOWN.H`**:
- [`width =SHOWN.W`<br>`height =SHOWN.H`](#-width-shown-w-br-height-shown-h-)

**`SHOWN.W`**:
- [`width =SHOWN.W`<br>`height =SHOWN.H`](#-width-shown-w-br-height-shown-h-)

**`SIN`**:
- [`cosine =COS(number)`<br>`sine =SIN(number)`](#-cosine-cos-number-br-sine-sin-number-)

**`SIZE`**:
- [`SPRITE sprite [PAL palette] [FLIP horizontal, vertical] [PRIO priority] [SIZE size]`](#-sprite-sprite-pal-palette-flip-horizontal-vertical-prio-priority-size-size-)
- [`size =SIZE(file)`](#-size-size-file-)

**`SKIP`**:
- [`SKIP number`](#-skip-number-)

**`SOUND`**:
- [`PLAY voice, pitch [,length] [SOUND sound]`](#-play-voice-pitch-length-sound-sound-)
- [`SOUND voice, [wave], [width], [length]`](#-sound-voice-wave-width-length-)
- [`SOUND SOURCE [address]`](#-sound-source-address-)

**`SOURCE`**:
- [`BG SOURCE address [,width, height]`](#-bg-source-address-width-height-)
- [`SOUND SOURCE [address]`](#-sound-source-address-)

**`SPRITE`**:
- [`SPRITE sprite, [x], [y], [character]`](#-sprite-sprite-x-y-character-)
- [`SPRITE sprite [PAL palette] [FLIP horizontal, vertical] [PRIO priority] [SIZE size]`](#-sprite-sprite-pal-palette-flip-horizontal-vertical-prio-priority-size-size-)
- [`x =SPRITE.X(sprite)`<br>`y =SPRITE.Y(sprite)`](#-x-sprite-x-sprite-br-y-sprite-y-sprite-)
- [`character =SPRITE.C(sprite)`](#-character-sprite-c-sprite-)
- [`SPRITE.A sprite, attributes`](#-sprite-a-sprite-attributes-)
- [`attributes =SPRITE.A(sprite)`](#-attributes-sprite-a-sprite-)
- [`SPRITE OFF`](#-sprite-off-)
- [`SPRITE OFF sprite`](#-sprite-off-sprite-)
- [`SPRITE OFF sprite1 TO sprite2`](#-sprite-off-sprite1-to-sprite2-)
- [`collides =SPRITE HIT(sprite)`](#-collides-sprite-hit-sprite-)
- [`collides =SPRITE HIT(sprite, sprite1)`](#-collides-sprite-hit-sprite-sprite1-)
- [`collides =SPRITE.HIT(sprite, sprite1 to sprite2)`](#-collides-sprite-hit-sprite-sprite1-to-sprite2-)
- [`SPRITE VIEW ON`<br>`SPRITE VIEW OFF`](#-sprite-view-on-br-sprite-view-off-)

**`SPRITE.A`**:
- [`SPRITE.A sprite, attributes`](#-sprite-a-sprite-attributes-)
- [`attributes =SPRITE.A(sprite)`](#-attributes-sprite-a-sprite-)

**`SPRITE.C`**:
- [`character =SPRITE.C(sprite)`](#-character-sprite-c-sprite-)

**`SPRITE.X`**:
- [`x =SPRITE.X(sprite)`<br>`y =SPRITE.Y(sprite)`](#-x-sprite-x-sprite-br-y-sprite-y-sprite-)

**`SPRITE.Y`**:
- [`x =SPRITE.X(sprite)`<br>`y =SPRITE.Y(sprite)`](#-x-sprite-x-sprite-br-y-sprite-y-sprite-)

**`SQR`**:
- [`square =SQR(number)`](#-square-sqr-number-)

**`STEP`**:
- [`FOR/TO/STEP/NEXT/EXIT`](#-for-to-step-next-exit-)

**`STOP`**:
- [`STOP`<br>`STOP [voice]`](#-stop-br-stop-voice-)

**`STR`**:
- [`text =STR$(number)`](#-text-str-number-)

**`SUB`**:
- [`SUB/END SUB/EXIT SUB`](#-sub-end-sub-exit-sub-)

**`SWAP`**:
- [`SWAP a, b`](#-swap-a-b-)

**`SYSTEM`**:
- [`SYSTEM setting, value`](#-system-setting-value-)

**`TAN`**:
- [`TAN`](#operators)

**`TAP`**:
- [`touched =TAP`](#-touched-tap-)
- [`WAIT TAP`](#-wait-tap-)

**`TEXT`**:
- [`TEXT x, y, text$`](#-text-x-y-text-)

**`THEN`**:
- [`IF/THEN/ELSE IF/ELSE/END IF`<br>`IF/THEN`](#-if-then-else-if-else-end-if-br-if-then-)

**`TIMER`**:
- [`frames =TIMER`](#-frames-timer-)

**`TINT`**:
- [`TINT y, y [PAL palette] [FLIP horizontal, vertical] [PRIO priority]`](#-tint-y-y-pal-palette-flip-horizontal-vertical-prio-priority-)
- [`BG TINT y1, y1 TO x2, y2 [PAL palette] [FLIP horizontal, vertical] [PRIO priority]`](#-bg-tint-y1-y1-to-x2-y2-pal-palette-flip-horizontal-vertical-prio-priority-)

**`TO`**:
- [`FOR/TO/STEP/NEXT/EXIT`](#-for-to-step-next-exit-)
- [`SPRITE OFF sprite1 TO sprite2`](#-sprite-off-sprite1-to-sprite2-)
- [`BG FILL x1, y1 TO x2, y2 CHAR character`](#-bg-fill-x1-y1-to-x2-y2-char-character-)
- [`BG TINT y1, y1 TO x2, y2 [PAL palette] [FLIP horizontal, vertical] [PRIO priority]`](#-bg-tint-y1-y1-to-x2-y2-pal-palette-flip-horizontal-vertical-prio-priority-)
- [`BG COPY x1, y1, width, height TO x2, y2`](#-bg-copy-x1-y1-width-height-to-x2-y2-)
- [`ADD variable, value`<br>`ADD variable, increment, min TO max`](#-add-variable-value-br-add-variable-increment-min-to-max-)
- [`COPY source, count TO destination`](#-copy-source-count-to-destination-)

**`TOUCH`**:
- [`touched =TOUCH`](#-touched-touch-)
- [`x =TOUCH.X`<br>`y =TOUCH.Y`](#-x-touch-x-br-y-touch-y-)

**`TOUCH.X`**:
- [`x =TOUCH.X`<br>`y =TOUCH.Y`](#-x-touch-x-br-y-touch-y-)

**`TOUCH.Y`**:
- [`x =TOUCH.X`<br>`y =TOUCH.Y`](#-x-touch-x-br-y-touch-y-)

**`TRACE`**:
- [`TRACE expression [,expression...]`](#-trace-expression-expression-)

**`TRACK`**:
- [`TRACK track, voice`](#-track-track-voice-)

**`UBOUND`**:
- [`=UBOUND`](#-ubound-)

**`UNTIL`**:
- [`REPEAT/UNTIL/EXIT`](#-repeat-until-exit-)

**`VAL`**:
- [`number =VAL(text)`](#-number-val-text-)

**`VBL`**:
- [`WAIT VBL`<br>`WAIT frame`](#-wait-vbl-br-wait-frame-)
- [`ON VBL CALL procedure`](#-on-vbl-call-procedure-)
- [`ON VBL OFF`](#-on-vbl-off-)

**`VIEW`**:
- [`SPRITE VIEW ON`<br>`SPRITE VIEW OFF`](#-sprite-view-on-br-sprite-view-off-)
- [`BG VIEW ON`<br>`BG VIEW OFF`](#-bg-view-on-br-bg-view-off-)

**`VOLUME`**:
- [`VOLUME voice, [volume], [mix]`](#-volume-voice-volume-mix-)

**`WAIT`**:
- [`WAIT TAP`](#-wait-tap-)
- [`WAIT VBL`<br>`WAIT frame`](#-wait-vbl-br-wait-frame-)

**`WAVE`**:
- [`LFO WAVE voice, [wave], [invert], [env], [trigger]`](#-lfo-wave-voice-wave-invert-env-trigger-)

**`WEND`**:
- [`WHILE/WEND/EXIT`](#-while-wend-exit-)

**`WHILE`**:
- [`WHILE/WEND/EXIT`](#-while-wend-exit-)

**`WINDOW`**:
- [`WINDOW x, y, width, height, layer`](#-window-x-y-width-height-layer-)

**`XOR`**:
- [`XOR`](#operators)
