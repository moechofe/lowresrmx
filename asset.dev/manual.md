# Manual

> This is a fork of the excellent fantasy console Lowres NX by Timo "Inutilis" Kloss.<br>
> This is a heavily modified version and way more complicated to use, I recommend you to stay on the original app:
> [LowRes NX Coder on App Store](https://apps.apple.com/app/lowres-nx-coder/id1318884577).

<!-- FIXME: spell checked start -->

Create your games and play them on the go using an iPhone or iPad. LowResRMX is a virtual game console and game development environment that includes all the tools you need to start and finish a project.

It **will** includes a project manager, a code editor, and various tools to edit game assets: sprites, palettes, backgrounds, instruments, and music.

> **The documentation and included tools are still in development.**

## Difference

The goal 🥅 of the app is to provide a development environment that I can use during my commutes on the subway, and later, to be able to publish a real app on the stores.

The original LowRes NX, despite being an excellent development environment, lacks some capabilities, in my opinion.

#### Background

4 background layers instead of 2, with 64x64 cells instead of 32x32.

---

Support for large 16x16 pixel cells has been removed.

Removed 🗑️ syntaxes:

`CELL SIZE bg,size`

---

Alternative syntax for the `FLIP` command:

`FLIP x,y`

Where `x` and `y` are both optional and can accept any number. If the number is different from zero, the flag will be set.

---

New settings with the `SYSTEM` command to render a layer at double size:

`SYSTEM setting,double`
<br>where `setting` ranges from `5` to `8`.

| setting | layer |
| ------- | ----- |
|       5 |     0 |
|       6 |     1 |
|       7 |     2 |
|       8 |     3 |

---

Internal scroll offset not capped to 512 any more, but 0xFFFF.

#### Screen

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

#### Colors

The 64 available colors do not follow the original EGA style of LowRes NX. Instead, they use the [FAMICUBE palette created by Arne](https://lospec.com/palette-list/famicube).

> This makes using the color chooser from the original "Gfx Designer" in LowRes NX a bit more challenging.

---

New settings with the `SYSTEM` command to make the color `0` for a layer opaque instead of transparent:

`SYSTEM setting,opacity`
<br>where `setting` ranges from `1` to `4`.

| setting | layer |
| ------- | ----- |
|       1 |     0 |
|       2 |     1 |
|       3 |     2 |
|       4 |     3 |

#### Sprites

170 sprites instead of 64.<br>
Sprite number range from `0` to `169`.

Positions support sub-pixels with `1/16` pixel precision.

e.g.:

    SPRITE 123,SPRITE.X(123)+0.25,

#### Input/Output

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

`=KEYBOARD`

Return `-1` if the keyboard is visible.

---

New `HAPTIC h` command to trigger haptic feedback on the device.

#### Control flow

New program control flow:

`ON value GOTO label,..`<br>
`ON value GOSUB label,..`<br>
`ON value RESTORE label,..`

These commands will `GOTO`, `GOSUB` or `RESTORE` to one of the listed `label`s based on the `value`.

e.g.:

	ON I GOTO ZERO,ONE,TWO

#### Data

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

#### Math

New function to return the ceiling value of a numerical value:

`=CEIL(value)`

---

New function to return the flooring value of a numerical value:

`=FLOOR(value)`

> This is different than `=INT(value)`

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

#### Text

New command to expose the existing overlay message API:

`MESSAGE text`

---

`PRINT` command can now word wrap before breaking word.

If there is not enough space to print a word inside the window, a new line is inserted to avoid breaking the word in two.

#### Other

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

#### Memory

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

#### Particles

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

## How does it works

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
- X and Y position,
- a size of 8x8, 16x16, 24x24 or 32x32 pixels,
- a character number for the top left one,
- a palette,
- horizontal and vertical flip,
- and priority to show the sprite on top of bg cells with priorities.

The 170 sprites are drawn in order from 170th (bottom) to 1st (top).
The sprite numbers range start at zero `0`,so the last one is `169`.

Sprites are groups of characters ranging from 1x1 to 4x4. The difference with bg is that you only choose the top-left character. The other character numbers are dependent on the top-left one.

A sprite can be freely placed anywhere on the screen with 1/16 sub-pixel precision. This means you can add 0.5 to its position even if there will be no change visually.

<!--FIXME: spell checked END -->

## Program Language

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

1. An identifier is a word defined by the and they are used to declare variables, labels or procedures (more on those terms later), e.g.: `hero`, `enemies`, `HP`, `score`, `spawn_monsters`, `update_screen`, `collect_coins`, `level12`…

2. An identifier can also be a built-in function name provided by LowResNX as part of the [API](#api-instructions).

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

### Literals

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

### Global scope

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

> They have to be declare in the main program before any procedure that use it is called.

The syntaxe to declare a global array is a bit different:

    dim global enemies

TODO: link GLOBAL

### Comments

Comments are piece of text that is not executed. They are usefull for taking notes of what a particular piece of code is doing.

    'get touch position in cells coordinates
    tx=touch.x\8

> Avoid Captain Obvious comment

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

Expression are used to compute values, be assigned to identifier or passed to function as arguments. Expression can be one of:

- A [literal value](#literals):

    ```
    123
    "gabu"
    ```

- A [variable identifier](#variables):

    ```
    myVar$
    myArray(0)
    ```

- A [built-in function call](#built-in-functions):

    ```
    cos(1.314)
    left$(name$,8)
    ```

- An [unary, binary or group operator](#operators):

    ```
    42+24
    $FF00+A*2
    count>=0
    not dead
    (2+3)*4
    ```

- A [label identifier](#labels):

    ```
    mylabel
    ```

### Operators

Operator are used inside [expression](#expression) to compute or alter it's value.

**Arithmetic operators.**

Operate on two numeric values to produce a new one.

| symbol | example   | purpose          |
| ------ | --------- | ---------------- |
| `-`    | `-42`     | negation         |
| `^`    | `x^3`     | exponentation    |
| `*`    | `2*y`     | multiplication   |
| `/`    | `x/2`     | division         |
| `\`    | `x\2`     | integer division |
| `mod`  | `x mod 2` | modulo           |
| `+`    | `c+2`     | addition         |
| `-`    | `100-d`   | subtraction      |

The priority of execution respect the mathematical rules.

**Group operator.**

Rounded parentheses `()` are used to counter the operator priority.

TODO: difference between `/` and `\`

**Comparison operator.**

Used to compair two numeric or string values and produce `-1` if the test succeed or `0` otherwise. Generally used as expression in [`IF` flow control](#flow-control).

| symbol | example  | purpose
| ------ | -------- | -------
| `=`    | `a=10`   | equal
| `<>`   | `a<>100` | not equal
| `>`    | `b>c`    | greater
| `<`    | `5<x`    | less
| `>=`   | `X>=20`  | greater or eqal
| `<=`   | `X<=30`  | less or eqal

**Bitwise operator.**

Used to manipulate each bit of numeric values.

| symbol | example                        | purpose                                                     |
| ------ | ------------------------------ | ----------------------------------------------------------- |
| `not`  | `not (x=15)`<br>`not 0`        | Bits that are 0 become 1,<br>and those that are 1 become 0. |
| `and`  | `a=1 and b=12`<br>`170 and 15` | If both bits are 1, the<br>result is 1, 0 otherwise.        |
| `or`   | `x=10 or y=0`<br>`128 or 2`    | The result is 0 if both<br>bits are 0, 1 otherwise.         |
| `xor`  | `a xor b`                      | The resulst is 1 if only<br>one bit is 1, 0 otherwise.     |

**Concatenation operator.**

Copy a string at the end of another string.

| symbol | example    | purpose                           |
| ------ | ---------- | --------------------------------- |
| `+`    | `"ab"+"c"` | Concatenate two strings into one. |

### Commands

LowResNX provide a bunch of built-in function to manipulate the fantasy device hardware such as the graphic or sound. You'll find them by consulting the [list of API instructions](#api-instructions).

To execute a command, use it's identifier followed by a list of coma-separated arguments or other reserved keywords. The exact syntax depends on the command.

    sprite off 0 to 169
    sprite 0,40,60,1

### Functions

LowResNX provide a bunch of built-in function to manipulate the fantasy device hardware such as the graphic or sound. You'll find them by consulting the [list of API instructions](#api-instructions).

To execute a function, use it's identifier followed by a list of coma-separated arguments surrounded by rounded parenthesis.

    print mid$("test",3,1) :'print s

A function will always return something and MUST be used as [rvalue](#lvalue-and-rvalue) inside an [expression](#expression).

### Lvalue and Rvalue

The equal `=` sign is used both as variable assignement and comparison operator, and It can lead to somo confusion. Here is a tip to help you:

[Expression](#expression) can be one lvalue or rvalue.

An lvalue appear at the left side of an assignation, and rvalue appear at the right side of an assignation.

| expression    | type of value    |
| ---------------- | ---------------- |
| [variable](#variables) | lvalue or rvalue |
| [literal](#literals)   | rvalue           |
| function call | rvalue           |
| any operator  | rvalue           |
| label         | rvalue           |

## BASIC instructions

TODO: continue

### Control flow

`END`

**Stop the execution of the program.**

It has the same effect as the execution reach the end of the program.

---

`IF/THEN/ELSE IF/ELSE/END IF`

**Conditionnaly execute instructions.**

Syntax:

    IF expression THEN
        instruction...
    [ELSE IF expression THEN
        instruction...]
    [ELSE IF...]
    [ELSE
        instruction]
    END IF

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

Syntax:

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

`DO/LOOP/EXIT`

**Infinite loop**

Syntax:

    DO
        instruction...
        [EXIT]
    LOOP

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

`EXIT` instruction will exit one level of `DO/LOOP`.

---

`REPEAT/UNTIL/EXIT`

**Condition-controlled loop***

Syntax:

    REPEAT
        instruction...
        [EXIT]
    UNTIL expression

Repeat the list of `instruction...` until the `expression` ahead became true.
An [expression](#expression) is evaluated as `true` if the result of the expression is different that `0` zero.

`EXIT` instruction will exit one level of `REPEAT/UNTIL`.

> It different from `WHILE/WEND` because `expression` is evaluated after the list of `instruction...` is executed.

    stop_now=-1
    repeat
        print "do it anyway"
    until stop_now

---

`WHILE/WEND/EXIT`

**Condition-controlled loop**

Syntax:

    WHILE expression
        instruction...
        [EXIT]
    WEND

Repeat the list of `instruction...` while the `expression` above is true.
An [expression](#expression) is evaluated as `true` if the result of the expression is different that `0` zero.

`EXIT` instruction will exit one level of `WHILE/WEND`.

> It different from `REPEAT/UNTIL` because `expression` is evaluated before the list `instruction...` is executed.

    count=0
    while count
    print "not executed"
    wend

---

`FOR/TO/NEXT/EXIT`

**Count-controlled loop**

Syntax:

    FOR identifier=begin TO ended [STEP incr]
        instruction...
        [EXIT]
    NEXT identifier

Repeat the list of `instruction...` while varying the value of the `identifier` starting at `begin` until it reach `ended` included.
The increment can be changed using `incr`, allowing to iterates in reverse.

Real example that print numbers from 1 to 9 in ascending and descending order.

    for i=1 to 9
    print i;
    next i
    print
    for i=9 to 1 step -1
    print i;
    next i

`EXIT` instruction will exit one level of `FOR/TO/NEXT`.

---

`GOTO`

**Jump to instruction**

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

**Jump to sub-routine**

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

    ON value GOTO label0[,label1...]

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

`ON GOSUB`

**Jump table to a sub-routine.**

Similar to `ON GOTO` but expect to encounter a `RETURN`.

---

`SUB/END SUB/EXIT SUB`

**Defining a proceduce.**

TODO: check for remaining function, this term should be reserve for built-in function only.

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

**Exiting early.**

Syntax:

    EXIT SUB

TODO: wait for next do loop repeat until while wend exit sub end sub call exit sub global dim

TODO: GOTO inside SUB

TODO: playing with GOSUB and RETURN

### Data, ROM and files

LowResRMX provide two ways to storing data or assets inside a program.

1. Use the combinaison of `DATA` and `READ` to store readable number and string and access it when you need it.

    Real example that list the Straw Hat Pirates members:

    ```
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
    ```

    Internaly it will use a read pointer that iterates all constant values.

2. TODO: continue

---

`DATA/READ`

**Declare constant values.**

Syntax:

    DATA constant[,constant...]

A comma separated list of constant values (numbers or strings) that can accessed using the `READ` command.

**Read constant values.**

Syntax:

    READ identifier[,identifier...]

Read one or more constant values that was declared using `DATA` into variables.

---

`RESTORE/SKIP`

**Move the read pointer.**

Syntax:

    RESTORE [label]

Move the read pointer to a specified label. The declared constant values that appear below the label will be read next.

**Skip a number of constant value.**

Syntax:

    SKIP number

Allow to skip a `number` of constant values by moving the read pointer.

    data "failure"
    data "success"
    skip 1
    read word$
    print word$

---

`ON RESTORE`

**Move table for the read pointer.**

Similar to `ON GOTO` but move the read pointer instead of jumping.

### Variable related

**Scalar initialization**

Syntax:

    identifier=literal

Single number or string variable MUST be initialized before reading it.

    score=0
    name$="player 1"
    print name$;":",score

> Learn more about [identifiers](#identifiers), [literals](#literals) and [variables](#variables).

---

`GLOBAL`

**Scalar global declaration**

Syntax:

    DIM identifier...

Declare a list of number or string variables to be globaly accessible in all scopes.
`GLOBAL` keywords are illegal inside a `SUB/END SUB` subroutine body.

> Learn more about [global scope](#global-scope).

---

`DIM`

**Array declaration**

Syntax:

    DIM identifier(highest) [,identifier(highest)]...

Will declare one or more arrays with `highest`+1 number of elements.
Array of numbers or strings MUST be declared before reading or writing it.

    dim scores(1),names$(1)
    for i=0 to 1
        print names$(i);":",scores(i)
    next i

---

`DIM GLOBAL`

**Global array declaration**

Syntax:

    DIM GLOBAL identifier(highest) [,identifier(highest)]...

Similar to `DIM` but will declare the arrays globally accessible.
`GLOBAL` keywords are illegal inside a `SUB/END SUB` subroutine body.

> Learn more about [identifiers](#identifiers), [arrays](#arrays) and [global scope](#global-scope).

---

`=UBOUND`

**Array highest index**

Syntax:

    =UBOUND(identifier[,dimension])

Return the highest index of the array variable `identifier` at specified `dimension`.

## API instructions

LowResRMX provide a bunch of built-in [commands](#commands) and [functions](#functions) to communicates with the differents features provided by the fantasy console: input, graphics, sound and memory access.

### Sprite API

> [How sprits works](#sprites).

Sprite are limited in numbers and should be used to show moving objects above background layers. It not an oblication and cool things can be made by infringe this rules.

#### `SPRITE n,[x],[y],[c]`

Sets the position `x`,`y` in pixel coordinates and character `c` number of the sprite `n`.

`x`,`y` and `c` can be ommited to keep their current value.

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

#### `SPRITE n [PAL pal] [FLIP h,v] [PRIO pri] [SIZE s]`

Sets one or more attributes for the sprite `n`:

- `PAL pal` Change the palette to `pal`.
- `FLIP h,v` Flip the sprite on horizontal `h` and vertical `v` axis.
- `PRIO pri` Change the priority to `pri`.
- `SIZE s` Change the size, a.k.a.: the number of characters width and height.

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

TODO: link to sprite references

#### `=SPRITE.X(n)`<br>`=SPRITE.Y(n)`

Return the position of the sprite `n`. With `n` between 0 and 169 inclusive.

Example of a sprite smoothly follow the finger touch:

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

#### `=SPRITE.C(n)`

Return the first character number of the sprite `n`. With `n` between 0 and 169 inclusive.

#### `SPRITE.A n,a`

Sets all attributes at once for sprite `n`.

TODO: link to sprite references

#### `=SPRITE.A(n)`

Return the whole attributes flags of the sprite `n`. With `n` between 0 and 169 inclusive.

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

#### `SPRITE OFF n`

Hide the sprite `n`.

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

#### `SPRITE OFF n1 to n2`.

Hides all sprites from range `n1` to `n2` included.

#### `=SPRITE.HIT(n)`

Return if the sprite `n` collides with another sprite. Collision detection is not by checking overlapping pixels that are not transparent (not using the color `0`). With `n` between 0 and 169 inclusive.

Example of a ball that bounce on the wall and fall into holes using `=SPRITE.HIT(n)` detection:

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

#### `=SPRITE.HIT(n,n1)`

Return if the sprite `n` collides with the sprite `n1`. With `n` and `n1` between 0 and 169 inclusive.

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

#### `=SPRITE.HIT(n,n1 to n2)`

Return if the sprite `n` collides with any of the sprite from the range `n1` to `n2` included.

> See `=HIT` below for an example.

#### `=HIT`

Return the sprite number of the one that collides with the sprite `n` from any of the last `=SPRITE.HIT()` function call.

Example of a ball that bounce on the wall and destroy the obstable using `=HIT`:

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

> [How backgrounds works](#backgrounds).

The most basic way of drawing using the background API is to, first, [choose which cell's attribute](#draw-state) to use, then [paint the cells](#draw-cells) with a characters number.

Another way of doing is to [copy attributes and character](#background-copy) number from a ROM.

Aside to that, there are API for accessing the scrolling value of each layers in pixels, getting character numbers or attributes directly from layers or draw in memory instead of layers.

Text generally use background mechanics as well, there are dedicated commands for that too.

#### `CLS`

Clear all background layers with character zero `0`, resets the current window to the default and the layer scrolling values.

#### `CLS layer`

Only clear the background layer numbered `layer` with character zero `0`. Do not alter the scrolling value.

#### `SCROLL layer,x,y`

Set the scroll offset of the `layer` in pixels coordinates.

    text 10,10,"hello!"
    bg 1
    text 10,10,"hello!"
    scroll 0,0,-4

#### `=SCROLL.X(layer)`<br>`=SCROLL.Y(layer)`

Return the scroll offset of the `layer` in pixels coordinates.

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

Change the current layer to `layer` for further cells draw operations.

With `layer` a number from `0` to `3`.

    text 10,10,"\"
    bg 1
    text 10,10,"/"

#### `PAL p`

Change the current palette to `p` for further cells draw operations.

    print "hello",
    pal 1
    print "world"

#### `FLIP h,v`

Change the horizontal and vertical flip attributes to `h` and `v` respectively for further cells draw operations.

    flip 1,0
    print "dlrow olleh"

#### `PRIO p`

Change the priority to `p` for further cells draw operations.

    sprite 0,78,78,1
    prio 1
    text 10,10,"s"
    #2:main characters
    00000000000000000000000000000000
    003C7E6A7E7E3C000000183C00000000

#### `ATTR a`

Sets the palette, flip and priority attributes all at once using `a` for further cells draw operations.

TODO: link to background reference

#### `CELL x,y,[c]`

Sets the cell of the current background at `x`,`y` coordinates to the character number `c` with the [current attributes](#cells-attributes).

E.g. draw a face:

    cell 10,10,1
    #2:main characters
    00000000000000000000000000000000
    003C7E6A7E7E3C000000183C00000000

By omiting the `c` argument, the command will only alter the palette, flip and priority, but not change the character number.

E.g. draw an inverted blue r letter:

    cell 10,10,242
    flip 1,0
    pal 1
    cell 10,10,

> Use `BG`, `PAL`, `FLIP`, `PRIO` and `ATTR` to alter the [current attributes](#cells-attributes).

#### `=CELL.C(x,y)`

Return the character number of the current background at `x`,`y` cell coordinates.

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

#### `=CELL.A(x,y)`

Return the attributes of the current background at `x`,`y` cell coordinates

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

TODO: link to background reference

#### `BG FILL x1,y1 TO x2,y2 CHAR c`

Fills all cells from `x1`,`y1` coordinates to `x2`,`y2` with character `c` and the [current attributes](#cells-attributes).

    bg fill 1,1 to 5,5 char 1
    bg fill 2,2 to 4,4 char 2
    #2:main characters
    00000000000000000000000000000000
    FFFFFFFFFFFFFFFF0000000000000000
    0000000000000000FFFFFFFFFFFFFFFF

Similar to `CELL x,y,c` but with a rectangle of cells instead of just one cell.

#### `TINT y,y [PAL pal] [FLIP h,v] [PRIO p]`

Changes one or more [attributes of the cell](TODO: link to cells attributes reference) at `x`,`y` coordinates without changing the character number.

E.g.: Yellow exclamation mark

    print "hello!"
    tint 5,0 pal 7

#### `BG TINT y1,y1 TO x2,y2 [PAL pal] [FLIP h,v] [PRIO p]`

Changes one or more [attributes](TODO: link to cells attributes reference) for all cells from `x1`,`y1` coordinates to `x2`,`y2` without changing the character number.

Similar to `TINT y,y [PAL pal] [FLIP h,v] [PRIO p]` but with a rectangle of cells instead of just one.

#### `BG SOURCE a[,w,h]`

Set the address `a` in the cartridge to use as source for `BG COPY` operations.

When `w` and `h` are specified, they are used as indiquating the number of cells in width and height of the source.

If they are not specified, the [official background format data](#background-format-data) are used to store the width and the height.

> If not specified the default address is taken by internally executing `=ROM(3)`

TODO: example

#### `BG COPY x1,y1,w,h TO x2,y2`

Copy a rectangle of cell's attributes and character numbers from background source specified previously using `BG SOURCE a[,w,h]` at `x2`,`y2` coordinates of the current background layer.

The rectangle is defined using the top-left corner `x1`,`y2` plus the width `w` and height `h`.

#### `BG SCROLL x1,y1 to x2,y2 step x3,y3`

Move a rectangle of cell's attributes and character numbers of the current layer by `x3`,`y3` in cell coordinates.

The rectangle is defined using the top-left corner `x1`,`y2` and the bottom-right corner `x2`,`y2`.

> This feature is internally used to scroll text when it reach the bottom of the window. TODO: link to window.

#### `MCELL x,y,c`

Similar to `CELL x,y,c` but modify the source in memory instead of the current layer.

The source must point to writable memory, so it will not work with `BG SOURCE ROM(3)`.

#### `=MCELL.C(x,y)`

Similar to `=CELL.C(x,y)` but read the current layer instead of the source in memory.

#### `=MCELL.A(x,y)`

Similar to `=CELL.A(x,y)` but read the current layer instead of the source in memory.

#### `TEXT x,y,text$`

Will print `text$` on the current layer starting at `x`,`y` using the current background attributes.

  data "ga","bu","zo","meu"
  for i=0 to 3
    pal i
    read c$
    text 8+i*2,10,c$
  next i

#### `NUMBER x,y,number,count`

Similar to `TEXT x,y,text$` but dedicated to print `number` instead of text. Will always show `count` digits by prefixing with `0` characters.

    score=123
    number 10,10,score,6

#### `FONT first`

Sets the characters range used for `TEXT`, `NUMBER` and `PRINT` commands. Sets `first` to be the character number where the range begin, it correspond to the space according to the ASCII reference. TODO: link

The default value is 192, which points to where the default font is loaded at the beginning of the program. It might be overriden by `LOAD` operation.

### Text API

To print text on screen, LowResRMX actually use the [background API](#background-api).

`WINDOW x,y,w,h,layer`

Sets the current window where to draw text characters. `x`, `y` are the cell position where to start drawing on the background number `layer` while `w`, `h` are the width and height also in number of cells.

    window 4,8,shown.w\8-8,20,0
    print "Why do things simply when you can make them complicated?"

> By default, the window is sets inside the safe area delimited by the safe functions (`=SAFE.L`...)

#### `CLW`

Clears the window but replacing all the cells by the space character. It also reset the cursor position.

#### `LOCATE x,y`

Move the cursor location at `x`,`y` in cells coordinates inside the window.

    for i=1 to 9
    locate i,i
    print str$(i);
    next i
    end

#### `=CURSOR.X`<br>`=CURSOR.Y`

Return the cursor location `x` and `y` in cells coordinates inside the window.

#### `PRINT expression-list`<br>

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

#### `PRINT`<br>

Outputs a new line.

### Input API

LowResRMX do not support gamepad, user are forced to rely on touch only to implement a interface that allow to control their application.

User can also rely on the OS virtual keyboard to capture typed characters.

#### `=TOUCH`

Return `-1` if the device screen is currently touched.

Example that print a text only when device screen is touched:

    do
        cls
        if touch then text 4,4,"touched"
        wait vbl
    loop

#### `=TAP`

Similar to `=TOUCH` but return `true` for one frame only. Ideal to trigger event on your program.

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

#### `=TOUCH.X`<br>`=TOUCH.Y`

Returns the last pixel position touched. It returns a float numeric value, with a 1/16 pixel precisions, allowing for smooth pan or drag gesture.

Example that display the float position on x and y axis:

    do
        cls
        print touch.x
        print touch.y
        wait vbl
    loop

#### `KEYBOARD ON`<br>`KEYBOARD OFF`

Show or hide the device virtual keyboard. Because user can hide the keybord using a dedicated key, do not assume the keyboard is visible. Use `KEYBOARD ON` inside a loop or rely on `=KEYBOARD` to detect when the keyboard is shown or not.

#### `=KEYBOARD`

Returns `-1` when the device virtual keyboard is visible.

#### `=INKEY$`

Returns only once the last pressed key. The returned value is a string containing one character, one of the supported by LowResRMX. If no key was pressed or if the pressed key is not supported, it returns an empty string.

Example that output the pressed key:

    do
        keyboard on
        k$=inkey$
        if k$<>"" then
            print k$;
        end if
        wait vbl
    loop

TODO: link to reference

#### `=TIMER`

Returns the number of frames since LowResRMX was launched. The value wraps to 0 when 5184000 is reached, which is about 24 hours.

### Display API

#### `PALETTE palette,[c0],[c1],[c2],[c3]`

Sets the four colors on the height available `palette` (0..7). The color 0 of the palette 0 is generally used as backdrop color. (It can be change using the `SYSTEM` command.) `c0`, `c1`, `c2`, `c3` can accept a numeric value between 0 and 63, omit them to keep the current value. Consult the [64 colors reference](#64-colors) to choose the color you want.

#### `=COLOR(palette,color)`

Returns one of the [64 colors](#64-colors) associated to the pair `palette` (0..7), `color` (0..3).

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





TODO: VBL WAIT ON VBL




TODO: SYSTEM



TODO: continue

TODO: API

TODO: touch screen

TODO: about memory

TODO: about sound

TODO: about priority

TODO: about keywords

TODO: about numeric technical

TODO: how window not updated when on-resized

## References

### 64 Colors

<style>
.famicube{display:flex:flex-wrap:wrap}
.famicube div{font-family:monospace;padding:0.8em 1em;}
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

### Background format data

TODO: continue

| addr | purpose        |
| ---- | ---------------|
|  a+0 | always zero    |
|  a+1 | always zero    |
|  a+2 | width in cell  |
|  a+3 | height in cell |
|  a+4 | cell's data    |


### Registers

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

#### Attributes bits:

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

#### Color registers:

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

#### Video registers:

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

#### Display attributes:

| bits | purpose                    |
| ---- | -------------------------- |
| 0    | Sprites enabled            |
| 1    | Background layer 0 enabled |
| 2    | Background layer 1 enabled |
| 3    | Background layer 2 enabled |
| 4    | Background layer 3 enabled |

#### TODO: Audio registers:

| addr  | size     | purpose         |
| ----- | -------- | --------------- |
| $FF40 | 48 Bytes | Audio registers |

#### I/O registers:

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

#### Other I/O status bits:

| bits | purpose                                   |
| ---- | ----------------------------------------- |
| 0    | Pause currently active                    |
| 1    | Fantasy screen currently touched          |
| 2    | Device virtual keyboard currently visible |

#### DMA registers:

| addr  | size    | purpose                 |
| ----- | ------- | ----------------------- |
| $FFA0 | 2 Bytes | Source address          |
| $FFA2 | 2 Bytes | Number of bytes to copy |
| $FFA4 | 2 Bytes | Destination address     |
