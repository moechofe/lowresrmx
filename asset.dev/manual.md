# LowResRMX manual

> This is a fork of the excellent fantasy console Lowres NX by Timo "Inutilis" Kloss.<br>
> This is a heavily modified version and way more complicated to use, I recommend you to stay on the original app:
> [LowRes NX Coder on App Store](https://apps.apple.com/app/lowres-nx-coder/id1318884577).

Create your games and play it on the go using an iPhone or iPad. LowResRMX is a virtual game console and a game development environment that include all the tools to start and finish a project.

It **will** includes a project manager, a code editor and some tools to edit game assets: sprites, palettes, backgrounds, instruments and musics.

> **The documentation and the included tools are still in development.**

## Difference with Lowres NX

The goal 🥅 of the app is to provide a development environment to work with during my commutes in the subway, and later, being able to publish a real app on the stores.

The original LowRes NX, despite being an excellent development environment, lacks some capabilities IMHO.

### Background

4 background layers instead of 2 with 64x64 cells instead of 32x32.

Support for big cells of 16x16 pixels has been removed.

Removed 🗑️ syntaxes:

`CELL SIZE bg,size`

---

Alternative syntaxe to `FLIP` command:

`FLIP x,y`

Where `x` and `y` are both optional are accept any number. If it is different than zero, the flag will be set.

### Screen

A fantasy screen of 216x384 pixels.

With perfect squared pixels, a fixed portrait orientation, and it always fill the entire device screen.

Depending on the device screen ratio, the number of visible virtual pixels may vary from 216x288 for 4:3 devices to 177x384 for 19.5:9 devices.

To detect the visible virtual pixels, use the new functions `=SHOWN.W` and `=SHOWN.H`.

---

New functions to recover the visible fantasy display size:

`=SHOWN.W`<br>
`=SHOWN.H`

Returns the number of visible pixels in width and height respectivly.

---

New functions to query the safe area offset:

`=SAFE.L`<br>
`=SAFE.T`<br>
`=SAFE.R`<br>
`=SAFE.B`

Returns the offset in virtual pixels to apply from the boundary of the fantasy device screen to reach the inner safe area specified by the device operating system.

It can be used to avoid the top camera notch or the bottom inset full-width buttons.

### Colors

The 64 available colors do not respect the original EGA style of Lowres NX. Instead it use the FAMICUBE palette created by Arne.

> This make the usage of the color chooser of the original "Gfx Designer" from Lowres NX a bit difficult to use.

---

New settings with the `SYSTEM` command:

`SYSTEM setting,opacity`
<br>with `setting` goes from 1 to 4.

Allow to make the color 0 of the background layer opaque instead of transparent.

### Sprites

172 sprites instead of 64.

And theirs position support sub-pixels with a 1/16 pixel precisions.

### Input

Touchscreen only, with a float precision.

Virtual controller and the posibility to use Bluetooth controller has been removed.

Virtual keyboard are still present.

When the device virtual keyboard is hidden by the user, the corresponding flag of I/O [Registers](#Registers) is updated.

Removed 🗑️ syntaxes:

`=BUTTON(p,n)`, `=UP(p)`, `=DOWN(p)`, `=LEFT(p)`, `=RIGHT(p)`, `GAMEPAD n`, `TOUCHSCREEN`

---

Change about the `=TOUCH.X/Y` functions that return float.

---

New alternative to `WAIT` command:

`WAIT TAP`

Will wait for a device tap before continue execution.

### Flow control

New program flow control:

`ON value GOTO label,..`<br>
`ON value GOSUB label,..`<br>
`ON value RESTORE label,..`

Will `GOTO`, `GOSUB` or `RESTORE` to one of the `label` listed according to the `value`.

e.g.:

	ON I GOTO ZERO,ONE,TWO

### Data

New keyword to skip readed data:

`SKIP number`

Will skip a `number` of data to not read.

e.g.:

	DATA 0,1,2
	SKIP 2
	READ A
	PRINT A
	'WILL PRINT "2"

---

It's now possible to access data directly using their label:

`PRINT label`

It allow to store a string using the `DATA` keyword and reference it in the `PRINT` command.

e.g.:

	PRINT TEST
	TEST:
	DATA "GABUZOMEU"

It also work in variables, functions and commands:

    A$=TEST+LEFT$(TEST,2)

And it support numeric values as well:

    TEST2:
    DATA 123.4
    PRINT TEST2*3


### Math

New function to clamp numerical value:

`=CLAMP(value,min,max)`

Returns the `value` clamped between `min` and `max`.

---

New random generator and feature:

`=RND`

Same as before but with a different generator, it use [PCG](https://www.pcg-random.org/).

`RANDOMIZE seed,address`
`=RND address`
`=RND(max,address)`

Can specify an `address` where the internal state will be stored. It will consume 16 bytes.

This allow to use multiple generator at the same times.

---

New syntaxe to compute length:

`=LEN(x,y)`

Calculate the length of a 2D vector using the values of `x` and `y` as the coordinates for both axes.

---

New function that interpolates values between 0 and 1 using easing mathematical functions.

`=EASE(function,mode,value)`

Returns the interpolation of the `value` between 0.0 and 1.0 using the easing `function` and the `mode`.

|   | function |
|---|----------|
| 0 | linear   |
| 1 | sine     |
| 2 | quad     |
| 3 | cubic    |
| 4 | quart    |
| 5 | quint    |
| 6 | circ     |
| 7 | back     |
| 8 | elastic  |
| 9 | bounce   |

|     | mode  |
|-----|-------|
| -1  | in    |
|  0  | inout |
| +1  | out   |

### Text

New command to expose the existing overlay message API:

`MESSAGE text`

### Other

New command to help support program from the original Lowres NX:

`COMPAT`

> Will be removed at some point. Some commands from Lowres NX are already not available.

Enable the compatibility mode:

- It will force the rendering process to keep the original device screen.
- It will reverse the `RND` and `=RND()` to it's original behavior

It do not fix everything and make the program 100% compatible but it can help.

For instance, the commands and functions that has been removed will still be unavailable.

---

Removed 🗑️ commands:

`DISPLAY`

Removed 🗑️ reserved keywords:

`ANIM`, `CLOSE`, `DECLARE`, `DEF`, `FLASH`, `FN`, `FUNCTION`, `LBOUND`, `OPEN`, `OUTPUT`, `SHARED`, `STATIC`, `TEMPO`, `VOICE`, `WRITE`

### Memory

An almost compatible memory mapping.

The total addressable memory grow from 64 Kibi to 128 Kibi.

Lowres NX:

| addr  | size      | purpose          |
|-------|-----------|------------------|
| $0000 | 32 Kibi   | Cartridge ROM    |
| $8000 | 4 Kibi    | Character Data   |
| $9000 | 2 Kibi    | BG0 Data         |
| $9800 | 2 Kibi    | BG1 Data         |
| $A000 | 16 Kibi   | Working RAM      |
| $E000 | 4 Kibi    | Persistent RAM   |
| $FE00 | 256 Bytes | Sprite Registers |
| $FF00 | 32 Bytes  | Color Registers  |
| $FF20 |           | Video Registers  |
| $FF40 |           | Audio Registers  |
| $FF70 |           | I/O Registers    |

LowResRMX:

| addr   | size       | purpose          |
|--------|------------|------------------|
| $0000  | 8 Kibi     | BG0 data         |
| $2000  | 8 Kibi     | BG1 data         |
| $4000  | 8 Kibi     | BG2 data         |
| $6000  | 8 Kibi     | BG3 data         |
| $8000  | 4 Kibi     | Character Data   |
| $9000  | 20 Kibi    | Working RAM      |
| $E000  | 6 Kibi     | Persisent RAM    |
| $FB00  | 1020 Bytes | Sprite registers |
| $FF00  | 32 Bytes   | Color registers  |
| $FF20  | 10 Bytes   | Video registers  |
| $FF40  | 48 Bytes   | Audio registers  |
| $FF70  | 40 Bytes   | I/O registers    |
| $FFA0  | 6 Bytes    | DMA registers    |
| $10000 | 64 Kibi    | Cartridge ROM    |

TODO: Add particle/emitter registers

---

New command and registers for a fast memory copy.

`DMA COPY`<br>
`DMA COPY ROM`

Perform a fast memory copy that can be done only during interrupt calls.

It uses the following registers as data:

| addr  | purpose             |
|-------|---------------------|
| $FFA0 | Source address      |
| $FFA2 | Bytes count         |
| $FFA4 | Destination address |

> Will only works in a subprogram that has been called during an interrupts, including: `RASTER`, `VBL` and `PARTICLE`.

---

The `COPY` command is slightly faster.

### Particles

The particles library reused the sprites to make them appear, dissapear, change theirs characters data and move them accross the screen.

---

`PARTICLE first,count AT address`

Declare to use `count` sprites starting from the `first` and store the internal data at `address`.

It will consume 6 bytes per sprite in memory. For each sprite:

| addr | size    | purpose         |
|------|---------|-----------------|
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

Declare a `label` that contains DATA for the particle `appearance`.

`appearance` is a number between 0 and 23.

The associated DATA must contains a list of character number that will be used to draw the particle. Using a negative number will loop to the previous DATA value.

A typical example is to put zero `0` and minus one `-1` for the two last data values, as it will allow to make the particle dissapear until the sprite is reused.

e.g.:

	MY_APPEARANCE:
	DATA 1,2,3,4,0,-1
	'Will show the character 1 to 4, each per frame.
	'And loop with the character 0.

---

`PARTICLE OFF`

Clear each sprite position and each particle appearance.

---

`ON PARTICLE CALL subprogram`

For each particle, will call the subprogram using an interrupt.

The target `subprogram` will receive two arguments:
1. the `sprite` number,
2. and the particle `address`.

e.g.:

	SUB MYPART(SPR,ADDR)
		SY=PEEKW(ADDR+2)
		POKEW ADDR+2,SY-5
		'Change the speed in y axis
	END SUB

TODO: specify the number of cycles available.

---

`EMITTER count AT address`

Declare to use `count` emitter and store the internal data at `address`.

`count` is a number between 0 and 15.

It will consume 6 bytes per emitter in memory. For each emitter:

| addr | size    | purpose                |
|------|---------|------------------------|
| +0   | 2 Bytes | Position on x axis     |
| +2   | 2 Bytes | Position on y axis     |
| +4   | 1 Byte  | Delay before emit      |
| +5   | 1 Byte  | Number of time to emit |

---

`EMITTER emitter DATA label`

Declare a `label` that contains DATA for the `emitter`.

The associated data must follow this list:
1. `appearance`,
2. `outer` ring radius,
3. `inner` ring radius,
4. initial particle `speed` on x axis,
5. initial particle `speed` on y axis,
6. `count` of the number of particle to spawn at this frame,
7. number of frame to `delay` the re-emittion,
8. how many times to `repeat` the emittion.

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

----

Colors:

LowResRMX provides the user with a fixed palette of 64 colors.
The colors are grouped into sets of 4 called palette, with 8 palettes available for users to colorize their creations.

From now on, the terms:
- **a palette** apply to one of the 8 palettes with 4 colors inside,
- **a color** means one of the 4 colors within a palette.

Check color [registers](#Registers) for information about the memory mapping and data format.

<!--FIXME: spell checked END -->

---

Characters:

The 256 characters available in the dedicated memory are 8x8 pixels each and are assigned a number from 0 to 255.
Whenever this number is encountered in the background or sprite, the rendering process will draw the corresponding block of 8x8 pixels.

A character is a block of 8x8 pixels and take 16 bytes.

TODO: move information about memory later
Each pixels as 2 bits to store the color information: 0 to 3.

The first 8 bytes contain the low bits of all its pixels, followed by 8 more bytes containing the high bits of all pixels.

> For expert🔥 user, here's a function that allow to set one pixel of a character anywhere in the memory: <pre><code>SUB CHARAPSET(A,C,X,Y,V)
'SET A PIXEL OF A CHARA AT A MEMORY LOCATION
'IN A BASE ADDRESS
'IN C DEST CHAR INDEX
'IN X,Y SOURCE CELL COORDS
'IN V COLOR VALUE
'LOCAL R CHAR ADDRESS
'LOCAL B LOW OR HIGH COLOR BIT
'LOCAL I ITERATOR
&#09;R=A+C*16+(Y AND 7)
&#09;B=V AND 1
&#09;FOR I=0 TO 1
&#09;&#09;ROL R,1+X
&#09;&#09;POKE R,(PEEK(R) AND 254) OR B
&#09;&#09;ROR R,1+X
&#09;&#09;R=R+8
&#09;&#09;B=(V AND 2)\2
&#09;NEXT I
END SUB
</code></pre>

---

Backgrounds:

LowResRMX has 4 background layers, each have independent:
- X and Y position,
- and 64x64 character cells.

The 4 layers are drawn in order from 4th to 1st. Cell's of the 1st background will be visible above the other, unless the priority flag is used.

Each cells have it's own:
- character number ,
- a palette,
- an horizontal and vertical flip,
- a priority to alter which layer is drawn on top of other.

Layers are not framebuffer, You CANNOT easily paint pixels directly. Instead, every cells reference a characters number and those last hold the pixel color informations.

A Character is a block of 8x8 pixels, and each pixel can hold one of 4 colors.
It means, one character can only have 4 colors. Less than that, because the 1st color is means to be transparent ; one character can only have 3 distinct colors.

Each layer can be shifted using the position in X and Y axes. When reaching the edges, the cells will wrap around and still remain visible.
Put another way, when the rendering process tries to draw the 64th cell of a row, because it does not exist, it will draw the 1st cell instead.
This can be used to achieve infinite scrolling.

As say before, the 1st color of each palette it transparent expect for the 1st color of the 1st palette. It will determine the background color of the whole fantasy screen.

Sprites:

TODO: continue

TODO: a word on the background color

TODO: a word the priority

## Registers

Sprite Registers:

There are 170 sprites, each occupies 6 bytes:

| addr  | size    | purpose      |
|-------|---------|--------------|
| $FB00 | 6 Bytes | 1st sprite   |
| $FB06 | 6 Bytes | 2nd sprite   |
| $FB0C | 6 Bytes | 3rd sprite   |
| …     |         | …            |
| $FEF6 | 6 Bytes | 170th sprite |
| $FEFC | 4 Bytes | Not used     |

For each sprite:

| offset | size    | purpose              |
|--------|---------|----------------------|
| +0     | 2 Bytes | Position on X axis   |
| +2     | 2 Bytes | Position on Y axis   |
| +4     | 1 Byte  | Character number     |
| +5     | 1 Byte  | Character attributes |

Both position on x and y axis use sub-pixels values. To advance by 1 pixel, the values should get increased by 16.
Also, they are both offseted by 32 pixels. To place a sprite in the 0x0 coordinates, the values should be 512x512.

Attributes bits:

| bits | purpose         |
|------|-----------------|
| 0..2 | Palette number  |
| 3    | Horizontal flip |
| 4    | Vertical flip   |
| 5    | Priority        |
| 6..7 | Size            |

Sprite size:

| binary | purpose                        |
|--------|--------------------------------|
| %00    | 8x8 pixels or 1x1 character    |
| %01    | 16x16 pixels or 2x2 characters |
| %10    | 24x24 pixels or 3x3 characters |
| %11    | 32x32 pixels or 4x4 characters |

---

Color registers:

There are 8 palettes with 4 colors each:

| addr  | size       | purpose         |
|-------|------------|-----------------|
| $FF00 | 32 Bytes   | Color registers |

For each palette:

| offset | size    | purpose         |
|--------|---------|-----------------|
| +0     | 1 Byte  | 1st color value |
| +1     | 1 Byte  | 2nd color value |
| +2     | 1 Byte  | 3rd color value |
| +3     | 1 Byte  | 4th color value |

---

Video registers:

| addr  | size     | purpose                     |
|-------|----------|-----------------------------|
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
|------|----------------------------|
| 0    | Sprites enabled            |
| 1    | Background layer 0 enabled |
| 2    | Background layer 1 enabled |
| 3    | Background layer 2 enabled |
| 4    | Background layer 3 enabled |

---

TODO: Audio registers:

| addr  | size     | purpose                     |
|-------|----------|-----------------------------|
| $FF40  | 48 Bytes   | Audio registers  |

---

I/O registers:

| addr  | size     | purpose                        |
|-------|----------|--------------------------------|
| $FF70 | 4 Bytes  | Last touch position X          |
| $FF74 | 4 Bytes  | Last touch position Y          |
| $FF78 | 2 Bytes  | Pixels shown in width          |
| $FF7a | 2 Bytes  | Pixels shown in height         |
| $FF7c | 2 Bytes  | Pixels outside the safe zone   |
| $FF7e | 2 Bytes  | Pixels outside the safe zone   |
| $FF80 | 2 Bytes  | Pixels outside the safe zone   |
| $FF82 | 2 Bytes  | Pixels outside the safe zone   |
| $FF84 | 1 Byte   | ASCII code of last pressed key |
| $FF85 | 1 Byte   | Other I/O status bits          |

Last touch position X and Y are stored as float and currently LowResRMX do not have a way to peek float from memory, use TOUCH.X and TOUCH.Y function instead.

Pixels shown represent the number of fantasy pixels that is visible by the user according to their device screen ratio. TODO: link See: SHOWN.W/H

Pixels outsied the safe zone represent the number of fantasy pixels that are visible but should be considered unsafe for touch input as they are outside the safe area. TODO: link See: SAFE.L/T/R/B

Other I/O status bits:

| bits | purpose                                   |
|------|-------------------------------------------|
| 0    | Pause currently active                    |
| 1    | Fantasy screen currently touched          |
| 2    | Device virtual keyboard currently visible |

---

DMA registers:

| addr   | size       | purpose                 |
|--------|------------|-------------------------|
| $FFA0  | 2 Bytes    | Source address          |
| $FFA2  | 2 Bytes    | Number of bytes to copy |
| $FFA4  | 2 Bytes    | Destination address     |
