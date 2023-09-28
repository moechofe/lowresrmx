# LowResRMX manual

> This is a fork of the excellent fantasy console Lowres NX by Timo "Inutilis" Kloss.<br>
> This is a heavily modified version and way more complicated to use, I recommend you to stay on the original app:
> [LowRes NX Coder on App Store](https://apps.apple.com/app/lowres-nx-coder/id1318884577).

Create your games and play it on the go using an iPhone or iPad. LowResRMX is a virtual game console and a game development environment that include all the tools to start and finish a project.

It **will** includes a project manager, a code editor and some tools to edit game assets: sprites, palettes, backgrounds, instruments and musics.

> **The documentation and the included tools are still in development.**

## Difference with the original Lowres NX

The goal 🥅 of the app is to provide a development environment to work with during my commutes in the subway, and later, being able to publish a real App on the Apple Store.

The original LowRes NX, despite being an excellent development environment, lacks some capabilities to publish an App that looks credible.

My main concern is the limited screen size unable to cover the whole devices screen and the landscape orientation.

### Background layers

4 background layers instead of 2 with 64x64 cells instead of 32x32.

Support for big cells of 16x16 pixels has been removed.

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

TODO: Provide a simple tool to visualize the colors and theirs indices.

### Sprites

172 sprites instead of 64.

And theirs position support sub-pixels with a 1/16 pixel precisions.

### Input

Touchscreen only, with a float precision.

Virtual controller and the posibility to use Bluetooth controller has been removed.

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

---

Alternative usage for `PRINT` where it can now use label literal:

`PRINT label`

It allow to store a string using the `DATA` keyword and reference it in the `PRINT` command.

e.g.:

	PRINT GA
	GA:
	DATA "GABUZOMEU"

TODO: Do it for `=LEFT$`, `=MID$`, `=RIGHT$`.

### Other

New command to help support program from the original Lowres NX:

`COMPAT`

Enable the compatibility mode:

- It will force the rendering process to keep the original device screen.
- It will reverse the `RND` and `=RND()` to it's original behavior

It do not fix everything and make the program 100% compatible but it can help.

For instance, the commands and functions that has been removed will still be unavailable.

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
| $8000  | 20 Kibi    | Working RAM      |
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

`PARTICLE appearence DATA label`

Declare a `label` that contains DATA for the particle `appearence`.

`appearence` is a number between 0 and 23.

The associated DATA must contains a list of character number that will be used to draw the particle. Using a negative number will loop to the previous DATA value.

A typical example is to put zero `0` and minus one `-1` for the two last data values, as it will allow to make the particle dissapear until the sprite is reused.

e.g.:

	MY_APPEARANCE:
	DATA 1,2,3,4,0,-1
	'Will show the character 1 to 4, each per frame.
	'And loop with the character 0.

---

`PARTICLE OFF`

Clear each sprite position and each particle appearence.

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
1. `appearence`,
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


