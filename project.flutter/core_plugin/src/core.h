#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
//
// Copyright 2016 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef io_chip_h
#define io_chip_h

#define NUM_GAMEPADS 2

// ================ Gamepad ================

// union Gamepad {
//     struct {
//         uint8_t up:1;
//         uint8_t down:1;
//         uint8_t left:1;
//         uint8_t right:1;
//         uint8_t buttonA:1;
//         uint8_t buttonB:1;
//     };
//     uint8_t value;
// };

// ================ Status ================

union IOStatus
{
	struct
	{
		uint8_t pause : 1;
		uint8_t touch : 1;
		uint8_t keyboardEnabled : 1;
	};
	uint8_t value;
};

// ================ Attributes ================

// union IOAttributes {
//     struct {
//     };
//     uint8_t value;
// };

// ===============================================
// ================ I/O Registers ================
// ===============================================

struct IORegisters
{
	// TODO: remove gamepad
	// union Gamepad gamepads[NUM_GAMEPADS]; // 2 bytes
	float touchX;
	float touchY;
	struct
	{
		uint16_t width, height;
	} shown;
	struct
	{
		uint16_t left, top, right, bottom;
	} safe;
	char key;
	union IOStatus status;
	// union IOAttributes attr;
};

#endif /* io_chip_h */

//
// Copyright 2016 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef video_chip_h
#define video_chip_h

#define UNIVERSAL_WIDTH 177
#define UNIVERSAL_HEIGHT 288
#define ICON_WIDTH 180 // 22.5
#define ICON_HEIGHT 180 // 22.5
#define SCREEN_WIDTH 216 // 27x8
#define SCREEN_HEIGHT 384 // 48x8
#define NUM_CHARACTERS 256
#define NUM_PALETTES 8
#define PLANE_COLUMNS 64
#define PLANE_ROWS 64
#define NUM_SPRITES 170
#define SPRITE_OFFSET_X 32
#define SPRITE_OFFSET_Y 32

#define COLS_MASK (PLANE_COLUMNS-1)
#define ROWS_MASK (PLANE_ROWS-1)

#if __APPLE__
#if TARGET_OS_IPHONE
#define BGR 1
#endif
#endif

struct Core;

// ================ Character ================

// 16 bytes
struct Character {
    uint8_t data[16];
};

// ================ Sprite ================

union CharacterAttributes {
    struct {
        uint8_t palette:3;
        uint8_t flipX:1;
        uint8_t flipY:1;
        uint8_t priority:1;
        uint8_t size:2; // 1x1 - 4x4 characters
    };
    uint8_t value;
};

// 6 bytes
struct Sprite {
    uint16_t x;
    uint16_t y;
    uint8_t character;
    union CharacterAttributes attr;
};

// ================ Cell ================

// 2 bytes
struct Cell {
    uint8_t character;
    union CharacterAttributes attr;
};

// ================ Plane ================

// 8 Kibi
struct Plane {
    struct Cell cells[PLANE_ROWS][PLANE_COLUMNS];
};

// ===========================================
// ================ Video RAM ================
// ===========================================

// 36Kibi
struct VideoRam {
    struct Plane planeA; // 8Kibi
    struct Plane planeB; // 8Kibi
    struct Plane planeC; // 8Kibi
    struct Plane planeD; // 8Kibi
    struct Character characters[NUM_CHARACTERS]; // 4Kibi
};

// =================================================
// ================ Video Registers ================
// =================================================

struct SpriteRegisters {
    struct Sprite sprites[NUM_SPRITES]; // 256 bytes
};

struct ColorRegisters {
    uint8_t colors[NUM_PALETTES * 4]; // 32 bytes
};

union DisplayAttributes {
    struct {
        uint8_t spritesEnabled:1;
        uint8_t planeAEnabled:1;
        uint8_t planeBEnabled:1;
        uint8_t planeCEnabled:1;
        uint8_t planeDEnabled:1;
        // uint8_t planeACellSize:1;
        // uint8_t planeBCellSize:1;
        // uint8_t planeCCellSize:1;
        // uint8_t planeDCellSize:1;
    };
    uint8_t value;
};

// union ScrollMSB {
//     struct {
//         uint8_t aX:1;
//         uint8_t aY:1;
//         uint8_t bX:1;
//         uint8_t bY:1;
//         uint8_t cX:1;
//         uint8_t cY:1;
//         uint8_t dX:1;
//         uint8_t dY:1;
//     };
//     uint8_t value;
// };

struct VideoRegisters {
    uint16_t scrollAX;
    uint16_t scrollAY;
    uint16_t scrollBX;
    uint16_t scrollBY;
    uint16_t scrollCX;
    uint16_t scrollCY;
    uint16_t scrollDX;
    uint16_t scrollDY;
    // union ScrollMSB scrollMSB;
    uint16_t rasterLine;
    union DisplayAttributes attr;
};

// ===========================================
// ================ Functions ================
// ===========================================

void video_renderScreen(struct Core *core, uint32_t *outputRGB);

#endif /* video_chip_h */

//
// Copyright 2016-2020 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef audio_chip_h
#define audio_chip_h

#define NUM_VOICES 4
#define NUM_AUDIO_BUFFERS 6
#define AUDIO_FILTER_BUFFER_SIZE 3

// audio output channels for stereo
#define NUM_CHANNELS 2

struct Core;

enum WaveType {
    WaveTypeSawtooth,
    WaveTypeTriangle,
    WaveTypePulse,
    WaveTypeNoise
};

enum EnvState {
    EnvStateAttack,
    EnvStateDecay,
    EnvStateRelease
};

union VoiceStatus {
    struct {
        uint8_t volume:4;
        uint8_t mix:2;
        uint8_t init:1;
        uint8_t gate:1;
    };
    uint8_t value;
};

union VoiceAttributes {
    struct {
        uint8_t pulseWidth:4;
        uint8_t wave:2;
        uint8_t timeout:1;
    };
    uint8_t value;
};

enum LFOWaveType {
    LFOWaveTypeTriangle,
    LFOWaveTypeSawtooth,
    LFOWaveTypeSquare,
    LFOWaveTypeRandom
};

union LFOAttributes {
    struct {
        uint8_t wave:2;
        uint8_t invert:1;
        uint8_t envMode:1;
        uint8_t trigger:1;
    };
    uint8_t value;
};

struct Voice {
    uint8_t frequencyLow;
    uint8_t frequencyHigh;
    union VoiceStatus status;
    uint8_t peak;
    union VoiceAttributes attr;
    uint8_t length;
    struct {
        uint8_t envA:4;
        uint8_t envD:4;
    };
    struct {
        uint8_t envS:4;
        uint8_t envR:4;
    };
    union LFOAttributes lfoAttr;
    struct {
        uint8_t lfoFrequency:4;
        uint8_t lfoOscAmount:4;
    };
    struct {
        uint8_t lfoVolAmount:4;
        uint8_t lfoPWAmount:4;
    };
    uint8_t reserved2;
};

struct AudioRegisters {
    struct Voice voices[NUM_VOICES];
};

struct VoiceInternals {
    double accumulator;
    uint16_t noiseRandom;
    double envCounter;
    enum EnvState envState;
    double lfoAccumulator;
    bool lfoHold;
    uint16_t lfoRandom;
    double timeoutCounter;
};

struct AudioInternals {
    struct VoiceInternals voices[NUM_VOICES];
    struct AudioRegisters buffers[NUM_AUDIO_BUFFERS];
    int readBufferIndex;
    int writeBufferIndex;
    bool audioEnabled;
    int32_t filterBuffer[NUM_CHANNELS][AUDIO_FILTER_BUFFER_SIZE];
};

void audio_reset(struct Core *core);
void audio_bufferRegisters(struct Core *core);
void audio_renderAudio(struct Core *core, int16_t *output, int numSamples, int outputFrequency, int volume);

#endif /* audio_chip_h */

//
// Copyright 2016 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef machine_h
#define machine_h

#define VM_SIZE 0x20000
#define VM_MAX 0x1FFFF
#define PERSISTENT_RAM_SIZE 6144

struct DmaRegisters {
  uint16_t src_addr; // Support RAM or ROM
  uint16_t bytes_count;
  uint16_t dst_addr; // Only RAM
};

struct Core;

// 128Kibi
struct Machine {

    // 0x00000..0x09000
    struct VideoRam videoRam; // 36Kibi

    // 0x09000..0x0E000
    uint8_t workingRam[0x05000]; // 20Kibi

    // 0x0E000..0x0F800
    uint8_t persistentRam[PERSISTENT_RAM_SIZE]; // 6Kibi

    // TODO: absorbe nothing1 into persidentRam

    // 0x0F800..0x0FB00
    uint8_t nothing1[0x0FB00-0x0F800]; // 768 Bytes

    // 0x0FB00..0x0FF00
    struct SpriteRegisters spriteRegisters; // 1Kibi
    uint8_t nothing2[0x400 - sizeof(struct SpriteRegisters)]; // 4 bytes

    // 0x0FF00
    struct ColorRegisters colorRegisters; // 32Bytes

    // 0xFF20
    struct VideoRegisters videoRegisters;
    uint8_t nothing3[0x20 - sizeof(struct VideoRegisters)]; // 12 Bytes

    // 0xFF40
    struct AudioRegisters audioRegisters;

    // 0xFF70
    struct IORegisters ioRegisters;
    uint8_t nothing5[0x30 - sizeof(struct IORegisters)]; // 18 Bytes

    // 0xFFA0
    struct DmaRegisters dmaRegisters;
    uint8_t nothing6[0x10 - sizeof(struct DmaRegisters)]; // 10 Bytes

    // 0xFFB0
    uint8_t nothing7[0x10000 - 0xFFB0];

    // 0x10000..0x20000
    uint8_t cartridgeRom[0x10000]; // 64Kibi
};

struct MachineInternals {
    struct AudioInternals audioInternals;
    bool hasAccessedPersistent;
    bool hasChangedPersistent;
    bool isEnergySaving;
    int energySavingTimer;
		bool planeColor0IsOpaque[4];
};

void machine_init(struct Core *core);
void machine_reset(struct Core *core, bool resetPersistent);
int machine_peek(struct Core *core, int address);
int16_t machine_peek_short(struct Core *core, int address);
bool machine_poke(struct Core *core, int address, int value);
bool machine_poke_short(struct Core *core, int address, int16_t value);
void machine_enableAudio(struct Core *core);
void machine_suspendEnergySaving(struct Core *core, int numUpdates);

#endif /* machine_h */

//
// Copyright 2016 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef video_chip_h
#define video_chip_h

#define UNIVERSAL_WIDTH 177
#define UNIVERSAL_HEIGHT 288
#define ICON_WIDTH 180 // 22.5
#define ICON_HEIGHT 180 // 22.5
#define SCREEN_WIDTH 216 // 27x8
#define SCREEN_HEIGHT 384 // 48x8
#define NUM_CHARACTERS 256
#define NUM_PALETTES 8
#define PLANE_COLUMNS 64
#define PLANE_ROWS 64
#define NUM_SPRITES 170
#define SPRITE_OFFSET_X 32
#define SPRITE_OFFSET_Y 32

#define COLS_MASK (PLANE_COLUMNS-1)
#define ROWS_MASK (PLANE_ROWS-1)

#if __APPLE__
#if TARGET_OS_IPHONE
#define BGR 1
#endif
#endif

struct Core;

// ================ Character ================

// 16 bytes
struct Character {
    uint8_t data[16];
};

// ================ Sprite ================

union CharacterAttributes {
    struct {
        uint8_t palette:3;
        uint8_t flipX:1;
        uint8_t flipY:1;
        uint8_t priority:1;
        uint8_t size:2; // 1x1 - 4x4 characters
    };
    uint8_t value;
};

// 6 bytes
struct Sprite {
    uint16_t x;
    uint16_t y;
    uint8_t character;
    union CharacterAttributes attr;
};

// ================ Cell ================

// 2 bytes
struct Cell {
    uint8_t character;
    union CharacterAttributes attr;
};

// ================ Plane ================

// 8 Kibi
struct Plane {
    struct Cell cells[PLANE_ROWS][PLANE_COLUMNS];
};

// ===========================================
// ================ Video RAM ================
// ===========================================

// 36Kibi
struct VideoRam {
    struct Plane planeA; // 8Kibi
    struct Plane planeB; // 8Kibi
    struct Plane planeC; // 8Kibi
    struct Plane planeD; // 8Kibi
    struct Character characters[NUM_CHARACTERS]; // 4Kibi
};

// =================================================
// ================ Video Registers ================
// =================================================

struct SpriteRegisters {
    struct Sprite sprites[NUM_SPRITES]; // 256 bytes
};

struct ColorRegisters {
    uint8_t colors[NUM_PALETTES * 4]; // 32 bytes
};

union DisplayAttributes {
    struct {
        uint8_t spritesEnabled:1;
        uint8_t planeAEnabled:1;
        uint8_t planeBEnabled:1;
        uint8_t planeCEnabled:1;
        uint8_t planeDEnabled:1;
        // uint8_t planeACellSize:1;
        // uint8_t planeBCellSize:1;
        // uint8_t planeCCellSize:1;
        // uint8_t planeDCellSize:1;
    };
    uint8_t value;
};

// union ScrollMSB {
//     struct {
//         uint8_t aX:1;
//         uint8_t aY:1;
//         uint8_t bX:1;
//         uint8_t bY:1;
//         uint8_t cX:1;
//         uint8_t cY:1;
//         uint8_t dX:1;
//         uint8_t dY:1;
//     };
//     uint8_t value;
// };

struct VideoRegisters {
    uint16_t scrollAX;
    uint16_t scrollAY;
    uint16_t scrollBX;
    uint16_t scrollBY;
    uint16_t scrollCX;
    uint16_t scrollCY;
    uint16_t scrollDX;
    uint16_t scrollDY;
    // union ScrollMSB scrollMSB;
    uint16_t rasterLine;
    union DisplayAttributes attr;
};

// ===========================================
// ================ Functions ================
// ===========================================

void video_renderScreen(struct Core *core, uint32_t *outputRGB);

#endif /* video_chip_h */

//
// Copyright 2016 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef video_chip_h
#define video_chip_h

#define UNIVERSAL_WIDTH 177
#define UNIVERSAL_HEIGHT 288
#define ICON_WIDTH 180 // 22.5
#define ICON_HEIGHT 180 // 22.5
#define SCREEN_WIDTH 216 // 27x8
#define SCREEN_HEIGHT 384 // 48x8
#define NUM_CHARACTERS 256
#define NUM_PALETTES 8
#define PLANE_COLUMNS 64
#define PLANE_ROWS 64
#define NUM_SPRITES 170
#define SPRITE_OFFSET_X 32
#define SPRITE_OFFSET_Y 32

#define COLS_MASK (PLANE_COLUMNS-1)
#define ROWS_MASK (PLANE_ROWS-1)

#if __APPLE__
#if TARGET_OS_IPHONE
#define BGR 1
#endif
#endif

struct Core;

// ================ Character ================

// 16 bytes
struct Character {
    uint8_t data[16];
};

// ================ Sprite ================

union CharacterAttributes {
    struct {
        uint8_t palette:3;
        uint8_t flipX:1;
        uint8_t flipY:1;
        uint8_t priority:1;
        uint8_t size:2; // 1x1 - 4x4 characters
    };
    uint8_t value;
};

// 6 bytes
struct Sprite {
    uint16_t x;
    uint16_t y;
    uint8_t character;
    union CharacterAttributes attr;
};

// ================ Cell ================

// 2 bytes
struct Cell {
    uint8_t character;
    union CharacterAttributes attr;
};

// ================ Plane ================

// 8 Kibi
struct Plane {
    struct Cell cells[PLANE_ROWS][PLANE_COLUMNS];
};

// ===========================================
// ================ Video RAM ================
// ===========================================

// 36Kibi
struct VideoRam {
    struct Plane planeA; // 8Kibi
    struct Plane planeB; // 8Kibi
    struct Plane planeC; // 8Kibi
    struct Plane planeD; // 8Kibi
    struct Character characters[NUM_CHARACTERS]; // 4Kibi
};

// =================================================
// ================ Video Registers ================
// =================================================

struct SpriteRegisters {
    struct Sprite sprites[NUM_SPRITES]; // 256 bytes
};

struct ColorRegisters {
    uint8_t colors[NUM_PALETTES * 4]; // 32 bytes
};

union DisplayAttributes {
    struct {
        uint8_t spritesEnabled:1;
        uint8_t planeAEnabled:1;
        uint8_t planeBEnabled:1;
        uint8_t planeCEnabled:1;
        uint8_t planeDEnabled:1;
        // uint8_t planeACellSize:1;
        // uint8_t planeBCellSize:1;
        // uint8_t planeCCellSize:1;
        // uint8_t planeDCellSize:1;
    };
    uint8_t value;
};

// union ScrollMSB {
//     struct {
//         uint8_t aX:1;
//         uint8_t aY:1;
//         uint8_t bX:1;
//         uint8_t bY:1;
//         uint8_t cX:1;
//         uint8_t cY:1;
//         uint8_t dX:1;
//         uint8_t dY:1;
//     };
//     uint8_t value;
// };

struct VideoRegisters {
    uint16_t scrollAX;
    uint16_t scrollAY;
    uint16_t scrollBX;
    uint16_t scrollBY;
    uint16_t scrollCX;
    uint16_t scrollCY;
    uint16_t scrollDX;
    uint16_t scrollDY;
    // union ScrollMSB scrollMSB;
    uint16_t rasterLine;
    union DisplayAttributes attr;
};

// ===========================================
// ================ Functions ================
// ===========================================

void video_renderScreen(struct Core *core, uint32_t *outputRGB);

#endif /* video_chip_h */

//
// Copyright 2017-2018 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef overlay_data_h
#define overlay_data_h

extern uint8_t overlayColors[];
extern uint8_t overlayCharacters[];

#endif /* overlay_data_h */

//
// Copyright 2016 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef video_chip_h
#define video_chip_h

#define UNIVERSAL_WIDTH 177
#define UNIVERSAL_HEIGHT 288
#define ICON_WIDTH 180 // 22.5
#define ICON_HEIGHT 180 // 22.5
#define SCREEN_WIDTH 216 // 27x8
#define SCREEN_HEIGHT 384 // 48x8
#define NUM_CHARACTERS 256
#define NUM_PALETTES 8
#define PLANE_COLUMNS 64
#define PLANE_ROWS 64
#define NUM_SPRITES 170
#define SPRITE_OFFSET_X 32
#define SPRITE_OFFSET_Y 32

#define COLS_MASK (PLANE_COLUMNS-1)
#define ROWS_MASK (PLANE_ROWS-1)

#if __APPLE__
#if TARGET_OS_IPHONE
#define BGR 1
#endif
#endif

struct Core;

// ================ Character ================

// 16 bytes
struct Character {
    uint8_t data[16];
};

// ================ Sprite ================

union CharacterAttributes {
    struct {
        uint8_t palette:3;
        uint8_t flipX:1;
        uint8_t flipY:1;
        uint8_t priority:1;
        uint8_t size:2; // 1x1 - 4x4 characters
    };
    uint8_t value;
};

// 6 bytes
struct Sprite {
    uint16_t x;
    uint16_t y;
    uint8_t character;
    union CharacterAttributes attr;
};

// ================ Cell ================

// 2 bytes
struct Cell {
    uint8_t character;
    union CharacterAttributes attr;
};

// ================ Plane ================

// 8 Kibi
struct Plane {
    struct Cell cells[PLANE_ROWS][PLANE_COLUMNS];
};

// ===========================================
// ================ Video RAM ================
// ===========================================

// 36Kibi
struct VideoRam {
    struct Plane planeA; // 8Kibi
    struct Plane planeB; // 8Kibi
    struct Plane planeC; // 8Kibi
    struct Plane planeD; // 8Kibi
    struct Character characters[NUM_CHARACTERS]; // 4Kibi
};

// =================================================
// ================ Video Registers ================
// =================================================

struct SpriteRegisters {
    struct Sprite sprites[NUM_SPRITES]; // 256 bytes
};

struct ColorRegisters {
    uint8_t colors[NUM_PALETTES * 4]; // 32 bytes
};

union DisplayAttributes {
    struct {
        uint8_t spritesEnabled:1;
        uint8_t planeAEnabled:1;
        uint8_t planeBEnabled:1;
        uint8_t planeCEnabled:1;
        uint8_t planeDEnabled:1;
        // uint8_t planeACellSize:1;
        // uint8_t planeBCellSize:1;
        // uint8_t planeCCellSize:1;
        // uint8_t planeDCellSize:1;
    };
    uint8_t value;
};

// union ScrollMSB {
//     struct {
//         uint8_t aX:1;
//         uint8_t aY:1;
//         uint8_t bX:1;
//         uint8_t bY:1;
//         uint8_t cX:1;
//         uint8_t cY:1;
//         uint8_t dX:1;
//         uint8_t dY:1;
//     };
//     uint8_t value;
// };

struct VideoRegisters {
    uint16_t scrollAX;
    uint16_t scrollAY;
    uint16_t scrollBX;
    uint16_t scrollBY;
    uint16_t scrollCX;
    uint16_t scrollCY;
    uint16_t scrollDX;
    uint16_t scrollDY;
    // union ScrollMSB scrollMSB;
    uint16_t rasterLine;
    union DisplayAttributes attr;
};

// ===========================================
// ================ Functions ================
// ===========================================

void video_renderScreen(struct Core *core, uint32_t *outputRGB);

#endif /* video_chip_h */

//
// Copyright 2016-2019 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef text_lib_h
#define text_lib_h

#define INPUT_BUFFER_SIZE 256
#define OVERLAY_BG 4

struct Core;

struct TextLib {
    struct Core *core;
    union CharacterAttributes charAttr;
    int fontCharOffset;
    int windowX;
    int windowY;
    int windowWidth;
    int windowHeight;
    int windowBg;
    int cursorX;
    int cursorY;
    int bg;
    int sourceAddress;
    int sourceWidth;
    int sourceHeight;
    char inputBuffer[INPUT_BUFFER_SIZE];
    int inputLength;
    int blink;
};

void txtlib_printText(struct TextLib *lib, const char *text);
bool txtlib_deleteBackward(struct TextLib *lib);
void txtlib_writeText(struct TextLib *lib, const char *text, int x, int y);
void txtlib_writeNumber(struct TextLib *lib, int number, int digits, int x, int y);
void txtlib_inputBegin(struct TextLib *lib);
bool txtlib_inputUpdate(struct TextLib *lib);
void txtlib_clearWindow(struct TextLib *lib);
void txtlib_clearScreen(struct TextLib *lib);
void txtlib_clearBackground(struct TextLib *lib, int bg);
struct Cell *txtlib_getCell(struct TextLib *lib, int x, int y);
void txtlib_setCell(struct TextLib *lib, int x, int y, int character);
void txtlib_setCells(struct TextLib *lib, int fromX, int fromY, int toX, int toY, int character);
void txtlib_setCellsAttr(struct TextLib *lib, int fromX, int fromY, int toX, int toY, int pal, int flipX, int flipY, int prio);
void txtlib_scrollBackground(struct TextLib *lib, int fromX, int fromY, int toX, int toY, int deltaX, int deltaY);
void txtlib_copyBackground(struct TextLib *lib, int srcX, int srcY, int width, int height, int dstX, int dstY);
int txtlib_getSourceCell(struct TextLib *lib, int x, int y, bool getAttrs);
bool txtlib_setSourceCell(struct TextLib *lib, int x, int y, int character);

void txtlib_itobin(char *buffer, size_t buffersize, size_t width, int value);

#endif /* text_lib_h */

//
// Copyright 2017-2018 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef overlay_h
#define overlay_h

struct Core;
struct CoreInput;

struct Overlay {
    struct Plane plane;
    struct TextLib textLib;
    int timer;
    int messageTimer;
};

void overlay_init(struct Core *core);
void overlay_reset(struct Core *core);
void overlay_updateLayout(struct Core *core, struct CoreInput *input);
void overlay_updateState(struct Core *core);
void overlay_message(struct Core *core, const char *message);
void overlay_draw(struct Core *core, bool ingame);

#endif /* overlay_h */

//
// Copyright 2017-2020 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef interpreter_config_h
#define interpreter_config_h

#define MAX_TOKENS 16384
#define MAX_SYMBOLS 2048
#define MAX_LABEL_STACK_ITEMS 128
#define MAX_JUMP_LABEL_ITEMS 256
#define MAX_SUB_ITEMS 256
#define MAX_SIMPLE_VARIABLES 256
#define MAX_ARRAY_VARIABLES 256
#define SYMBOL_NAME_SIZE 21
#define MAX_ARRAY_DIMENSIONS 4
#define MAX_ARRAY_SIZE 32768
// XXX: #define MAX_CYCLES_TOTAL_PER_FRAME 17556
// XXX: #define MAX_CYCLES_PER_VBL 1140
// XXX: #define MAX_CYCLES_PER_RASTER 51
#define MAX_CYCLES_TOTAL_PER_FRAME 35112 // ??
#define MAX_CYCLES_PER_VBL 3420 // 1140*3 ??
#define MAX_CYCLES_PER_RASTER 204 // 51*4 OK
#define MAX_CYCLES_PER_PARTICLE 51 // ??
#define MAX_CYCLES_PER_EMITTER 102 // ??
#define TIMER_WRAP_VALUE 5184000

#endif /* interpreter_config_h */

//
// Copyright 2017-2020 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef interpreter_config_h
#define interpreter_config_h

#define MAX_TOKENS 16384
#define MAX_SYMBOLS 2048
#define MAX_LABEL_STACK_ITEMS 128
#define MAX_JUMP_LABEL_ITEMS 256
#define MAX_SUB_ITEMS 256
#define MAX_SIMPLE_VARIABLES 256
#define MAX_ARRAY_VARIABLES 256
#define SYMBOL_NAME_SIZE 21
#define MAX_ARRAY_DIMENSIONS 4
#define MAX_ARRAY_SIZE 32768
// XXX: #define MAX_CYCLES_TOTAL_PER_FRAME 17556
// XXX: #define MAX_CYCLES_PER_VBL 1140
// XXX: #define MAX_CYCLES_PER_RASTER 51
#define MAX_CYCLES_TOTAL_PER_FRAME 35112 // ??
#define MAX_CYCLES_PER_VBL 3420 // 1140*3 ??
#define MAX_CYCLES_PER_RASTER 204 // 51*4 OK
#define MAX_CYCLES_PER_PARTICLE 51 // ??
#define MAX_CYCLES_PER_EMITTER 102 // ??
#define TIMER_WRAP_VALUE 5184000

#endif /* interpreter_config_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef string_h
#define string_h

struct RCString {
    int refCount;
    char chars[1]; // ...
};

struct RCString *rcstring_new(const char *chars, size_t len);
void rcstring_retain(struct RCString *string);
void rcstring_release(struct RCString *string);

#endif /* string_h */

//
// Copyright 2017-2020 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef token_h
#define token_h

enum TokenType {
    TokenUndefined,

    TokenIdentifier,
    TokenStringIdentifier,
    TokenLabel,
    TokenFloat,
    TokenString,

    // Signs
    TokenColon,
    TokenComma,
    TokenSemicolon,
    TokenApostrophe,
    TokenEol,

    // Operators
    TokenEq,
    TokenGrEq,
    TokenLeEq,
    TokenUneq,
    TokenGr,
    TokenLe,
    TokenBracketOpen,
    TokenBracketClose,
    TokenPlus,
    TokenMinus,
    TokenMul,
    TokenDiv,
    TokenDivInt,
    TokenPow,
    TokenAND,
    TokenNOT,
    TokenOR,
    TokenXOR,
    TokenMOD,

    // Commands/Functions
    TokenABS,
    TokenACOS,
    TokenADD,
    TokenASC,
    TokenASIN,
    TokenATAN,
    TokenATTR,
    TokenBG,
    TokenBIN,
    // TokenBUTTON,
    TokenCALL,
    TokenCELLA,
    TokenCELLC,
    TokenCELL,
    TokenCHAR,
    TokenCHR,
    TokenCLS,
    TokenCLW,
    TokenCOLOR,
    TokenCOPY,
    TokenCOS,
    TokenCURSORX,
    TokenCURSORY,
    TokenDATA,
    TokenDEC,
    TokenDIM,
    // TokenDISPLAY,
    //TokenDOWN,
    TokenDO,
    TokenELSE,
    TokenEND,
    TokenENVELOPE,
    TokenEXIT,
    TokenEXP,
    TokenFILE,
    TokenFILES,
    TokenFILL,
    TokenFLIP,
    TokenFONT,
    TokenFOR,
    TokenFSIZE,
    //TokenGAMEPAD,
    TokenGLOBAL,
    TokenGOSUB,
    TokenGOTO,
    TokenHEX,
    TokenHCOS,
    TokenHIT,
    TokenHSIN,
    TokenHTAN,
    TokenIF,
    TokenINC,
    TokenINKEY,
    TokenINPUT,
    TokenINSTR,
    TokenINT,
    TokenKEYBOARD,
    TokenLEFTStr,
    //TokenLEFT,
    TokenLEN,
    TokenLET,
    TokenLFOA,
    TokenLFO,
    TokenLOAD,
    TokenLOCATE,
    TokenLOG,
    TokenLOOP,
    TokenMAX,
    TokenMCELLA,
    TokenMCELLC,
    TokenMCELL,
    TokenMID,
    TokenMIN,
    TokenCLAMP,
    TokenMUSIC,
    TokenNEXT,
    TokenNUMBER,
    TokenOFF,
    TokenON,
    TokenOPTIONAL,
    TokenPALETTE,
    TokenPAL,
    TokenPAUSE,
    TokenPEEKL,
    TokenPEEKW,
    TokenPEEK,
    TokenPI,
    TokenPLAY,
    TokenPOKEL,
    TokenPOKEW,
    TokenPOKE,
    TokenPRINT,
    TokenPRIO,
    TokenRANDOMIZE,
    TokenRASTER,
    TokenREAD,
    TokenSKIP,
    TokenREM,
    TokenREPEAT,
    TokenRESTORE,
    TokenRETURN,
    TokenRIGHTStr,
    //TokenRIGHT,
    TokenRND,
    TokenROL,
    TokenROM,
    TokenROR,
    TokenSAVE,
    TokenSCROLLX,
    TokenSCROLLY,
    TokenSCROLL,
    TokenSGN,
    TokenSIN,
    TokenSIZE,
    TokenSOUND,
    TokenSOURCE,
    TokenSPRITEA,
    TokenSPRITEC,
    TokenSPRITEX,
    TokenSPRITEY,
    TokenSPRITE,
    TokenSQR,
    TokenSTEP,
    TokenSTOP,
    TokenSTR,
    TokenSUB,
    TokenSWAP,
    TokenSYSTEM,
    TokenTAN,
    TokenTAP,
    TokenTEXT,
    TokenTHEN,
    TokenTIMER,
    TokenTINT,
    //TokenTOUCHSCREEN,
    TokenTOUCHX,
    TokenTOUCHY,
    TokenTOUCH,
    TokenTO,
    TokenTRACE,
    TokenTRACK,
    TokenUBOUND,
    TokenUNTIL,
    //TokenUP,
    TokenVAL,
    TokenVBL,
    TokenVIEW,
    TokenVOLUME,
    TokenWAIT,
    TokenWAVE,
    TokenWEND,
    TokenWHILE,
    TokenWINDOW,

    TokenSHOWNW,
    TokenSHOWNH,
    TokenSAFEL,
    TokenSAFET,
    TokenSAFER,
    TokenSAFEB,

    TokenPARTICLE,
    TokenEMITTER,
    TokenAT,
    TokenCOMPAT,
    TokenEASE,
    TokenMESSAGE,
    TokenDMA,
		TokenCEIL,

    // Reserved Keywords
    Token_reserved,
    // TokenANIM,
    // TokenCLOSE,
    // TokenDECLARE,
    // TokenDEF,
    // TokenFLASH,
    // TokenFN,
    // TokenFUNCTION,
    // TokenLBOUND,
    // TokenOPEN,
    // TokenOUTPUT,
    // TokenSHARED,
    // TokenSTATIC,
    // TokenTEMPO,
    // TokenVOICE,
    // TokenWRITE,

    Token_count
};

struct Token {
    enum TokenType type;
    union {
        float floatValue;
        struct RCString *stringValue;
        int symbolIndex;
        struct Token *jumpToken;
    };
    int sourcePosition;
};

extern const char *TokenStrings[];

#endif /* token_h */

//
// Copyright 2016-2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef tokenizer_h
#define tokenizer_h

struct Symbol {
    char name[SYMBOL_NAME_SIZE];
};

struct JumpLabelItem {
    int symbolIndex;
    struct Token *token;
};

struct SubItem {
    int symbolIndex;
    struct Token *token;
};

struct Tokenizer
{
    struct Token tokens[MAX_TOKENS];
    int numTokens;
    struct Symbol symbols[MAX_SYMBOLS];
    int numSymbols;

    struct JumpLabelItem jumpLabelItems[MAX_JUMP_LABEL_ITEMS];
    int numJumpLabelItems;
    struct SubItem subItems[MAX_SUB_ITEMS];
    int numSubItems;
};

struct CoreError tok_tokenizeProgram(struct Tokenizer *tokenizer, const char *sourceCode);
struct CoreError tok_tokenizeUppercaseProgram(struct Tokenizer *tokenizer, const char *sourceCode);
void tok_freeTokens(struct Tokenizer *tokenizer);
struct JumpLabelItem *tok_getJumpLabel(struct Tokenizer *tokenizer, int symbolIndex);
enum ErrorCode tok_setJumpLabel(struct Tokenizer *tokenizer, int symbolIndex, struct Token *token);
struct SubItem *tok_getSub(struct Tokenizer *tokenizer, int symbolIndex);
enum ErrorCode tok_setSub(struct Tokenizer *tokenizer, int symbolIndex, struct Token *token);

#endif /* tokenizer_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef string_h
#define string_h

struct RCString {
    int refCount;
    char chars[1]; // ...
};

struct RCString *rcstring_new(const char *chars, size_t len);
void rcstring_retain(struct RCString *string);
void rcstring_release(struct RCString *string);

#endif /* string_h */

//
// Copyright 2017-2020 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef token_h
#define token_h

enum TokenType {
    TokenUndefined,

    TokenIdentifier,
    TokenStringIdentifier,
    TokenLabel,
    TokenFloat,
    TokenString,

    // Signs
    TokenColon,
    TokenComma,
    TokenSemicolon,
    TokenApostrophe,
    TokenEol,

    // Operators
    TokenEq,
    TokenGrEq,
    TokenLeEq,
    TokenUneq,
    TokenGr,
    TokenLe,
    TokenBracketOpen,
    TokenBracketClose,
    TokenPlus,
    TokenMinus,
    TokenMul,
    TokenDiv,
    TokenDivInt,
    TokenPow,
    TokenAND,
    TokenNOT,
    TokenOR,
    TokenXOR,
    TokenMOD,

    // Commands/Functions
    TokenABS,
    TokenACOS,
    TokenADD,
    TokenASC,
    TokenASIN,
    TokenATAN,
    TokenATTR,
    TokenBG,
    TokenBIN,
    // TokenBUTTON,
    TokenCALL,
    TokenCELLA,
    TokenCELLC,
    TokenCELL,
    TokenCHAR,
    TokenCHR,
    TokenCLS,
    TokenCLW,
    TokenCOLOR,
    TokenCOPY,
    TokenCOS,
    TokenCURSORX,
    TokenCURSORY,
    TokenDATA,
    TokenDEC,
    TokenDIM,
    // TokenDISPLAY,
    //TokenDOWN,
    TokenDO,
    TokenELSE,
    TokenEND,
    TokenENVELOPE,
    TokenEXIT,
    TokenEXP,
    TokenFILE,
    TokenFILES,
    TokenFILL,
    TokenFLIP,
    TokenFONT,
    TokenFOR,
    TokenFSIZE,
    //TokenGAMEPAD,
    TokenGLOBAL,
    TokenGOSUB,
    TokenGOTO,
    TokenHEX,
    TokenHCOS,
    TokenHIT,
    TokenHSIN,
    TokenHTAN,
    TokenIF,
    TokenINC,
    TokenINKEY,
    TokenINPUT,
    TokenINSTR,
    TokenINT,
    TokenKEYBOARD,
    TokenLEFTStr,
    //TokenLEFT,
    TokenLEN,
    TokenLET,
    TokenLFOA,
    TokenLFO,
    TokenLOAD,
    TokenLOCATE,
    TokenLOG,
    TokenLOOP,
    TokenMAX,
    TokenMCELLA,
    TokenMCELLC,
    TokenMCELL,
    TokenMID,
    TokenMIN,
    TokenCLAMP,
    TokenMUSIC,
    TokenNEXT,
    TokenNUMBER,
    TokenOFF,
    TokenON,
    TokenOPTIONAL,
    TokenPALETTE,
    TokenPAL,
    TokenPAUSE,
    TokenPEEKL,
    TokenPEEKW,
    TokenPEEK,
    TokenPI,
    TokenPLAY,
    TokenPOKEL,
    TokenPOKEW,
    TokenPOKE,
    TokenPRINT,
    TokenPRIO,
    TokenRANDOMIZE,
    TokenRASTER,
    TokenREAD,
    TokenSKIP,
    TokenREM,
    TokenREPEAT,
    TokenRESTORE,
    TokenRETURN,
    TokenRIGHTStr,
    //TokenRIGHT,
    TokenRND,
    TokenROL,
    TokenROM,
    TokenROR,
    TokenSAVE,
    TokenSCROLLX,
    TokenSCROLLY,
    TokenSCROLL,
    TokenSGN,
    TokenSIN,
    TokenSIZE,
    TokenSOUND,
    TokenSOURCE,
    TokenSPRITEA,
    TokenSPRITEC,
    TokenSPRITEX,
    TokenSPRITEY,
    TokenSPRITE,
    TokenSQR,
    TokenSTEP,
    TokenSTOP,
    TokenSTR,
    TokenSUB,
    TokenSWAP,
    TokenSYSTEM,
    TokenTAN,
    TokenTAP,
    TokenTEXT,
    TokenTHEN,
    TokenTIMER,
    TokenTINT,
    //TokenTOUCHSCREEN,
    TokenTOUCHX,
    TokenTOUCHY,
    TokenTOUCH,
    TokenTO,
    TokenTRACE,
    TokenTRACK,
    TokenUBOUND,
    TokenUNTIL,
    //TokenUP,
    TokenVAL,
    TokenVBL,
    TokenVIEW,
    TokenVOLUME,
    TokenWAIT,
    TokenWAVE,
    TokenWEND,
    TokenWHILE,
    TokenWINDOW,

    TokenSHOWNW,
    TokenSHOWNH,
    TokenSAFEL,
    TokenSAFET,
    TokenSAFER,
    TokenSAFEB,

    TokenPARTICLE,
    TokenEMITTER,
    TokenAT,
    TokenCOMPAT,
    TokenEASE,
    TokenMESSAGE,
    TokenDMA,
		TokenCEIL,

    // Reserved Keywords
    Token_reserved,
    // TokenANIM,
    // TokenCLOSE,
    // TokenDECLARE,
    // TokenDEF,
    // TokenFLASH,
    // TokenFN,
    // TokenFUNCTION,
    // TokenLBOUND,
    // TokenOPEN,
    // TokenOUTPUT,
    // TokenSHARED,
    // TokenSTATIC,
    // TokenTEMPO,
    // TokenVOICE,
    // TokenWRITE,

    Token_count
};

struct Token {
    enum TokenType type;
    union {
        float floatValue;
        struct RCString *stringValue;
        int symbolIndex;
        struct Token *jumpToken;
    };
    int sourcePosition;
};

extern const char *TokenStrings[];

#endif /* token_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef error_h
#define error_h

enum ErrorCode {
    ErrorNone,
    
    ErrorCouldNotOpenProgram,
    ErrorTooManyTokens,
    ErrorRomIsFull,
    ErrorIndexAlreadyDefined,
    ErrorUnterminatedString,
    ErrorUnexpectedCharacter,
    ErrorReservedKeyword,
    ErrorSyntax,
    ErrorSymbolNameTooLong,
    ErrorTooManySymbols,
    ErrorTypeMismatch,
    ErrorOutOfMemory,
    ErrorElseWithoutIf,
    ErrorEndIfWithoutIf,
    ErrorExpectedCommand,
    ErrorNextWithoutFor,
    ErrorLoopWithoutDo,
    ErrorUntilWithoutRepeat,
    ErrorWendWithoutWhile,
    ErrorLabelAlreadyDefined,
    ErrorTooManyLabels,
    ErrorExpectedLabel,
    ErrorUndefinedLabel,
    ErrorArrayNotDimensionized,
    ErrorArrayAlreadyDimensionized,
    ErrorVariableAlreadyUsed,
    ErrorIndexOutOfBounds,
    ErrorWrongNumberOfDimensions,
    ErrorInvalidParameter,
    ErrorReturnWithoutGosub,
    ErrorStackOverflow,
    ErrorOutOfData,
    ErrorIllegalMemoryAccess,
    ErrorTooManyCPUCyclesInInterrupt,
    ErrorNotAllowedInInterrupt,
    ErrorIfWithoutEndIf,
    ErrorForWithoutNext,
    ErrorDoWithoutLoop,
    ErrorRepeatWithoutUntil,
    ErrorWhileWithoutWend,
    ErrorExitNotInsideLoop,
    ErrorDirectoryNotLoaded,
    ErrorDivisionByZero,
    ErrorVariableNotInitialized,
    ErrorArrayVariableWithoutIndex,
    ErrorEndSubWithoutSub,
    ErrorSubWithoutEndSub,
    ErrorSubCannotBeNested,
    ErrorUndefinedSubprogram,
    ErrorExpectedSubprogramName,
    ErrorArgumentCountMismatch,
    ErrorSubAlreadyDefined,
    ErrorTooManySubprograms,
    ErrorSharedOutsideOfASubprogram,
    ErrorGlobalInsideOfASubprogram,
    ErrorExitSubOutsideOfASubprogram,
    ErrorKeyboardNotEnabled,
    ErrorAutomaticPauseNotDisabled,
    ErrorNotAllowedOutsideOfInterrupt
};

struct CoreError {
    enum ErrorCode code;
    int sourcePosition;
};

const char *err_getString(enum ErrorCode errorCode);
struct CoreError err_makeCoreError(enum ErrorCode code, int sourcePosition);
struct CoreError err_noCoreError(void);

#endif /* error_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef error_h
#define error_h

enum ErrorCode {
    ErrorNone,
    
    ErrorCouldNotOpenProgram,
    ErrorTooManyTokens,
    ErrorRomIsFull,
    ErrorIndexAlreadyDefined,
    ErrorUnterminatedString,
    ErrorUnexpectedCharacter,
    ErrorReservedKeyword,
    ErrorSyntax,
    ErrorSymbolNameTooLong,
    ErrorTooManySymbols,
    ErrorTypeMismatch,
    ErrorOutOfMemory,
    ErrorElseWithoutIf,
    ErrorEndIfWithoutIf,
    ErrorExpectedCommand,
    ErrorNextWithoutFor,
    ErrorLoopWithoutDo,
    ErrorUntilWithoutRepeat,
    ErrorWendWithoutWhile,
    ErrorLabelAlreadyDefined,
    ErrorTooManyLabels,
    ErrorExpectedLabel,
    ErrorUndefinedLabel,
    ErrorArrayNotDimensionized,
    ErrorArrayAlreadyDimensionized,
    ErrorVariableAlreadyUsed,
    ErrorIndexOutOfBounds,
    ErrorWrongNumberOfDimensions,
    ErrorInvalidParameter,
    ErrorReturnWithoutGosub,
    ErrorStackOverflow,
    ErrorOutOfData,
    ErrorIllegalMemoryAccess,
    ErrorTooManyCPUCyclesInInterrupt,
    ErrorNotAllowedInInterrupt,
    ErrorIfWithoutEndIf,
    ErrorForWithoutNext,
    ErrorDoWithoutLoop,
    ErrorRepeatWithoutUntil,
    ErrorWhileWithoutWend,
    ErrorExitNotInsideLoop,
    ErrorDirectoryNotLoaded,
    ErrorDivisionByZero,
    ErrorVariableNotInitialized,
    ErrorArrayVariableWithoutIndex,
    ErrorEndSubWithoutSub,
    ErrorSubWithoutEndSub,
    ErrorSubCannotBeNested,
    ErrorUndefinedSubprogram,
    ErrorExpectedSubprogramName,
    ErrorArgumentCountMismatch,
    ErrorSubAlreadyDefined,
    ErrorTooManySubprograms,
    ErrorSharedOutsideOfASubprogram,
    ErrorGlobalInsideOfASubprogram,
    ErrorExitSubOutsideOfASubprogram,
    ErrorKeyboardNotEnabled,
    ErrorAutomaticPauseNotDisabled,
    ErrorNotAllowedOutsideOfInterrupt
};

struct CoreError {
    enum ErrorCode code;
    int sourcePosition;
};

const char *err_getString(enum ErrorCode errorCode);
struct CoreError err_makeCoreError(enum ErrorCode code, int sourcePosition);
struct CoreError err_noCoreError(void);

#endif /* error_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef string_h
#define string_h

struct RCString {
    int refCount;
    char chars[1]; // ...
};

struct RCString *rcstring_new(const char *chars, size_t len);
void rcstring_retain(struct RCString *string);
void rcstring_release(struct RCString *string);

#endif /* string_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef value_h
#define value_h

enum ValueType {
    ValueTypeNull,
    ValueTypeError,
    ValueTypeFloat,
    ValueTypeString
};

union Value {
    float floatValue;
    struct RCString *stringValue;
    union Value *reference;
    enum ErrorCode errorCode;
};

struct TypedValue {
    enum ValueType type;
    union Value v;
};

enum TypeClass {
    TypeClassAny,
    TypeClassNumeric,
    TypeClassString
};

extern union Value ValueDummy;

struct TypedValue val_makeError(enum ErrorCode errorCode);

#endif /* value_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef labels_h
#define labels_h

struct Interpreter;
struct Token;

enum LabelType {
    LabelTypeIF,
    LabelTypeELSE,
    LabelTypeELSEIF,
    LabelTypeFOR,
    LabelTypeFORVar,
    LabelTypeFORLimit,
    LabelTypeGOSUB,
    LabelTypeDO,
    LabelTypeREPEAT,
    LabelTypeWHILE,
    LabelTypeSUB,
    LabelTypeCALL,
    LabelTypeONCALL,
};

struct LabelStackItem {
    enum LabelType type;
    struct Token *token;
};

enum ErrorCode lab_pushLabelStackItem(struct Interpreter *interpreter, enum LabelType type, struct Token *token);
struct LabelStackItem *lab_popLabelStackItem(struct Interpreter *interpreter);
struct LabelStackItem *lab_peekLabelStackItem(struct Interpreter *interpreter);
struct LabelStackItem *lab_searchLabelStackItem(struct Interpreter *interpreter, enum LabelType types[], int numTypes);

#endif /* labels_h */

//
// Copyright 2017-2020 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef interpreter_config_h
#define interpreter_config_h

#define MAX_TOKENS 16384
#define MAX_SYMBOLS 2048
#define MAX_LABEL_STACK_ITEMS 128
#define MAX_JUMP_LABEL_ITEMS 256
#define MAX_SUB_ITEMS 256
#define MAX_SIMPLE_VARIABLES 256
#define MAX_ARRAY_VARIABLES 256
#define SYMBOL_NAME_SIZE 21
#define MAX_ARRAY_DIMENSIONS 4
#define MAX_ARRAY_SIZE 32768
// XXX: #define MAX_CYCLES_TOTAL_PER_FRAME 17556
// XXX: #define MAX_CYCLES_PER_VBL 1140
// XXX: #define MAX_CYCLES_PER_RASTER 51
#define MAX_CYCLES_TOTAL_PER_FRAME 35112 // ??
#define MAX_CYCLES_PER_VBL 3420 // 1140*3 ??
#define MAX_CYCLES_PER_RASTER 204 // 51*4 OK
#define MAX_CYCLES_PER_PARTICLE 51 // ??
#define MAX_CYCLES_PER_EMITTER 102 // ??
#define TIMER_WRAP_VALUE 5184000

#endif /* interpreter_config_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef error_h
#define error_h

enum ErrorCode {
    ErrorNone,
    
    ErrorCouldNotOpenProgram,
    ErrorTooManyTokens,
    ErrorRomIsFull,
    ErrorIndexAlreadyDefined,
    ErrorUnterminatedString,
    ErrorUnexpectedCharacter,
    ErrorReservedKeyword,
    ErrorSyntax,
    ErrorSymbolNameTooLong,
    ErrorTooManySymbols,
    ErrorTypeMismatch,
    ErrorOutOfMemory,
    ErrorElseWithoutIf,
    ErrorEndIfWithoutIf,
    ErrorExpectedCommand,
    ErrorNextWithoutFor,
    ErrorLoopWithoutDo,
    ErrorUntilWithoutRepeat,
    ErrorWendWithoutWhile,
    ErrorLabelAlreadyDefined,
    ErrorTooManyLabels,
    ErrorExpectedLabel,
    ErrorUndefinedLabel,
    ErrorArrayNotDimensionized,
    ErrorArrayAlreadyDimensionized,
    ErrorVariableAlreadyUsed,
    ErrorIndexOutOfBounds,
    ErrorWrongNumberOfDimensions,
    ErrorInvalidParameter,
    ErrorReturnWithoutGosub,
    ErrorStackOverflow,
    ErrorOutOfData,
    ErrorIllegalMemoryAccess,
    ErrorTooManyCPUCyclesInInterrupt,
    ErrorNotAllowedInInterrupt,
    ErrorIfWithoutEndIf,
    ErrorForWithoutNext,
    ErrorDoWithoutLoop,
    ErrorRepeatWithoutUntil,
    ErrorWhileWithoutWend,
    ErrorExitNotInsideLoop,
    ErrorDirectoryNotLoaded,
    ErrorDivisionByZero,
    ErrorVariableNotInitialized,
    ErrorArrayVariableWithoutIndex,
    ErrorEndSubWithoutSub,
    ErrorSubWithoutEndSub,
    ErrorSubCannotBeNested,
    ErrorUndefinedSubprogram,
    ErrorExpectedSubprogramName,
    ErrorArgumentCountMismatch,
    ErrorSubAlreadyDefined,
    ErrorTooManySubprograms,
    ErrorSharedOutsideOfASubprogram,
    ErrorGlobalInsideOfASubprogram,
    ErrorExitSubOutsideOfASubprogram,
    ErrorKeyboardNotEnabled,
    ErrorAutomaticPauseNotDisabled,
    ErrorNotAllowedOutsideOfInterrupt
};

struct CoreError {
    enum ErrorCode code;
    int sourcePosition;
};

const char *err_getString(enum ErrorCode errorCode);
struct CoreError err_makeCoreError(enum ErrorCode code, int sourcePosition);
struct CoreError err_noCoreError(void);

#endif /* error_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef string_h
#define string_h

struct RCString {
    int refCount;
    char chars[1]; // ...
};

struct RCString *rcstring_new(const char *chars, size_t len);
void rcstring_retain(struct RCString *string);
void rcstring_release(struct RCString *string);

#endif /* string_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef value_h
#define value_h

enum ValueType {
    ValueTypeNull,
    ValueTypeError,
    ValueTypeFloat,
    ValueTypeString
};

union Value {
    float floatValue;
    struct RCString *stringValue;
    union Value *reference;
    enum ErrorCode errorCode;
};

struct TypedValue {
    enum ValueType type;
    union Value v;
};

enum TypeClass {
    TypeClassAny,
    TypeClassNumeric,
    TypeClassString
};

extern union Value ValueDummy;

struct TypedValue val_makeError(enum ErrorCode errorCode);

#endif /* value_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef variables_h
#define variables_h

#define SUB_LEVEL_GLOBAL -1

struct Core;
struct Interpreter;

struct SimpleVariable {
    int symbolIndex;
    int8_t subLevel;
    int8_t isReference:1;
    enum ValueType type;
    union Value v;
};

struct ArrayVariable {
    int symbolIndex;
    int8_t subLevel;
    int8_t isReference:1;
    enum ValueType type;
    int numDimensions;
    int dimensionSizes[MAX_ARRAY_DIMENSIONS];
    int numValues;
    union Value *values;
};

struct SimpleVariable *var_getSimpleVariable(struct Interpreter *interpreter, int symbolIndex, int subLevel);
struct SimpleVariable *var_createSimpleVariable(struct Interpreter *interpreter, enum ErrorCode *errorCode, int symbolIndex, int subLevel, enum ValueType type, union Value *valueReference);
void var_freeSimpleVariables(struct Interpreter *interpreter, int minSubLevel);

struct ArrayVariable *var_getArrayVariable(struct Interpreter *interpreter, int symbolIndex, int subLevel);
union Value *var_getArrayValue(struct Interpreter *interpreter, struct ArrayVariable *variable, int *indices);
struct ArrayVariable *var_dimVariable(struct Interpreter *interpreter, enum ErrorCode *errorCode, int symbolIndex, int numDimensions, int *dimensionSizes);
struct ArrayVariable *var_createArrayVariable(struct Interpreter *interpreter, enum ErrorCode *errorCode, int symbolIndex, int subLevel, struct ArrayVariable *arrayReference);
void var_freeArrayVariables(struct Interpreter *interpreter, int minSubLevel);

#endif /* variables_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef string_h
#define string_h

struct RCString {
    int refCount;
    char chars[1]; // ...
};

struct RCString *rcstring_new(const char *chars, size_t len);
void rcstring_retain(struct RCString *string);
void rcstring_release(struct RCString *string);

#endif /* string_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef data_h
#define data_h

struct Interpreter;
struct Token;

void dat_nextData(struct Interpreter *interpreter);
void dat_restoreData(struct Interpreter *interpreter, struct Token *jumpToken);

struct Token* dat_reachData(struct Interpreter *interpreter, struct Token *jumpToken);

struct Token* dat_readData(struct Token *jumpToken, int skip);
float dat_readFloat(struct Token *jumpToken, int skip, float def);
uint8_t dat_readU8(struct Token *jumpToken, int skip, uint8_t def);

struct RCString *dat_readString(struct Token *jumpToken, int skip);

#endif /* data_h */

//
// Copyright 2016 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef video_chip_h
#define video_chip_h

#define UNIVERSAL_WIDTH 177
#define UNIVERSAL_HEIGHT 288
#define ICON_WIDTH 180 // 22.5
#define ICON_HEIGHT 180 // 22.5
#define SCREEN_WIDTH 216 // 27x8
#define SCREEN_HEIGHT 384 // 48x8
#define NUM_CHARACTERS 256
#define NUM_PALETTES 8
#define PLANE_COLUMNS 64
#define PLANE_ROWS 64
#define NUM_SPRITES 170
#define SPRITE_OFFSET_X 32
#define SPRITE_OFFSET_Y 32

#define COLS_MASK (PLANE_COLUMNS-1)
#define ROWS_MASK (PLANE_ROWS-1)

#if __APPLE__
#if TARGET_OS_IPHONE
#define BGR 1
#endif
#endif

struct Core;

// ================ Character ================

// 16 bytes
struct Character {
    uint8_t data[16];
};

// ================ Sprite ================

union CharacterAttributes {
    struct {
        uint8_t palette:3;
        uint8_t flipX:1;
        uint8_t flipY:1;
        uint8_t priority:1;
        uint8_t size:2; // 1x1 - 4x4 characters
    };
    uint8_t value;
};

// 6 bytes
struct Sprite {
    uint16_t x;
    uint16_t y;
    uint8_t character;
    union CharacterAttributes attr;
};

// ================ Cell ================

// 2 bytes
struct Cell {
    uint8_t character;
    union CharacterAttributes attr;
};

// ================ Plane ================

// 8 Kibi
struct Plane {
    struct Cell cells[PLANE_ROWS][PLANE_COLUMNS];
};

// ===========================================
// ================ Video RAM ================
// ===========================================

// 36Kibi
struct VideoRam {
    struct Plane planeA; // 8Kibi
    struct Plane planeB; // 8Kibi
    struct Plane planeC; // 8Kibi
    struct Plane planeD; // 8Kibi
    struct Character characters[NUM_CHARACTERS]; // 4Kibi
};

// =================================================
// ================ Video Registers ================
// =================================================

struct SpriteRegisters {
    struct Sprite sprites[NUM_SPRITES]; // 256 bytes
};

struct ColorRegisters {
    uint8_t colors[NUM_PALETTES * 4]; // 32 bytes
};

union DisplayAttributes {
    struct {
        uint8_t spritesEnabled:1;
        uint8_t planeAEnabled:1;
        uint8_t planeBEnabled:1;
        uint8_t planeCEnabled:1;
        uint8_t planeDEnabled:1;
        // uint8_t planeACellSize:1;
        // uint8_t planeBCellSize:1;
        // uint8_t planeCCellSize:1;
        // uint8_t planeDCellSize:1;
    };
    uint8_t value;
};

// union ScrollMSB {
//     struct {
//         uint8_t aX:1;
//         uint8_t aY:1;
//         uint8_t bX:1;
//         uint8_t bY:1;
//         uint8_t cX:1;
//         uint8_t cY:1;
//         uint8_t dX:1;
//         uint8_t dY:1;
//     };
//     uint8_t value;
// };

struct VideoRegisters {
    uint16_t scrollAX;
    uint16_t scrollAY;
    uint16_t scrollBX;
    uint16_t scrollBY;
    uint16_t scrollCX;
    uint16_t scrollCY;
    uint16_t scrollDX;
    uint16_t scrollDY;
    // union ScrollMSB scrollMSB;
    uint16_t rasterLine;
    union DisplayAttributes attr;
};

// ===========================================
// ================ Functions ================
// ===========================================

void video_renderScreen(struct Core *core, uint32_t *outputRGB);

#endif /* video_chip_h */

//
// Copyright 2016-2019 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef text_lib_h
#define text_lib_h

#define INPUT_BUFFER_SIZE 256
#define OVERLAY_BG 4

struct Core;

struct TextLib {
    struct Core *core;
    union CharacterAttributes charAttr;
    int fontCharOffset;
    int windowX;
    int windowY;
    int windowWidth;
    int windowHeight;
    int windowBg;
    int cursorX;
    int cursorY;
    int bg;
    int sourceAddress;
    int sourceWidth;
    int sourceHeight;
    char inputBuffer[INPUT_BUFFER_SIZE];
    int inputLength;
    int blink;
};

void txtlib_printText(struct TextLib *lib, const char *text);
bool txtlib_deleteBackward(struct TextLib *lib);
void txtlib_writeText(struct TextLib *lib, const char *text, int x, int y);
void txtlib_writeNumber(struct TextLib *lib, int number, int digits, int x, int y);
void txtlib_inputBegin(struct TextLib *lib);
bool txtlib_inputUpdate(struct TextLib *lib);
void txtlib_clearWindow(struct TextLib *lib);
void txtlib_clearScreen(struct TextLib *lib);
void txtlib_clearBackground(struct TextLib *lib, int bg);
struct Cell *txtlib_getCell(struct TextLib *lib, int x, int y);
void txtlib_setCell(struct TextLib *lib, int x, int y, int character);
void txtlib_setCells(struct TextLib *lib, int fromX, int fromY, int toX, int toY, int character);
void txtlib_setCellsAttr(struct TextLib *lib, int fromX, int fromY, int toX, int toY, int pal, int flipX, int flipY, int prio);
void txtlib_scrollBackground(struct TextLib *lib, int fromX, int fromY, int toX, int toY, int deltaX, int deltaY);
void txtlib_copyBackground(struct TextLib *lib, int srcX, int srcY, int width, int height, int dstX, int dstY);
int txtlib_getSourceCell(struct TextLib *lib, int x, int y, bool getAttrs);
bool txtlib_setSourceCell(struct TextLib *lib, int x, int y, int character);

void txtlib_itobin(char *buffer, size_t buffersize, size_t width, int value);

#endif /* text_lib_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef sprites_lib_h
#define sprites_lib_h

struct Core;
struct Sprite;

struct SpritesLib {
    struct Core *core;
    int lastHit;
};

bool sprlib_isSpriteOnScreen(struct Sprite *sprite);
bool sprlib_checkCollision(struct SpritesLib *lib, int checkIndex, int firstIndex, int lastIndex);

#endif /* sprites_lib_h */

//
// Copyright 2016-2020 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef audio_chip_h
#define audio_chip_h

#define NUM_VOICES 4
#define NUM_AUDIO_BUFFERS 6
#define AUDIO_FILTER_BUFFER_SIZE 3

// audio output channels for stereo
#define NUM_CHANNELS 2

struct Core;

enum WaveType {
    WaveTypeSawtooth,
    WaveTypeTriangle,
    WaveTypePulse,
    WaveTypeNoise
};

enum EnvState {
    EnvStateAttack,
    EnvStateDecay,
    EnvStateRelease
};

union VoiceStatus {
    struct {
        uint8_t volume:4;
        uint8_t mix:2;
        uint8_t init:1;
        uint8_t gate:1;
    };
    uint8_t value;
};

union VoiceAttributes {
    struct {
        uint8_t pulseWidth:4;
        uint8_t wave:2;
        uint8_t timeout:1;
    };
    uint8_t value;
};

enum LFOWaveType {
    LFOWaveTypeTriangle,
    LFOWaveTypeSawtooth,
    LFOWaveTypeSquare,
    LFOWaveTypeRandom
};

union LFOAttributes {
    struct {
        uint8_t wave:2;
        uint8_t invert:1;
        uint8_t envMode:1;
        uint8_t trigger:1;
    };
    uint8_t value;
};

struct Voice {
    uint8_t frequencyLow;
    uint8_t frequencyHigh;
    union VoiceStatus status;
    uint8_t peak;
    union VoiceAttributes attr;
    uint8_t length;
    struct {
        uint8_t envA:4;
        uint8_t envD:4;
    };
    struct {
        uint8_t envS:4;
        uint8_t envR:4;
    };
    union LFOAttributes lfoAttr;
    struct {
        uint8_t lfoFrequency:4;
        uint8_t lfoOscAmount:4;
    };
    struct {
        uint8_t lfoVolAmount:4;
        uint8_t lfoPWAmount:4;
    };
    uint8_t reserved2;
};

struct AudioRegisters {
    struct Voice voices[NUM_VOICES];
};

struct VoiceInternals {
    double accumulator;
    uint16_t noiseRandom;
    double envCounter;
    enum EnvState envState;
    double lfoAccumulator;
    bool lfoHold;
    uint16_t lfoRandom;
    double timeoutCounter;
};

struct AudioInternals {
    struct VoiceInternals voices[NUM_VOICES];
    struct AudioRegisters buffers[NUM_AUDIO_BUFFERS];
    int readBufferIndex;
    int writeBufferIndex;
    bool audioEnabled;
    int32_t filterBuffer[NUM_CHANNELS][AUDIO_FILTER_BUFFER_SIZE];
};

void audio_reset(struct Core *core);
void audio_bufferRegisters(struct Core *core);
void audio_renderAudio(struct Core *core, int16_t *output, int numSamples, int outputFrequency, int volume);

#endif /* audio_chip_h */

//
// Copyright 2018 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef audio_lib_h
#define audio_lib_h

#define NUM_SOUNDS 16
#define NUM_PATTERNS 64
#define NUM_TRACKS 64
#define NUM_TRACK_ROWS 32

struct Core;

struct ComposerPlayer {
    int sourceAddress;
    int index; // pattern for music, otherwise track
    int speed;
    int tick;
    int row;
    bool willBreak;
};

struct AudioLib {
    struct Core *core;
    int sourceAddress;

    struct ComposerPlayer musicPlayer;
    struct ComposerPlayer trackPlayers[NUM_VOICES];
};

void audlib_play(struct AudioLib *lib, int voiceIndex, float pitch, int len, int sound);
void audlib_copySound(struct AudioLib *lib, int sourceAddress, int sound, int voiceIndex);
void audlib_playMusic(struct AudioLib *lib, int startPattern);
void audlib_playTrack(struct AudioLib *lib, int track, int voiceIndex);
void audlib_stopAll(struct AudioLib *lib);
void audlib_stopVoice(struct AudioLib *lib, int voiceIndex);
void audlib_update(struct AudioLib *lib);

#endif /* audio_lib_h */

//
// Copyright 2016 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef io_chip_h
#define io_chip_h

#define NUM_GAMEPADS 2

// ================ Gamepad ================

// union Gamepad {
//     struct {
//         uint8_t up:1;
//         uint8_t down:1;
//         uint8_t left:1;
//         uint8_t right:1;
//         uint8_t buttonA:1;
//         uint8_t buttonB:1;
//     };
//     uint8_t value;
// };

// ================ Status ================

union IOStatus
{
	struct
	{
		uint8_t pause : 1;
		uint8_t touch : 1;
		uint8_t keyboardEnabled : 1;
	};
	uint8_t value;
};

// ================ Attributes ================

// union IOAttributes {
//     struct {
//     };
//     uint8_t value;
// };

// ===============================================
// ================ I/O Registers ================
// ===============================================

struct IORegisters
{
	// TODO: remove gamepad
	// union Gamepad gamepads[NUM_GAMEPADS]; // 2 bytes
	float touchX;
	float touchY;
	struct
	{
		uint16_t width, height;
	} shown;
	struct
	{
		uint16_t left, top, right, bottom;
	} safe;
	char key;
	union IOStatus status;
	// union IOAttributes attr;
};

#endif /* io_chip_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef error_h
#define error_h

enum ErrorCode {
    ErrorNone,
    
    ErrorCouldNotOpenProgram,
    ErrorTooManyTokens,
    ErrorRomIsFull,
    ErrorIndexAlreadyDefined,
    ErrorUnterminatedString,
    ErrorUnexpectedCharacter,
    ErrorReservedKeyword,
    ErrorSyntax,
    ErrorSymbolNameTooLong,
    ErrorTooManySymbols,
    ErrorTypeMismatch,
    ErrorOutOfMemory,
    ErrorElseWithoutIf,
    ErrorEndIfWithoutIf,
    ErrorExpectedCommand,
    ErrorNextWithoutFor,
    ErrorLoopWithoutDo,
    ErrorUntilWithoutRepeat,
    ErrorWendWithoutWhile,
    ErrorLabelAlreadyDefined,
    ErrorTooManyLabels,
    ErrorExpectedLabel,
    ErrorUndefinedLabel,
    ErrorArrayNotDimensionized,
    ErrorArrayAlreadyDimensionized,
    ErrorVariableAlreadyUsed,
    ErrorIndexOutOfBounds,
    ErrorWrongNumberOfDimensions,
    ErrorInvalidParameter,
    ErrorReturnWithoutGosub,
    ErrorStackOverflow,
    ErrorOutOfData,
    ErrorIllegalMemoryAccess,
    ErrorTooManyCPUCyclesInInterrupt,
    ErrorNotAllowedInInterrupt,
    ErrorIfWithoutEndIf,
    ErrorForWithoutNext,
    ErrorDoWithoutLoop,
    ErrorRepeatWithoutUntil,
    ErrorWhileWithoutWend,
    ErrorExitNotInsideLoop,
    ErrorDirectoryNotLoaded,
    ErrorDivisionByZero,
    ErrorVariableNotInitialized,
    ErrorArrayVariableWithoutIndex,
    ErrorEndSubWithoutSub,
    ErrorSubWithoutEndSub,
    ErrorSubCannotBeNested,
    ErrorUndefinedSubprogram,
    ErrorExpectedSubprogramName,
    ErrorArgumentCountMismatch,
    ErrorSubAlreadyDefined,
    ErrorTooManySubprograms,
    ErrorSharedOutsideOfASubprogram,
    ErrorGlobalInsideOfASubprogram,
    ErrorExitSubOutsideOfASubprogram,
    ErrorKeyboardNotEnabled,
    ErrorAutomaticPauseNotDisabled,
    ErrorNotAllowedOutsideOfInterrupt
};

struct CoreError {
    enum ErrorCode code;
    int sourcePosition;
};

const char *err_getString(enum ErrorCode errorCode);
struct CoreError err_makeCoreError(enum ErrorCode code, int sourcePosition);
struct CoreError err_noCoreError(void);

#endif /* error_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef data_manager_h
#define data_manager_h

#define MAX_ENTRIES 16
#define DATA_SIZE 0x10000
#define ENTRY_COMMENT_SIZE 32

struct DataEntry {
    char comment[ENTRY_COMMENT_SIZE];
    int start;
    int length;
};

struct DataManager {
    struct DataEntry entries[MAX_ENTRIES];
    uint8_t *data;
    const char *diskSourceCode;
};

void data_init(struct DataManager *manager);
void data_deinit(struct DataManager *manager);
void data_reset(struct DataManager *manager);
struct CoreError data_import(struct DataManager *manager, const char *input, bool keepSourceCode);
struct CoreError data_uppercaseImport(struct DataManager *manager, const char *input, bool keepSourceCode);
char *data_export(struct DataManager *manager);

int data_currentSize(struct DataManager *manager);

bool data_canSetEntry(struct DataManager *manager, int index, int length);
void data_setEntry(struct DataManager *manager, int index, const char *comment, uint8_t *source, int length);

#endif /* data_manager_h */

//
// Copyright 2017-2020 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef interpreter_config_h
#define interpreter_config_h

#define MAX_TOKENS 16384
#define MAX_SYMBOLS 2048
#define MAX_LABEL_STACK_ITEMS 128
#define MAX_JUMP_LABEL_ITEMS 256
#define MAX_SUB_ITEMS 256
#define MAX_SIMPLE_VARIABLES 256
#define MAX_ARRAY_VARIABLES 256
#define SYMBOL_NAME_SIZE 21
#define MAX_ARRAY_DIMENSIONS 4
#define MAX_ARRAY_SIZE 32768
// XXX: #define MAX_CYCLES_TOTAL_PER_FRAME 17556
// XXX: #define MAX_CYCLES_PER_VBL 1140
// XXX: #define MAX_CYCLES_PER_RASTER 51
#define MAX_CYCLES_TOTAL_PER_FRAME 35112 // ??
#define MAX_CYCLES_PER_VBL 3420 // 1140*3 ??
#define MAX_CYCLES_PER_RASTER 204 // 51*4 OK
#define MAX_CYCLES_PER_PARTICLE 51 // ??
#define MAX_CYCLES_PER_EMITTER 102 // ??
#define TIMER_WRAP_VALUE 5184000

#endif /* interpreter_config_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef string_h
#define string_h

struct RCString {
    int refCount;
    char chars[1]; // ...
};

struct RCString *rcstring_new(const char *chars, size_t len);
void rcstring_retain(struct RCString *string);
void rcstring_release(struct RCString *string);

#endif /* string_h */

//
// Copyright 2017-2020 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef token_h
#define token_h

enum TokenType {
    TokenUndefined,

    TokenIdentifier,
    TokenStringIdentifier,
    TokenLabel,
    TokenFloat,
    TokenString,

    // Signs
    TokenColon,
    TokenComma,
    TokenSemicolon,
    TokenApostrophe,
    TokenEol,

    // Operators
    TokenEq,
    TokenGrEq,
    TokenLeEq,
    TokenUneq,
    TokenGr,
    TokenLe,
    TokenBracketOpen,
    TokenBracketClose,
    TokenPlus,
    TokenMinus,
    TokenMul,
    TokenDiv,
    TokenDivInt,
    TokenPow,
    TokenAND,
    TokenNOT,
    TokenOR,
    TokenXOR,
    TokenMOD,

    // Commands/Functions
    TokenABS,
    TokenACOS,
    TokenADD,
    TokenASC,
    TokenASIN,
    TokenATAN,
    TokenATTR,
    TokenBG,
    TokenBIN,
    // TokenBUTTON,
    TokenCALL,
    TokenCELLA,
    TokenCELLC,
    TokenCELL,
    TokenCHAR,
    TokenCHR,
    TokenCLS,
    TokenCLW,
    TokenCOLOR,
    TokenCOPY,
    TokenCOS,
    TokenCURSORX,
    TokenCURSORY,
    TokenDATA,
    TokenDEC,
    TokenDIM,
    // TokenDISPLAY,
    //TokenDOWN,
    TokenDO,
    TokenELSE,
    TokenEND,
    TokenENVELOPE,
    TokenEXIT,
    TokenEXP,
    TokenFILE,
    TokenFILES,
    TokenFILL,
    TokenFLIP,
    TokenFONT,
    TokenFOR,
    TokenFSIZE,
    //TokenGAMEPAD,
    TokenGLOBAL,
    TokenGOSUB,
    TokenGOTO,
    TokenHEX,
    TokenHCOS,
    TokenHIT,
    TokenHSIN,
    TokenHTAN,
    TokenIF,
    TokenINC,
    TokenINKEY,
    TokenINPUT,
    TokenINSTR,
    TokenINT,
    TokenKEYBOARD,
    TokenLEFTStr,
    //TokenLEFT,
    TokenLEN,
    TokenLET,
    TokenLFOA,
    TokenLFO,
    TokenLOAD,
    TokenLOCATE,
    TokenLOG,
    TokenLOOP,
    TokenMAX,
    TokenMCELLA,
    TokenMCELLC,
    TokenMCELL,
    TokenMID,
    TokenMIN,
    TokenCLAMP,
    TokenMUSIC,
    TokenNEXT,
    TokenNUMBER,
    TokenOFF,
    TokenON,
    TokenOPTIONAL,
    TokenPALETTE,
    TokenPAL,
    TokenPAUSE,
    TokenPEEKL,
    TokenPEEKW,
    TokenPEEK,
    TokenPI,
    TokenPLAY,
    TokenPOKEL,
    TokenPOKEW,
    TokenPOKE,
    TokenPRINT,
    TokenPRIO,
    TokenRANDOMIZE,
    TokenRASTER,
    TokenREAD,
    TokenSKIP,
    TokenREM,
    TokenREPEAT,
    TokenRESTORE,
    TokenRETURN,
    TokenRIGHTStr,
    //TokenRIGHT,
    TokenRND,
    TokenROL,
    TokenROM,
    TokenROR,
    TokenSAVE,
    TokenSCROLLX,
    TokenSCROLLY,
    TokenSCROLL,
    TokenSGN,
    TokenSIN,
    TokenSIZE,
    TokenSOUND,
    TokenSOURCE,
    TokenSPRITEA,
    TokenSPRITEC,
    TokenSPRITEX,
    TokenSPRITEY,
    TokenSPRITE,
    TokenSQR,
    TokenSTEP,
    TokenSTOP,
    TokenSTR,
    TokenSUB,
    TokenSWAP,
    TokenSYSTEM,
    TokenTAN,
    TokenTAP,
    TokenTEXT,
    TokenTHEN,
    TokenTIMER,
    TokenTINT,
    //TokenTOUCHSCREEN,
    TokenTOUCHX,
    TokenTOUCHY,
    TokenTOUCH,
    TokenTO,
    TokenTRACE,
    TokenTRACK,
    TokenUBOUND,
    TokenUNTIL,
    //TokenUP,
    TokenVAL,
    TokenVBL,
    TokenVIEW,
    TokenVOLUME,
    TokenWAIT,
    TokenWAVE,
    TokenWEND,
    TokenWHILE,
    TokenWINDOW,

    TokenSHOWNW,
    TokenSHOWNH,
    TokenSAFEL,
    TokenSAFET,
    TokenSAFER,
    TokenSAFEB,

    TokenPARTICLE,
    TokenEMITTER,
    TokenAT,
    TokenCOMPAT,
    TokenEASE,
    TokenMESSAGE,
    TokenDMA,
		TokenCEIL,

    // Reserved Keywords
    Token_reserved,
    // TokenANIM,
    // TokenCLOSE,
    // TokenDECLARE,
    // TokenDEF,
    // TokenFLASH,
    // TokenFN,
    // TokenFUNCTION,
    // TokenLBOUND,
    // TokenOPEN,
    // TokenOUTPUT,
    // TokenSHARED,
    // TokenSTATIC,
    // TokenTEMPO,
    // TokenVOICE,
    // TokenWRITE,

    Token_count
};

struct Token {
    enum TokenType type;
    union {
        float floatValue;
        struct RCString *stringValue;
        int symbolIndex;
        struct Token *jumpToken;
    };
    int sourcePosition;
};

extern const char *TokenStrings[];

#endif /* token_h */

//
// Copyright 2016-2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef tokenizer_h
#define tokenizer_h

struct Symbol {
    char name[SYMBOL_NAME_SIZE];
};

struct JumpLabelItem {
    int symbolIndex;
    struct Token *token;
};

struct SubItem {
    int symbolIndex;
    struct Token *token;
};

struct Tokenizer
{
    struct Token tokens[MAX_TOKENS];
    int numTokens;
    struct Symbol symbols[MAX_SYMBOLS];
    int numSymbols;

    struct JumpLabelItem jumpLabelItems[MAX_JUMP_LABEL_ITEMS];
    int numJumpLabelItems;
    struct SubItem subItems[MAX_SUB_ITEMS];
    int numSubItems;
};

struct CoreError tok_tokenizeProgram(struct Tokenizer *tokenizer, const char *sourceCode);
struct CoreError tok_tokenizeUppercaseProgram(struct Tokenizer *tokenizer, const char *sourceCode);
void tok_freeTokens(struct Tokenizer *tokenizer);
struct JumpLabelItem *tok_getJumpLabel(struct Tokenizer *tokenizer, int symbolIndex);
enum ErrorCode tok_setJumpLabel(struct Tokenizer *tokenizer, int symbolIndex, struct Token *token);
struct SubItem *tok_getSub(struct Tokenizer *tokenizer, int symbolIndex);
enum ErrorCode tok_setSub(struct Tokenizer *tokenizer, int symbolIndex, struct Token *token);

#endif /* tokenizer_h */

//
// Copyright 2016 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef video_chip_h
#define video_chip_h

#define UNIVERSAL_WIDTH 177
#define UNIVERSAL_HEIGHT 288
#define ICON_WIDTH 180 // 22.5
#define ICON_HEIGHT 180 // 22.5
#define SCREEN_WIDTH 216 // 27x8
#define SCREEN_HEIGHT 384 // 48x8
#define NUM_CHARACTERS 256
#define NUM_PALETTES 8
#define PLANE_COLUMNS 64
#define PLANE_ROWS 64
#define NUM_SPRITES 170
#define SPRITE_OFFSET_X 32
#define SPRITE_OFFSET_Y 32

#define COLS_MASK (PLANE_COLUMNS-1)
#define ROWS_MASK (PLANE_ROWS-1)

#if __APPLE__
#if TARGET_OS_IPHONE
#define BGR 1
#endif
#endif

struct Core;

// ================ Character ================

// 16 bytes
struct Character {
    uint8_t data[16];
};

// ================ Sprite ================

union CharacterAttributes {
    struct {
        uint8_t palette:3;
        uint8_t flipX:1;
        uint8_t flipY:1;
        uint8_t priority:1;
        uint8_t size:2; // 1x1 - 4x4 characters
    };
    uint8_t value;
};

// 6 bytes
struct Sprite {
    uint16_t x;
    uint16_t y;
    uint8_t character;
    union CharacterAttributes attr;
};

// ================ Cell ================

// 2 bytes
struct Cell {
    uint8_t character;
    union CharacterAttributes attr;
};

// ================ Plane ================

// 8 Kibi
struct Plane {
    struct Cell cells[PLANE_ROWS][PLANE_COLUMNS];
};

// ===========================================
// ================ Video RAM ================
// ===========================================

// 36Kibi
struct VideoRam {
    struct Plane planeA; // 8Kibi
    struct Plane planeB; // 8Kibi
    struct Plane planeC; // 8Kibi
    struct Plane planeD; // 8Kibi
    struct Character characters[NUM_CHARACTERS]; // 4Kibi
};

// =================================================
// ================ Video Registers ================
// =================================================

struct SpriteRegisters {
    struct Sprite sprites[NUM_SPRITES]; // 256 bytes
};

struct ColorRegisters {
    uint8_t colors[NUM_PALETTES * 4]; // 32 bytes
};

union DisplayAttributes {
    struct {
        uint8_t spritesEnabled:1;
        uint8_t planeAEnabled:1;
        uint8_t planeBEnabled:1;
        uint8_t planeCEnabled:1;
        uint8_t planeDEnabled:1;
        // uint8_t planeACellSize:1;
        // uint8_t planeBCellSize:1;
        // uint8_t planeCCellSize:1;
        // uint8_t planeDCellSize:1;
    };
    uint8_t value;
};

// union ScrollMSB {
//     struct {
//         uint8_t aX:1;
//         uint8_t aY:1;
//         uint8_t bX:1;
//         uint8_t bY:1;
//         uint8_t cX:1;
//         uint8_t cY:1;
//         uint8_t dX:1;
//         uint8_t dY:1;
//     };
//     uint8_t value;
// };

struct VideoRegisters {
    uint16_t scrollAX;
    uint16_t scrollAY;
    uint16_t scrollBX;
    uint16_t scrollBY;
    uint16_t scrollCX;
    uint16_t scrollCY;
    uint16_t scrollDX;
    uint16_t scrollDY;
    // union ScrollMSB scrollMSB;
    uint16_t rasterLine;
    union DisplayAttributes attr;
};

// ===========================================
// ================ Functions ================
// ===========================================

void video_renderScreen(struct Core *core, uint32_t *outputRGB);

#endif /* video_chip_h */

#ifndef particles_lib_h
#define particles_lib_h

#define EMITTER_MAX 16
#define APPEARANCE_MAX 24
#define SPAWNER_MAX 16

#define PARTICLE_MEM_X 0
#define PARTICLE_MEM_Y 2
#define PARTICLE_MEM_APPEARANCE 4
#define PARTICLE_MEM_FRAME 5
#define PARTICLE_MEM_SIZE 6

#define EMITTER_MEM_X 0
#define EMITTER_MEM_Y 2
#define EMITTER_MEM_DELAY 4
#define EMITTER_MEM_REPEAT 5
#define EMITTER_MEM_SIZE 6

#define EMITTER_DATA_APPEARANCE 0
#define EMITTER_DATA_SHAPE 1
#define EMITTER_DATA_OUTER 2
#define EMITTER_DATA_INNER 3
#define EMITTER_DATA_ARC 4
#define EMITTER_DATA_ROTATION 5
#define EMITTER_DATA_SPEED_X 6
#define EMITTER_DATA_SPEED_Y 7
#define EMITTER_DATA_GRAVITY 8
#define EMITTER_DATA_COUNT 9
#define EMITTER_DATA_DELAY 10
#define EMITTER_DATA_REPEAT 11

// TODO: EMITTER_MAX and SPAWNER_MAX should be the same, right?

struct Core;

struct ParticlesLib
{
    struct Core *core;

    int first_sprite_id;
    int pool_count;
    int pool_next_id;

    int particles_data_addr;
    struct Token *particles_update;
    struct Token *apperances_label[APPEARANCE_MAX];

    int emitters_count;
    int emitters_data_addr;
    struct Token *emitters_label[SPAWNER_MAX];

    // for PARTICLE sub token during interrupt
    int interrupt_sprite_id;
    int interrupt_particle_addr;

		// for EMITTER sub token during interrupt
		int interrupt_emitter_id;
		int interrupt_emitter_addr;
};

void prtclib_setupPool(struct ParticlesLib *lib,int firstSprite,int poolCount,int particleAddr);
void prtclib_setApperanceLabel(struct ParticlesLib *lib,int apperanceId,struct Token *label);

void prtclib_setupEmitter(struct ParticlesLib *lib,int poolCount,int particleAddr);
void prtclib_setSpawnerLabel(struct ParticlesLib *lib,int emitterId,struct Token *label);

void prtclib_spawn(struct ParticlesLib *lib,int emitterId,float posX,float posY);
void prtclib_stop(struct ParticlesLib *lib,int emitterId);

void prtclib_update(struct Core *core,struct ParticlesLib *lib);
void prtclib_interrupt(struct Core *core,struct ParticlesLib *lib);
void prtclib_clear(struct Core *core,struct ParticlesLib *lib);

#endif

/*
 * PCG Random Number Generation for C.
 *
 * Copyright 2014 Melissa O'Neill <oneill@pcg-random.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For additional information about the PCG random number generation scheme,
 * including its license and other licensing options, visit
 *
 *     http://www.pcg-random.org
 */

/*
 * This code is derived from the full C implementation, which is in turn
 * derived from the canonical C++ PCG implementation. The C++ version
 * has many additional features and is preferable if you can use C++ in
 * your project.
 */

#ifndef PCG_BASIC_H_INCLUDED
#define PCG_BASIC_H_INCLUDED 1

#if __cplusplus
extern "C" {
#endif

struct pcg_state_setseq_64 {    // Internals are *Private*.
    uint64_t state;             // RNG state.  All values are possible.
    uint64_t inc;               // Controls which RNG sequence (stream) is
                                // selected. Must *always* be odd.
};
typedef struct pcg_state_setseq_64 pcg32_random_t;

// If you *must* statically initialize it, here's one.

#define PCG32_INITIALIZER   { 0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL }

// pcg32_srandom(initstate, initseq)
// pcg32_srandom_r(rng, initstate, initseq):
//     Seed the rng.  Specified in two parts, state initializer and a
//     sequence selection constant (a.k.a. stream id)

void pcg32_srandom(uint64_t initstate, uint64_t initseq);
void pcg32_srandom_r(pcg32_random_t* rng, uint64_t initstate,
                     uint64_t initseq);

// pcg32_random()
// pcg32_random_r(rng)
//     Generate a uniformly distributed 32-bit random number

uint32_t pcg32_random(void);
uint32_t pcg32_random_r(pcg32_random_t* rng);

// pcg32_boundedrand(bound):
// pcg32_boundedrand_r(rng, bound):
//     Generate a uniformly distributed number, r, where 0 <= r < bound

uint32_t pcg32_boundedrand(uint32_t bound);
uint32_t pcg32_boundedrand_r(pcg32_random_t* rng, uint32_t bound);

#if __cplusplus
}
#endif

#endif // PCG_BASIC_H_INCLUDED

//
// Copyright 2016-2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef interpreter_h
#define interpreter_h

#define BAS_TRUE -1.0f
#define BAS_FALSE 0.0f

struct Core;

enum Pass {
    PassPrepare,
    PassRun
};

enum State {
    StateNoProgram,
    StateEvaluate,
    StateInput,
    StatePaused,
    StateWaitForDisk,
    StateEnd
};

enum Mode {
    ModeNone,
    ModeMain,
    ModeInterrupt
};

enum InterruptType {
    InterruptTypeRaster,
    InterruptTypeVBL,
    InterruptTypeParticle,
		InterruptTypeEmitter,
};

struct Interpreter {
    const char *sourceCode;

    enum Pass pass;
    enum State state;
    enum Mode mode;
    struct Token *pc;
    int subLevel;
    int cycles;
    int interruptOverCycles;
    bool debug;
    bool handlesPause;
    int cpuLoadDisplay;
    int cpuLoadMax;
    int cpuLoadTimer;

    bool compat;

    struct Tokenizer tokenizer;

    struct DataManager romDataManager;

    struct LabelStackItem labelStackItems[MAX_LABEL_STACK_ITEMS];
    int numLabelStackItems;

    bool isSingleLineIf;

    struct SimpleVariable simpleVariables[MAX_SIMPLE_VARIABLES];
    int numSimpleVariables;
    struct ArrayVariable arrayVariables[MAX_ARRAY_VARIABLES];
    int numArrayVariables;
    struct RCString *nullString;

    struct Token *firstData;
    struct Token *lastData;
    struct Token *currentDataToken;
    struct Token *currentDataValueToken;

    struct Token *currentOnRasterToken;
    struct Token *currentOnVBLToken;
    struct Token *currentOnParticleToken;
		struct Token *currentOnEmitterToken;

    int waitCount;
    bool waitTap;
    bool exitEvaluation;
    // union Gamepad lastFrameGamepads[NUM_GAMEPADS];
    union IOStatus lastFrameIOStatus;
    float timer;
    int seed;
    bool isKeyboardOptional;
    union Value *lastVariableValue;

    struct TextLib textLib;
    struct SpritesLib spritesLib;
    struct AudioLib audioLib;
    struct ParticlesLib particlesLib;

    pcg32_random_t defaultRng;
};

void itp_init(struct Core *core);
void itp_deinit(struct Core *core);
struct CoreError itp_compileProgram(struct Core *core, const char *sourceCode);
void itp_runProgram(struct Core *core);
void itp_runInterrupt(struct Core *core, enum InterruptType type);
void itp_didFinishVBL(struct Core *core);
void itp_endProgram(struct Core *core);
void itp_freeProgram(struct Core *core);

enum ValueType itp_getIdentifierTokenValueType(struct Token *token);
union Value *itp_readVariable(struct Core *core, enum ValueType *type, enum ErrorCode *errorCode, bool forWriting);
struct TypedValue itp_evaluateExpression(struct Core *core, enum TypeClass typeClass);
struct TypedValue itp_evaluateNumericExpression(struct Core *core, int min, int max);
struct TypedValue itp_evaluateOptionalExpression(struct Core *core, enum TypeClass typeClass);
struct TypedValue itp_evaluateOptionalNumericExpression(struct Core *core, int min, int max);
bool itp_isEndOfCommand(struct Interpreter *interpreter);
enum ErrorCode itp_endOfCommand(struct Interpreter *interpreter);
enum ErrorCode itp_labelStackError(struct LabelStackItem *item);

#endif /* interpreter_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef error_h
#define error_h

enum ErrorCode {
    ErrorNone,
    
    ErrorCouldNotOpenProgram,
    ErrorTooManyTokens,
    ErrorRomIsFull,
    ErrorIndexAlreadyDefined,
    ErrorUnterminatedString,
    ErrorUnexpectedCharacter,
    ErrorReservedKeyword,
    ErrorSyntax,
    ErrorSymbolNameTooLong,
    ErrorTooManySymbols,
    ErrorTypeMismatch,
    ErrorOutOfMemory,
    ErrorElseWithoutIf,
    ErrorEndIfWithoutIf,
    ErrorExpectedCommand,
    ErrorNextWithoutFor,
    ErrorLoopWithoutDo,
    ErrorUntilWithoutRepeat,
    ErrorWendWithoutWhile,
    ErrorLabelAlreadyDefined,
    ErrorTooManyLabels,
    ErrorExpectedLabel,
    ErrorUndefinedLabel,
    ErrorArrayNotDimensionized,
    ErrorArrayAlreadyDimensionized,
    ErrorVariableAlreadyUsed,
    ErrorIndexOutOfBounds,
    ErrorWrongNumberOfDimensions,
    ErrorInvalidParameter,
    ErrorReturnWithoutGosub,
    ErrorStackOverflow,
    ErrorOutOfData,
    ErrorIllegalMemoryAccess,
    ErrorTooManyCPUCyclesInInterrupt,
    ErrorNotAllowedInInterrupt,
    ErrorIfWithoutEndIf,
    ErrorForWithoutNext,
    ErrorDoWithoutLoop,
    ErrorRepeatWithoutUntil,
    ErrorWhileWithoutWend,
    ErrorExitNotInsideLoop,
    ErrorDirectoryNotLoaded,
    ErrorDivisionByZero,
    ErrorVariableNotInitialized,
    ErrorArrayVariableWithoutIndex,
    ErrorEndSubWithoutSub,
    ErrorSubWithoutEndSub,
    ErrorSubCannotBeNested,
    ErrorUndefinedSubprogram,
    ErrorExpectedSubprogramName,
    ErrorArgumentCountMismatch,
    ErrorSubAlreadyDefined,
    ErrorTooManySubprograms,
    ErrorSharedOutsideOfASubprogram,
    ErrorGlobalInsideOfASubprogram,
    ErrorExitSubOutsideOfASubprogram,
    ErrorKeyboardNotEnabled,
    ErrorAutomaticPauseNotDisabled,
    ErrorNotAllowedOutsideOfInterrupt
};

struct CoreError {
    enum ErrorCode code;
    int sourcePosition;
};

const char *err_getString(enum ErrorCode errorCode);
struct CoreError err_makeCoreError(enum ErrorCode code, int sourcePosition);
struct CoreError err_noCoreError(void);

#endif /* error_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef data_manager_h
#define data_manager_h

#define MAX_ENTRIES 16
#define DATA_SIZE 0x10000
#define ENTRY_COMMENT_SIZE 32

struct DataEntry {
    char comment[ENTRY_COMMENT_SIZE];
    int start;
    int length;
};

struct DataManager {
    struct DataEntry entries[MAX_ENTRIES];
    uint8_t *data;
    const char *diskSourceCode;
};

void data_init(struct DataManager *manager);
void data_deinit(struct DataManager *manager);
void data_reset(struct DataManager *manager);
struct CoreError data_import(struct DataManager *manager, const char *input, bool keepSourceCode);
struct CoreError data_uppercaseImport(struct DataManager *manager, const char *input, bool keepSourceCode);
char *data_export(struct DataManager *manager);

int data_currentSize(struct DataManager *manager);

bool data_canSetEntry(struct DataManager *manager, int index, int length);
void data_setEntry(struct DataManager *manager, int index, const char *comment, uint8_t *source, int length);

#endif /* data_manager_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef disk_drive_h
#define disk_drive_h

struct Core;

struct DiskDrive {
    struct DataManager dataManager;
};

void disk_init(struct Core *core);
void disk_deinit(struct Core *core);
void disk_reset(struct Core *core);

bool disk_prepare(struct Core *core);
bool disk_saveFile(struct Core *core, int index, char *comment, int address, int length);
bool disk_loadFile(struct Core *core, int index, int address, int maxLength, int offset, bool *pokeFailed);

#endif /* disk_drive_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef error_h
#define error_h

enum ErrorCode {
    ErrorNone,
    
    ErrorCouldNotOpenProgram,
    ErrorTooManyTokens,
    ErrorRomIsFull,
    ErrorIndexAlreadyDefined,
    ErrorUnterminatedString,
    ErrorUnexpectedCharacter,
    ErrorReservedKeyword,
    ErrorSyntax,
    ErrorSymbolNameTooLong,
    ErrorTooManySymbols,
    ErrorTypeMismatch,
    ErrorOutOfMemory,
    ErrorElseWithoutIf,
    ErrorEndIfWithoutIf,
    ErrorExpectedCommand,
    ErrorNextWithoutFor,
    ErrorLoopWithoutDo,
    ErrorUntilWithoutRepeat,
    ErrorWendWithoutWhile,
    ErrorLabelAlreadyDefined,
    ErrorTooManyLabels,
    ErrorExpectedLabel,
    ErrorUndefinedLabel,
    ErrorArrayNotDimensionized,
    ErrorArrayAlreadyDimensionized,
    ErrorVariableAlreadyUsed,
    ErrorIndexOutOfBounds,
    ErrorWrongNumberOfDimensions,
    ErrorInvalidParameter,
    ErrorReturnWithoutGosub,
    ErrorStackOverflow,
    ErrorOutOfData,
    ErrorIllegalMemoryAccess,
    ErrorTooManyCPUCyclesInInterrupt,
    ErrorNotAllowedInInterrupt,
    ErrorIfWithoutEndIf,
    ErrorForWithoutNext,
    ErrorDoWithoutLoop,
    ErrorRepeatWithoutUntil,
    ErrorWhileWithoutWend,
    ErrorExitNotInsideLoop,
    ErrorDirectoryNotLoaded,
    ErrorDivisionByZero,
    ErrorVariableNotInitialized,
    ErrorArrayVariableWithoutIndex,
    ErrorEndSubWithoutSub,
    ErrorSubWithoutEndSub,
    ErrorSubCannotBeNested,
    ErrorUndefinedSubprogram,
    ErrorExpectedSubprogramName,
    ErrorArgumentCountMismatch,
    ErrorSubAlreadyDefined,
    ErrorTooManySubprograms,
    ErrorSharedOutsideOfASubprogram,
    ErrorGlobalInsideOfASubprogram,
    ErrorExitSubOutsideOfASubprogram,
    ErrorKeyboardNotEnabled,
    ErrorAutomaticPauseNotDisabled,
    ErrorNotAllowedOutsideOfInterrupt
};

struct CoreError {
    enum ErrorCode code;
    int sourcePosition;
};

const char *err_getString(enum ErrorCode errorCode);
struct CoreError err_makeCoreError(enum ErrorCode code, int sourcePosition);
struct CoreError err_noCoreError(void);

#endif /* error_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef data_manager_h
#define data_manager_h

#define MAX_ENTRIES 16
#define DATA_SIZE 0x10000
#define ENTRY_COMMENT_SIZE 32

struct DataEntry {
    char comment[ENTRY_COMMENT_SIZE];
    int start;
    int length;
};

struct DataManager {
    struct DataEntry entries[MAX_ENTRIES];
    uint8_t *data;
    const char *diskSourceCode;
};

void data_init(struct DataManager *manager);
void data_deinit(struct DataManager *manager);
void data_reset(struct DataManager *manager);
struct CoreError data_import(struct DataManager *manager, const char *input, bool keepSourceCode);
struct CoreError data_uppercaseImport(struct DataManager *manager, const char *input, bool keepSourceCode);
char *data_export(struct DataManager *manager);

int data_currentSize(struct DataManager *manager);

bool data_canSetEntry(struct DataManager *manager, int index, int length);
void data_setEntry(struct DataManager *manager, int index, const char *comment, uint8_t *source, int length);

#endif /* data_manager_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef error_h
#define error_h

enum ErrorCode {
    ErrorNone,
    
    ErrorCouldNotOpenProgram,
    ErrorTooManyTokens,
    ErrorRomIsFull,
    ErrorIndexAlreadyDefined,
    ErrorUnterminatedString,
    ErrorUnexpectedCharacter,
    ErrorReservedKeyword,
    ErrorSyntax,
    ErrorSymbolNameTooLong,
    ErrorTooManySymbols,
    ErrorTypeMismatch,
    ErrorOutOfMemory,
    ErrorElseWithoutIf,
    ErrorEndIfWithoutIf,
    ErrorExpectedCommand,
    ErrorNextWithoutFor,
    ErrorLoopWithoutDo,
    ErrorUntilWithoutRepeat,
    ErrorWendWithoutWhile,
    ErrorLabelAlreadyDefined,
    ErrorTooManyLabels,
    ErrorExpectedLabel,
    ErrorUndefinedLabel,
    ErrorArrayNotDimensionized,
    ErrorArrayAlreadyDimensionized,
    ErrorVariableAlreadyUsed,
    ErrorIndexOutOfBounds,
    ErrorWrongNumberOfDimensions,
    ErrorInvalidParameter,
    ErrorReturnWithoutGosub,
    ErrorStackOverflow,
    ErrorOutOfData,
    ErrorIllegalMemoryAccess,
    ErrorTooManyCPUCyclesInInterrupt,
    ErrorNotAllowedInInterrupt,
    ErrorIfWithoutEndIf,
    ErrorForWithoutNext,
    ErrorDoWithoutLoop,
    ErrorRepeatWithoutUntil,
    ErrorWhileWithoutWend,
    ErrorExitNotInsideLoop,
    ErrorDirectoryNotLoaded,
    ErrorDivisionByZero,
    ErrorVariableNotInitialized,
    ErrorArrayVariableWithoutIndex,
    ErrorEndSubWithoutSub,
    ErrorSubWithoutEndSub,
    ErrorSubCannotBeNested,
    ErrorUndefinedSubprogram,
    ErrorExpectedSubprogramName,
    ErrorArgumentCountMismatch,
    ErrorSubAlreadyDefined,
    ErrorTooManySubprograms,
    ErrorSharedOutsideOfASubprogram,
    ErrorGlobalInsideOfASubprogram,
    ErrorExitSubOutsideOfASubprogram,
    ErrorKeyboardNotEnabled,
    ErrorAutomaticPauseNotDisabled,
    ErrorNotAllowedOutsideOfInterrupt
};

struct CoreError {
    enum ErrorCode code;
    int sourcePosition;
};

const char *err_getString(enum ErrorCode errorCode);
struct CoreError err_makeCoreError(enum ErrorCode code, int sourcePosition);
struct CoreError err_noCoreError(void);

#endif /* error_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef core_delegate_h
#define core_delegate_h

struct Core;

enum KeyboardMode {
    KeyboardModeOff,
    KeyboardModeOn,
    KeyboardModeOptional
};

struct ControlsInfo {
    enum KeyboardMode keyboardMode;
    bool isAudioEnabled;
};

struct CoreDelegate {
    void *context;

    /** Called on error */
    void (*interpreterDidFail)(void *context, struct CoreError coreError);

    /** Returns true if the disk is ready, false if not. In case of not, core_diskLoaded must be called when ready. */
    bool (*diskDriveWillAccess)(void *context, struct DataManager *diskDataManager);

    /** Called when a disk data entry was saved */
    void (*diskDriveDidSave)(void *context, struct DataManager *diskDataManager);

    /** Called when a disk data entry was tried to be saved, but the disk is full */
    void (*diskDriveIsFull)(void *context, struct DataManager *diskDataManager);

    /** Called when keyboard or gamepad settings changed */
    void (*controlsDidChange)(void *context, struct ControlsInfo controlsInfo);

    /** Called when persistent RAM will be accessed the first time */
    void (*persistentRamWillAccess)(void *context, uint8_t *destination, int size);

    /** Called when persistent RAM should be saved */
    void (*persistentRamDidChange)(void *context, uint8_t *data, int size);
};

void delegate_interpreterDidFail(struct Core *core, struct CoreError coreError);
bool delegate_diskDriveWillAccess(struct Core *core);
void delegate_diskDriveDidSave(struct Core *core);
void delegate_diskDriveIsFull(struct Core *core);
void delegate_controlsDidChange(struct Core *core);
void delegate_persistentRamWillAccess(struct Core *core, uint8_t *destination, int size);
void delegate_persistentRamDidChange(struct Core *core, uint8_t *data, int size);

#endif /* core_delegate_h */

//
// Copyright 2016-2020 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef core_h
#define core_h

#define CORE_VERSION "2.0"

struct Core
{
	struct Machine *machine;
	struct MachineInternals *machineInternals;
	struct Interpreter *interpreter;
	struct DiskDrive *diskDrive;
	struct Overlay *overlay;
	struct CoreDelegate *delegate;
};

struct CoreInput
{
	// For SHOWN and SAFE
	int width, height, left, top, right, bottom;
	bool pause;
	float touchX;
	float touchY;
	bool touch;
	char key;
	bool out_hasUsedInput;
};

extern const char CoreInputKeyReturn;
extern const char CoreInputKeyBackspace;
extern const char CoreInputKeyRight;
extern const char CoreInputKeyLeft;
extern const char CoreInputKeyDown;
extern const char CoreInputKeyUp;

void core_init(struct Core *core);
void core_deinit(struct Core *core);
void core_setDelegate(struct Core *core, struct CoreDelegate *delegate);
struct CoreError core_compileProgram(struct Core *core, const char *sourceCode, bool resetPersistent);
void core_traceError(struct Core *core, struct CoreError error);
void core_willRunProgram(struct Core *core, long secondsSincePowerOn);
void core_update(struct Core *core, struct CoreInput *input);
void core_willSuspendProgram(struct Core *core);
void core_setDebug(struct Core *core, bool enabled);
bool core_getDebug(struct Core *core);
bool core_isKeyboardEnabled(struct Core *core);
void core_setKeybordEnabled(struct Core *core, bool enabled);
bool core_shouldRender(struct Core *core);

void core_setInputGamepad(struct CoreInput *input, int player, bool up, bool down, bool left, bool right, bool buttonA, bool buttonB);

void core_diskLoaded(struct Core *core);

// for dev mode only:
void core_handleInput(struct Core *core, struct CoreInput *input);

#endif /* core_h */

//
// Copyright 2017-2018 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef boot_intro_h
#define boot_intro_h

extern const int bootIntroStateAddress;
extern const char *bootIntroSourceCode;

enum BootIntroState {
    BootIntroStateDefault,
    BootIntroStateProgramAvailable,
    BootIntroStateReadyToRun,
    BootIntroStateDone
};

#endif /* boot_intro_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef error_h
#define error_h

enum ErrorCode {
    ErrorNone,
    
    ErrorCouldNotOpenProgram,
    ErrorTooManyTokens,
    ErrorRomIsFull,
    ErrorIndexAlreadyDefined,
    ErrorUnterminatedString,
    ErrorUnexpectedCharacter,
    ErrorReservedKeyword,
    ErrorSyntax,
    ErrorSymbolNameTooLong,
    ErrorTooManySymbols,
    ErrorTypeMismatch,
    ErrorOutOfMemory,
    ErrorElseWithoutIf,
    ErrorEndIfWithoutIf,
    ErrorExpectedCommand,
    ErrorNextWithoutFor,
    ErrorLoopWithoutDo,
    ErrorUntilWithoutRepeat,
    ErrorWendWithoutWhile,
    ErrorLabelAlreadyDefined,
    ErrorTooManyLabels,
    ErrorExpectedLabel,
    ErrorUndefinedLabel,
    ErrorArrayNotDimensionized,
    ErrorArrayAlreadyDimensionized,
    ErrorVariableAlreadyUsed,
    ErrorIndexOutOfBounds,
    ErrorWrongNumberOfDimensions,
    ErrorInvalidParameter,
    ErrorReturnWithoutGosub,
    ErrorStackOverflow,
    ErrorOutOfData,
    ErrorIllegalMemoryAccess,
    ErrorTooManyCPUCyclesInInterrupt,
    ErrorNotAllowedInInterrupt,
    ErrorIfWithoutEndIf,
    ErrorForWithoutNext,
    ErrorDoWithoutLoop,
    ErrorRepeatWithoutUntil,
    ErrorWhileWithoutWend,
    ErrorExitNotInsideLoop,
    ErrorDirectoryNotLoaded,
    ErrorDivisionByZero,
    ErrorVariableNotInitialized,
    ErrorArrayVariableWithoutIndex,
    ErrorEndSubWithoutSub,
    ErrorSubWithoutEndSub,
    ErrorSubCannotBeNested,
    ErrorUndefinedSubprogram,
    ErrorExpectedSubprogramName,
    ErrorArgumentCountMismatch,
    ErrorSubAlreadyDefined,
    ErrorTooManySubprograms,
    ErrorSharedOutsideOfASubprogram,
    ErrorGlobalInsideOfASubprogram,
    ErrorExitSubOutsideOfASubprogram,
    ErrorKeyboardNotEnabled,
    ErrorAutomaticPauseNotDisabled,
    ErrorNotAllowedOutsideOfInterrupt
};

struct CoreError {
    enum ErrorCode code;
    int sourcePosition;
};

const char *err_getString(enum ErrorCode errorCode);
struct CoreError err_makeCoreError(enum ErrorCode code, int sourcePosition);
struct CoreError err_noCoreError(void);

#endif /* error_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef data_manager_h
#define data_manager_h

#define MAX_ENTRIES 16
#define DATA_SIZE 0x10000
#define ENTRY_COMMENT_SIZE 32

struct DataEntry {
    char comment[ENTRY_COMMENT_SIZE];
    int start;
    int length;
};

struct DataManager {
    struct DataEntry entries[MAX_ENTRIES];
    uint8_t *data;
    const char *diskSourceCode;
};

void data_init(struct DataManager *manager);
void data_deinit(struct DataManager *manager);
void data_reset(struct DataManager *manager);
struct CoreError data_import(struct DataManager *manager, const char *input, bool keepSourceCode);
struct CoreError data_uppercaseImport(struct DataManager *manager, const char *input, bool keepSourceCode);
char *data_export(struct DataManager *manager);

int data_currentSize(struct DataManager *manager);

bool data_canSetEntry(struct DataManager *manager, int index, int length);
void data_setEntry(struct DataManager *manager, int index, const char *comment, uint8_t *source, int length);

#endif /* data_manager_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef error_h
#define error_h

enum ErrorCode {
    ErrorNone,
    
    ErrorCouldNotOpenProgram,
    ErrorTooManyTokens,
    ErrorRomIsFull,
    ErrorIndexAlreadyDefined,
    ErrorUnterminatedString,
    ErrorUnexpectedCharacter,
    ErrorReservedKeyword,
    ErrorSyntax,
    ErrorSymbolNameTooLong,
    ErrorTooManySymbols,
    ErrorTypeMismatch,
    ErrorOutOfMemory,
    ErrorElseWithoutIf,
    ErrorEndIfWithoutIf,
    ErrorExpectedCommand,
    ErrorNextWithoutFor,
    ErrorLoopWithoutDo,
    ErrorUntilWithoutRepeat,
    ErrorWendWithoutWhile,
    ErrorLabelAlreadyDefined,
    ErrorTooManyLabels,
    ErrorExpectedLabel,
    ErrorUndefinedLabel,
    ErrorArrayNotDimensionized,
    ErrorArrayAlreadyDimensionized,
    ErrorVariableAlreadyUsed,
    ErrorIndexOutOfBounds,
    ErrorWrongNumberOfDimensions,
    ErrorInvalidParameter,
    ErrorReturnWithoutGosub,
    ErrorStackOverflow,
    ErrorOutOfData,
    ErrorIllegalMemoryAccess,
    ErrorTooManyCPUCyclesInInterrupt,
    ErrorNotAllowedInInterrupt,
    ErrorIfWithoutEndIf,
    ErrorForWithoutNext,
    ErrorDoWithoutLoop,
    ErrorRepeatWithoutUntil,
    ErrorWhileWithoutWend,
    ErrorExitNotInsideLoop,
    ErrorDirectoryNotLoaded,
    ErrorDivisionByZero,
    ErrorVariableNotInitialized,
    ErrorArrayVariableWithoutIndex,
    ErrorEndSubWithoutSub,
    ErrorSubWithoutEndSub,
    ErrorSubCannotBeNested,
    ErrorUndefinedSubprogram,
    ErrorExpectedSubprogramName,
    ErrorArgumentCountMismatch,
    ErrorSubAlreadyDefined,
    ErrorTooManySubprograms,
    ErrorSharedOutsideOfASubprogram,
    ErrorGlobalInsideOfASubprogram,
    ErrorExitSubOutsideOfASubprogram,
    ErrorKeyboardNotEnabled,
    ErrorAutomaticPauseNotDisabled,
    ErrorNotAllowedOutsideOfInterrupt
};

struct CoreError {
    enum ErrorCode code;
    int sourcePosition;
};

const char *err_getString(enum ErrorCode errorCode);
struct CoreError err_makeCoreError(enum ErrorCode code, int sourcePosition);
struct CoreError err_noCoreError(void);

#endif /* error_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef core_delegate_h
#define core_delegate_h

struct Core;

enum KeyboardMode {
    KeyboardModeOff,
    KeyboardModeOn,
    KeyboardModeOptional
};

struct ControlsInfo {
    enum KeyboardMode keyboardMode;
    bool isAudioEnabled;
};

struct CoreDelegate {
    void *context;

    /** Called on error */
    void (*interpreterDidFail)(void *context, struct CoreError coreError);

    /** Returns true if the disk is ready, false if not. In case of not, core_diskLoaded must be called when ready. */
    bool (*diskDriveWillAccess)(void *context, struct DataManager *diskDataManager);

    /** Called when a disk data entry was saved */
    void (*diskDriveDidSave)(void *context, struct DataManager *diskDataManager);

    /** Called when a disk data entry was tried to be saved, but the disk is full */
    void (*diskDriveIsFull)(void *context, struct DataManager *diskDataManager);

    /** Called when keyboard or gamepad settings changed */
    void (*controlsDidChange)(void *context, struct ControlsInfo controlsInfo);

    /** Called when persistent RAM will be accessed the first time */
    void (*persistentRamWillAccess)(void *context, uint8_t *destination, int size);

    /** Called when persistent RAM should be saved */
    void (*persistentRamDidChange)(void *context, uint8_t *data, int size);
};

void delegate_interpreterDidFail(struct Core *core, struct CoreError coreError);
bool delegate_diskDriveWillAccess(struct Core *core);
void delegate_diskDriveDidSave(struct Core *core);
void delegate_diskDriveIsFull(struct Core *core);
void delegate_controlsDidChange(struct Core *core);
void delegate_persistentRamWillAccess(struct Core *core, uint8_t *destination, int size);
void delegate_persistentRamDidChange(struct Core *core, uint8_t *data, int size);

#endif /* core_delegate_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef error_h
#define error_h

enum ErrorCode {
    ErrorNone,
    
    ErrorCouldNotOpenProgram,
    ErrorTooManyTokens,
    ErrorRomIsFull,
    ErrorIndexAlreadyDefined,
    ErrorUnterminatedString,
    ErrorUnexpectedCharacter,
    ErrorReservedKeyword,
    ErrorSyntax,
    ErrorSymbolNameTooLong,
    ErrorTooManySymbols,
    ErrorTypeMismatch,
    ErrorOutOfMemory,
    ErrorElseWithoutIf,
    ErrorEndIfWithoutIf,
    ErrorExpectedCommand,
    ErrorNextWithoutFor,
    ErrorLoopWithoutDo,
    ErrorUntilWithoutRepeat,
    ErrorWendWithoutWhile,
    ErrorLabelAlreadyDefined,
    ErrorTooManyLabels,
    ErrorExpectedLabel,
    ErrorUndefinedLabel,
    ErrorArrayNotDimensionized,
    ErrorArrayAlreadyDimensionized,
    ErrorVariableAlreadyUsed,
    ErrorIndexOutOfBounds,
    ErrorWrongNumberOfDimensions,
    ErrorInvalidParameter,
    ErrorReturnWithoutGosub,
    ErrorStackOverflow,
    ErrorOutOfData,
    ErrorIllegalMemoryAccess,
    ErrorTooManyCPUCyclesInInterrupt,
    ErrorNotAllowedInInterrupt,
    ErrorIfWithoutEndIf,
    ErrorForWithoutNext,
    ErrorDoWithoutLoop,
    ErrorRepeatWithoutUntil,
    ErrorWhileWithoutWend,
    ErrorExitNotInsideLoop,
    ErrorDirectoryNotLoaded,
    ErrorDivisionByZero,
    ErrorVariableNotInitialized,
    ErrorArrayVariableWithoutIndex,
    ErrorEndSubWithoutSub,
    ErrorSubWithoutEndSub,
    ErrorSubCannotBeNested,
    ErrorUndefinedSubprogram,
    ErrorExpectedSubprogramName,
    ErrorArgumentCountMismatch,
    ErrorSubAlreadyDefined,
    ErrorTooManySubprograms,
    ErrorSharedOutsideOfASubprogram,
    ErrorGlobalInsideOfASubprogram,
    ErrorExitSubOutsideOfASubprogram,
    ErrorKeyboardNotEnabled,
    ErrorAutomaticPauseNotDisabled,
    ErrorNotAllowedOutsideOfInterrupt
};

struct CoreError {
    enum ErrorCode code;
    int sourcePosition;
};

const char *err_getString(enum ErrorCode errorCode);
struct CoreError err_makeCoreError(enum ErrorCode code, int sourcePosition);
struct CoreError err_noCoreError(void);

#endif /* error_h */

//
// Copyright 2017-2020 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef interpreter_config_h
#define interpreter_config_h

#define MAX_TOKENS 16384
#define MAX_SYMBOLS 2048
#define MAX_LABEL_STACK_ITEMS 128
#define MAX_JUMP_LABEL_ITEMS 256
#define MAX_SUB_ITEMS 256
#define MAX_SIMPLE_VARIABLES 256
#define MAX_ARRAY_VARIABLES 256
#define SYMBOL_NAME_SIZE 21
#define MAX_ARRAY_DIMENSIONS 4
#define MAX_ARRAY_SIZE 32768
// XXX: #define MAX_CYCLES_TOTAL_PER_FRAME 17556
// XXX: #define MAX_CYCLES_PER_VBL 1140
// XXX: #define MAX_CYCLES_PER_RASTER 51
#define MAX_CYCLES_TOTAL_PER_FRAME 35112 // ??
#define MAX_CYCLES_PER_VBL 3420 // 1140*3 ??
#define MAX_CYCLES_PER_RASTER 204 // 51*4 OK
#define MAX_CYCLES_PER_PARTICLE 51 // ??
#define MAX_CYCLES_PER_EMITTER 102 // ??
#define TIMER_WRAP_VALUE 5184000

#endif /* interpreter_config_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef string_h
#define string_h

struct RCString {
    int refCount;
    char chars[1]; // ...
};

struct RCString *rcstring_new(const char *chars, size_t len);
void rcstring_retain(struct RCString *string);
void rcstring_release(struct RCString *string);

#endif /* string_h */

//
// Copyright 2017-2020 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef token_h
#define token_h

enum TokenType {
    TokenUndefined,

    TokenIdentifier,
    TokenStringIdentifier,
    TokenLabel,
    TokenFloat,
    TokenString,

    // Signs
    TokenColon,
    TokenComma,
    TokenSemicolon,
    TokenApostrophe,
    TokenEol,

    // Operators
    TokenEq,
    TokenGrEq,
    TokenLeEq,
    TokenUneq,
    TokenGr,
    TokenLe,
    TokenBracketOpen,
    TokenBracketClose,
    TokenPlus,
    TokenMinus,
    TokenMul,
    TokenDiv,
    TokenDivInt,
    TokenPow,
    TokenAND,
    TokenNOT,
    TokenOR,
    TokenXOR,
    TokenMOD,

    // Commands/Functions
    TokenABS,
    TokenACOS,
    TokenADD,
    TokenASC,
    TokenASIN,
    TokenATAN,
    TokenATTR,
    TokenBG,
    TokenBIN,
    // TokenBUTTON,
    TokenCALL,
    TokenCELLA,
    TokenCELLC,
    TokenCELL,
    TokenCHAR,
    TokenCHR,
    TokenCLS,
    TokenCLW,
    TokenCOLOR,
    TokenCOPY,
    TokenCOS,
    TokenCURSORX,
    TokenCURSORY,
    TokenDATA,
    TokenDEC,
    TokenDIM,
    // TokenDISPLAY,
    //TokenDOWN,
    TokenDO,
    TokenELSE,
    TokenEND,
    TokenENVELOPE,
    TokenEXIT,
    TokenEXP,
    TokenFILE,
    TokenFILES,
    TokenFILL,
    TokenFLIP,
    TokenFONT,
    TokenFOR,
    TokenFSIZE,
    //TokenGAMEPAD,
    TokenGLOBAL,
    TokenGOSUB,
    TokenGOTO,
    TokenHEX,
    TokenHCOS,
    TokenHIT,
    TokenHSIN,
    TokenHTAN,
    TokenIF,
    TokenINC,
    TokenINKEY,
    TokenINPUT,
    TokenINSTR,
    TokenINT,
    TokenKEYBOARD,
    TokenLEFTStr,
    //TokenLEFT,
    TokenLEN,
    TokenLET,
    TokenLFOA,
    TokenLFO,
    TokenLOAD,
    TokenLOCATE,
    TokenLOG,
    TokenLOOP,
    TokenMAX,
    TokenMCELLA,
    TokenMCELLC,
    TokenMCELL,
    TokenMID,
    TokenMIN,
    TokenCLAMP,
    TokenMUSIC,
    TokenNEXT,
    TokenNUMBER,
    TokenOFF,
    TokenON,
    TokenOPTIONAL,
    TokenPALETTE,
    TokenPAL,
    TokenPAUSE,
    TokenPEEKL,
    TokenPEEKW,
    TokenPEEK,
    TokenPI,
    TokenPLAY,
    TokenPOKEL,
    TokenPOKEW,
    TokenPOKE,
    TokenPRINT,
    TokenPRIO,
    TokenRANDOMIZE,
    TokenRASTER,
    TokenREAD,
    TokenSKIP,
    TokenREM,
    TokenREPEAT,
    TokenRESTORE,
    TokenRETURN,
    TokenRIGHTStr,
    //TokenRIGHT,
    TokenRND,
    TokenROL,
    TokenROM,
    TokenROR,
    TokenSAVE,
    TokenSCROLLX,
    TokenSCROLLY,
    TokenSCROLL,
    TokenSGN,
    TokenSIN,
    TokenSIZE,
    TokenSOUND,
    TokenSOURCE,
    TokenSPRITEA,
    TokenSPRITEC,
    TokenSPRITEX,
    TokenSPRITEY,
    TokenSPRITE,
    TokenSQR,
    TokenSTEP,
    TokenSTOP,
    TokenSTR,
    TokenSUB,
    TokenSWAP,
    TokenSYSTEM,
    TokenTAN,
    TokenTAP,
    TokenTEXT,
    TokenTHEN,
    TokenTIMER,
    TokenTINT,
    //TokenTOUCHSCREEN,
    TokenTOUCHX,
    TokenTOUCHY,
    TokenTOUCH,
    TokenTO,
    TokenTRACE,
    TokenTRACK,
    TokenUBOUND,
    TokenUNTIL,
    //TokenUP,
    TokenVAL,
    TokenVBL,
    TokenVIEW,
    TokenVOLUME,
    TokenWAIT,
    TokenWAVE,
    TokenWEND,
    TokenWHILE,
    TokenWINDOW,

    TokenSHOWNW,
    TokenSHOWNH,
    TokenSAFEL,
    TokenSAFET,
    TokenSAFER,
    TokenSAFEB,

    TokenPARTICLE,
    TokenEMITTER,
    TokenAT,
    TokenCOMPAT,
    TokenEASE,
    TokenMESSAGE,
    TokenDMA,
		TokenCEIL,

    // Reserved Keywords
    Token_reserved,
    // TokenANIM,
    // TokenCLOSE,
    // TokenDECLARE,
    // TokenDEF,
    // TokenFLASH,
    // TokenFN,
    // TokenFUNCTION,
    // TokenLBOUND,
    // TokenOPEN,
    // TokenOUTPUT,
    // TokenSHARED,
    // TokenSTATIC,
    // TokenTEMPO,
    // TokenVOICE,
    // TokenWRITE,

    Token_count
};

struct Token {
    enum TokenType type;
    union {
        float floatValue;
        struct RCString *stringValue;
        int symbolIndex;
        struct Token *jumpToken;
    };
    int sourcePosition;
};

extern const char *TokenStrings[];

#endif /* token_h */

//
// Copyright 2016-2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef tokenizer_h
#define tokenizer_h

struct Symbol {
    char name[SYMBOL_NAME_SIZE];
};

struct JumpLabelItem {
    int symbolIndex;
    struct Token *token;
};

struct SubItem {
    int symbolIndex;
    struct Token *token;
};

struct Tokenizer
{
    struct Token tokens[MAX_TOKENS];
    int numTokens;
    struct Symbol symbols[MAX_SYMBOLS];
    int numSymbols;

    struct JumpLabelItem jumpLabelItems[MAX_JUMP_LABEL_ITEMS];
    int numJumpLabelItems;
    struct SubItem subItems[MAX_SUB_ITEMS];
    int numSubItems;
};

struct CoreError tok_tokenizeProgram(struct Tokenizer *tokenizer, const char *sourceCode);
struct CoreError tok_tokenizeUppercaseProgram(struct Tokenizer *tokenizer, const char *sourceCode);
void tok_freeTokens(struct Tokenizer *tokenizer);
struct JumpLabelItem *tok_getJumpLabel(struct Tokenizer *tokenizer, int symbolIndex);
enum ErrorCode tok_setJumpLabel(struct Tokenizer *tokenizer, int symbolIndex, struct Token *token);
struct SubItem *tok_getSub(struct Tokenizer *tokenizer, int symbolIndex);
enum ErrorCode tok_setSub(struct Tokenizer *tokenizer, int symbolIndex, struct Token *token);

#endif /* tokenizer_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef error_h
#define error_h

enum ErrorCode {
    ErrorNone,
    
    ErrorCouldNotOpenProgram,
    ErrorTooManyTokens,
    ErrorRomIsFull,
    ErrorIndexAlreadyDefined,
    ErrorUnterminatedString,
    ErrorUnexpectedCharacter,
    ErrorReservedKeyword,
    ErrorSyntax,
    ErrorSymbolNameTooLong,
    ErrorTooManySymbols,
    ErrorTypeMismatch,
    ErrorOutOfMemory,
    ErrorElseWithoutIf,
    ErrorEndIfWithoutIf,
    ErrorExpectedCommand,
    ErrorNextWithoutFor,
    ErrorLoopWithoutDo,
    ErrorUntilWithoutRepeat,
    ErrorWendWithoutWhile,
    ErrorLabelAlreadyDefined,
    ErrorTooManyLabels,
    ErrorExpectedLabel,
    ErrorUndefinedLabel,
    ErrorArrayNotDimensionized,
    ErrorArrayAlreadyDimensionized,
    ErrorVariableAlreadyUsed,
    ErrorIndexOutOfBounds,
    ErrorWrongNumberOfDimensions,
    ErrorInvalidParameter,
    ErrorReturnWithoutGosub,
    ErrorStackOverflow,
    ErrorOutOfData,
    ErrorIllegalMemoryAccess,
    ErrorTooManyCPUCyclesInInterrupt,
    ErrorNotAllowedInInterrupt,
    ErrorIfWithoutEndIf,
    ErrorForWithoutNext,
    ErrorDoWithoutLoop,
    ErrorRepeatWithoutUntil,
    ErrorWhileWithoutWend,
    ErrorExitNotInsideLoop,
    ErrorDirectoryNotLoaded,
    ErrorDivisionByZero,
    ErrorVariableNotInitialized,
    ErrorArrayVariableWithoutIndex,
    ErrorEndSubWithoutSub,
    ErrorSubWithoutEndSub,
    ErrorSubCannotBeNested,
    ErrorUndefinedSubprogram,
    ErrorExpectedSubprogramName,
    ErrorArgumentCountMismatch,
    ErrorSubAlreadyDefined,
    ErrorTooManySubprograms,
    ErrorSharedOutsideOfASubprogram,
    ErrorGlobalInsideOfASubprogram,
    ErrorExitSubOutsideOfASubprogram,
    ErrorKeyboardNotEnabled,
    ErrorAutomaticPauseNotDisabled,
    ErrorNotAllowedOutsideOfInterrupt
};

struct CoreError {
    enum ErrorCode code;
    int sourcePosition;
};

const char *err_getString(enum ErrorCode errorCode);
struct CoreError err_makeCoreError(enum ErrorCode code, int sourcePosition);
struct CoreError err_noCoreError(void);

#endif /* error_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef data_manager_h
#define data_manager_h

#define MAX_ENTRIES 16
#define DATA_SIZE 0x10000
#define ENTRY_COMMENT_SIZE 32

struct DataEntry {
    char comment[ENTRY_COMMENT_SIZE];
    int start;
    int length;
};

struct DataManager {
    struct DataEntry entries[MAX_ENTRIES];
    uint8_t *data;
    const char *diskSourceCode;
};

void data_init(struct DataManager *manager);
void data_deinit(struct DataManager *manager);
void data_reset(struct DataManager *manager);
struct CoreError data_import(struct DataManager *manager, const char *input, bool keepSourceCode);
struct CoreError data_uppercaseImport(struct DataManager *manager, const char *input, bool keepSourceCode);
char *data_export(struct DataManager *manager);

int data_currentSize(struct DataManager *manager);

bool data_canSetEntry(struct DataManager *manager, int index, int length);
void data_setEntry(struct DataManager *manager, int index, const char *comment, uint8_t *source, int length);

#endif /* data_manager_h */

//
// Copyright 2020 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef core_stats_h
#define core_stats_h

struct Stats {
    struct Tokenizer *tokenizer;
    struct DataManager *romDataManager;
    int numTokens;
    int romSize;
};

void stats_init(struct Stats *stats);
void stats_deinit(struct Stats *stats);
struct CoreError stats_update(struct Stats *stats, const char *sourceCode);

#endif /* core_stats_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef string_utils_h
#define string_utils_h

const char *uppercaseString(const char *source);
const char *lineString(const char *source, int pos);
int lineNumber(const char *source, int pos);
void stringConvertCopy(char *dest, const char *source, size_t length);

#endif /* string_utils_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef startup_sequence_h
#define startup_sequence_h

struct Core;

void runStartupSequence(struct Core *core);

#endif /* startup_sequence_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef charsets_h
#define charsets_h

extern const char *CharSetDigits;
extern const char *CharSetLetters;
extern const char *CharSetAlphaNum;
extern const char *CharSetHex;

#endif /* charsets_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef error_h
#define error_h

enum ErrorCode {
    ErrorNone,
    
    ErrorCouldNotOpenProgram,
    ErrorTooManyTokens,
    ErrorRomIsFull,
    ErrorIndexAlreadyDefined,
    ErrorUnterminatedString,
    ErrorUnexpectedCharacter,
    ErrorReservedKeyword,
    ErrorSyntax,
    ErrorSymbolNameTooLong,
    ErrorTooManySymbols,
    ErrorTypeMismatch,
    ErrorOutOfMemory,
    ErrorElseWithoutIf,
    ErrorEndIfWithoutIf,
    ErrorExpectedCommand,
    ErrorNextWithoutFor,
    ErrorLoopWithoutDo,
    ErrorUntilWithoutRepeat,
    ErrorWendWithoutWhile,
    ErrorLabelAlreadyDefined,
    ErrorTooManyLabels,
    ErrorExpectedLabel,
    ErrorUndefinedLabel,
    ErrorArrayNotDimensionized,
    ErrorArrayAlreadyDimensionized,
    ErrorVariableAlreadyUsed,
    ErrorIndexOutOfBounds,
    ErrorWrongNumberOfDimensions,
    ErrorInvalidParameter,
    ErrorReturnWithoutGosub,
    ErrorStackOverflow,
    ErrorOutOfData,
    ErrorIllegalMemoryAccess,
    ErrorTooManyCPUCyclesInInterrupt,
    ErrorNotAllowedInInterrupt,
    ErrorIfWithoutEndIf,
    ErrorForWithoutNext,
    ErrorDoWithoutLoop,
    ErrorRepeatWithoutUntil,
    ErrorWhileWithoutWend,
    ErrorExitNotInsideLoop,
    ErrorDirectoryNotLoaded,
    ErrorDivisionByZero,
    ErrorVariableNotInitialized,
    ErrorArrayVariableWithoutIndex,
    ErrorEndSubWithoutSub,
    ErrorSubWithoutEndSub,
    ErrorSubCannotBeNested,
    ErrorUndefinedSubprogram,
    ErrorExpectedSubprogramName,
    ErrorArgumentCountMismatch,
    ErrorSubAlreadyDefined,
    ErrorTooManySubprograms,
    ErrorSharedOutsideOfASubprogram,
    ErrorGlobalInsideOfASubprogram,
    ErrorExitSubOutsideOfASubprogram,
    ErrorKeyboardNotEnabled,
    ErrorAutomaticPauseNotDisabled,
    ErrorNotAllowedOutsideOfInterrupt
};

struct CoreError {
    enum ErrorCode code;
    int sourcePosition;
};

const char *err_getString(enum ErrorCode errorCode);
struct CoreError err_makeCoreError(enum ErrorCode code, int sourcePosition);
struct CoreError err_noCoreError(void);

#endif /* error_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef string_h
#define string_h

struct RCString {
    int refCount;
    char chars[1]; // ...
};

struct RCString *rcstring_new(const char *chars, size_t len);
void rcstring_retain(struct RCString *string);
void rcstring_release(struct RCString *string);

#endif /* string_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef value_h
#define value_h

enum ValueType {
    ValueTypeNull,
    ValueTypeError,
    ValueTypeFloat,
    ValueTypeString
};

union Value {
    float floatValue;
    struct RCString *stringValue;
    union Value *reference;
    enum ErrorCode errorCode;
};

struct TypedValue {
    enum ValueType type;
    union Value v;
};

enum TypeClass {
    TypeClassAny,
    TypeClassNumeric,
    TypeClassString
};

extern union Value ValueDummy;

struct TypedValue val_makeError(enum ErrorCode errorCode);

#endif /* value_h */

//
// Copyright 2016 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef video_chip_h
#define video_chip_h

#define UNIVERSAL_WIDTH 177
#define UNIVERSAL_HEIGHT 288
#define ICON_WIDTH 180 // 22.5
#define ICON_HEIGHT 180 // 22.5
#define SCREEN_WIDTH 216 // 27x8
#define SCREEN_HEIGHT 384 // 48x8
#define NUM_CHARACTERS 256
#define NUM_PALETTES 8
#define PLANE_COLUMNS 64
#define PLANE_ROWS 64
#define NUM_SPRITES 170
#define SPRITE_OFFSET_X 32
#define SPRITE_OFFSET_Y 32

#define COLS_MASK (PLANE_COLUMNS-1)
#define ROWS_MASK (PLANE_ROWS-1)

#if __APPLE__
#if TARGET_OS_IPHONE
#define BGR 1
#endif
#endif

struct Core;

// ================ Character ================

// 16 bytes
struct Character {
    uint8_t data[16];
};

// ================ Sprite ================

union CharacterAttributes {
    struct {
        uint8_t palette:3;
        uint8_t flipX:1;
        uint8_t flipY:1;
        uint8_t priority:1;
        uint8_t size:2; // 1x1 - 4x4 characters
    };
    uint8_t value;
};

// 6 bytes
struct Sprite {
    uint16_t x;
    uint16_t y;
    uint8_t character;
    union CharacterAttributes attr;
};

// ================ Cell ================

// 2 bytes
struct Cell {
    uint8_t character;
    union CharacterAttributes attr;
};

// ================ Plane ================

// 8 Kibi
struct Plane {
    struct Cell cells[PLANE_ROWS][PLANE_COLUMNS];
};

// ===========================================
// ================ Video RAM ================
// ===========================================

// 36Kibi
struct VideoRam {
    struct Plane planeA; // 8Kibi
    struct Plane planeB; // 8Kibi
    struct Plane planeC; // 8Kibi
    struct Plane planeD; // 8Kibi
    struct Character characters[NUM_CHARACTERS]; // 4Kibi
};

// =================================================
// ================ Video Registers ================
// =================================================

struct SpriteRegisters {
    struct Sprite sprites[NUM_SPRITES]; // 256 bytes
};

struct ColorRegisters {
    uint8_t colors[NUM_PALETTES * 4]; // 32 bytes
};

union DisplayAttributes {
    struct {
        uint8_t spritesEnabled:1;
        uint8_t planeAEnabled:1;
        uint8_t planeBEnabled:1;
        uint8_t planeCEnabled:1;
        uint8_t planeDEnabled:1;
        // uint8_t planeACellSize:1;
        // uint8_t planeBCellSize:1;
        // uint8_t planeCCellSize:1;
        // uint8_t planeDCellSize:1;
    };
    uint8_t value;
};

// union ScrollMSB {
//     struct {
//         uint8_t aX:1;
//         uint8_t aY:1;
//         uint8_t bX:1;
//         uint8_t bY:1;
//         uint8_t cX:1;
//         uint8_t cY:1;
//         uint8_t dX:1;
//         uint8_t dY:1;
//     };
//     uint8_t value;
// };

struct VideoRegisters {
    uint16_t scrollAX;
    uint16_t scrollAY;
    uint16_t scrollBX;
    uint16_t scrollBY;
    uint16_t scrollCX;
    uint16_t scrollCY;
    uint16_t scrollDX;
    uint16_t scrollDY;
    // union ScrollMSB scrollMSB;
    uint16_t rasterLine;
    union DisplayAttributes attr;
};

// ===========================================
// ================ Functions ================
// ===========================================

void video_renderScreen(struct Core *core, uint32_t *outputRGB);

#endif /* video_chip_h */

//
// Copyright 2016-2020 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef audio_chip_h
#define audio_chip_h

#define NUM_VOICES 4
#define NUM_AUDIO_BUFFERS 6
#define AUDIO_FILTER_BUFFER_SIZE 3

// audio output channels for stereo
#define NUM_CHANNELS 2

struct Core;

enum WaveType {
    WaveTypeSawtooth,
    WaveTypeTriangle,
    WaveTypePulse,
    WaveTypeNoise
};

enum EnvState {
    EnvStateAttack,
    EnvStateDecay,
    EnvStateRelease
};

union VoiceStatus {
    struct {
        uint8_t volume:4;
        uint8_t mix:2;
        uint8_t init:1;
        uint8_t gate:1;
    };
    uint8_t value;
};

union VoiceAttributes {
    struct {
        uint8_t pulseWidth:4;
        uint8_t wave:2;
        uint8_t timeout:1;
    };
    uint8_t value;
};

enum LFOWaveType {
    LFOWaveTypeTriangle,
    LFOWaveTypeSawtooth,
    LFOWaveTypeSquare,
    LFOWaveTypeRandom
};

union LFOAttributes {
    struct {
        uint8_t wave:2;
        uint8_t invert:1;
        uint8_t envMode:1;
        uint8_t trigger:1;
    };
    uint8_t value;
};

struct Voice {
    uint8_t frequencyLow;
    uint8_t frequencyHigh;
    union VoiceStatus status;
    uint8_t peak;
    union VoiceAttributes attr;
    uint8_t length;
    struct {
        uint8_t envA:4;
        uint8_t envD:4;
    };
    struct {
        uint8_t envS:4;
        uint8_t envR:4;
    };
    union LFOAttributes lfoAttr;
    struct {
        uint8_t lfoFrequency:4;
        uint8_t lfoOscAmount:4;
    };
    struct {
        uint8_t lfoVolAmount:4;
        uint8_t lfoPWAmount:4;
    };
    uint8_t reserved2;
};

struct AudioRegisters {
    struct Voice voices[NUM_VOICES];
};

struct VoiceInternals {
    double accumulator;
    uint16_t noiseRandom;
    double envCounter;
    enum EnvState envState;
    double lfoAccumulator;
    bool lfoHold;
    uint16_t lfoRandom;
    double timeoutCounter;
};

struct AudioInternals {
    struct VoiceInternals voices[NUM_VOICES];
    struct AudioRegisters buffers[NUM_AUDIO_BUFFERS];
    int readBufferIndex;
    int writeBufferIndex;
    bool audioEnabled;
    int32_t filterBuffer[NUM_CHANNELS][AUDIO_FILTER_BUFFER_SIZE];
};

void audio_reset(struct Core *core);
void audio_bufferRegisters(struct Core *core);
void audio_renderAudio(struct Core *core, int16_t *output, int numSamples, int outputFrequency, int volume);

#endif /* audio_chip_h */

//
// Copyright 2017-2018 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef interpreter_utils_h
#define interpreter_utils_h

struct Core;

struct SimpleAttributes
{
    int pal;
    int flipX;
    int flipY;
    int prio;
    int size;
};

enum ErrorCode itp_evaluateSimpleAttributes(struct Core *core, struct SimpleAttributes *attrs);

struct TypedValue itp_evaluateCharAttributes(struct Core *core, union CharacterAttributes oldAttr);
//struct TypedValue itp_evaluateDisplayAttributes(struct Core *core, union DisplayAttributes oldAttr);
struct TypedValue itp_evaluateLFOAttributes(struct Core *core, union LFOAttributes oldAttr);

#endif /* interpreter_utils_h */

//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef error_h
#define error_h

enum ErrorCode {
    ErrorNone,
    
    ErrorCouldNotOpenProgram,
    ErrorTooManyTokens,
    ErrorRomIsFull,
    ErrorIndexAlreadyDefined,
    ErrorUnterminatedString,
    ErrorUnexpectedCharacter,
    ErrorReservedKeyword,
    ErrorSyntax,
    ErrorSymbolNameTooLong,
    ErrorTooManySymbols,
    ErrorTypeMismatch,
    ErrorOutOfMemory,
    ErrorElseWithoutIf,
    ErrorEndIfWithoutIf,
    ErrorExpectedCommand,
    ErrorNextWithoutFor,
    ErrorLoopWithoutDo,
    ErrorUntilWithoutRepeat,
    ErrorWendWithoutWhile,
    ErrorLabelAlreadyDefined,
    ErrorTooManyLabels,
    ErrorExpectedLabel,
    ErrorUndefinedLabel,
    ErrorArrayNotDimensionized,
    ErrorArrayAlreadyDimensionized,
    ErrorVariableAlreadyUsed,
    ErrorIndexOutOfBounds,
    ErrorWrongNumberOfDimensions,
    ErrorInvalidParameter,
    ErrorReturnWithoutGosub,
    ErrorStackOverflow,
    ErrorOutOfData,
    ErrorIllegalMemoryAccess,
    ErrorTooManyCPUCyclesInInterrupt,
    ErrorNotAllowedInInterrupt,
    ErrorIfWithoutEndIf,
    ErrorForWithoutNext,
    ErrorDoWithoutLoop,
    ErrorRepeatWithoutUntil,
    ErrorWhileWithoutWend,
    ErrorExitNotInsideLoop,
    ErrorDirectoryNotLoaded,
    ErrorDivisionByZero,
    ErrorVariableNotInitialized,
    ErrorArrayVariableWithoutIndex,
    ErrorEndSubWithoutSub,
    ErrorSubWithoutEndSub,
    ErrorSubCannotBeNested,
    ErrorUndefinedSubprogram,
    ErrorExpectedSubprogramName,
    ErrorArgumentCountMismatch,
    ErrorSubAlreadyDefined,
    ErrorTooManySubprograms,
    ErrorSharedOutsideOfASubprogram,
    ErrorGlobalInsideOfASubprogram,
    ErrorExitSubOutsideOfASubprogram,
    ErrorKeyboardNotEnabled,
    ErrorAutomaticPauseNotDisabled,
    ErrorNotAllowedOutsideOfInterrupt
};

struct CoreError {
    enum ErrorCode code;
    int sourcePosition;
};

const char *err_getString(enum ErrorCode errorCode);
struct CoreError err_makeCoreError(enum ErrorCode code, int sourcePosition);
struct CoreError err_noCoreError(void);

#endif /* error_h */

//
// Copyright 2017-2020 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef cmd_text_h
#define cmd_text_h

struct Core;

enum ErrorCode cmd_PRINT(struct Core *core);
enum ErrorCode cmd_INPUT(struct Core *core);
enum ErrorCode cmd_endINPUT(struct Core *core);
enum ErrorCode cmd_TEXT(struct Core *core);
enum ErrorCode cmd_NUMBER(struct Core *core);
enum ErrorCode cmd_CLS(struct Core *core);
enum ErrorCode cmd_WINDOW(struct Core *core);
enum ErrorCode cmd_FONT(struct Core *core);
enum ErrorCode cmd_LOCATE(struct Core *core);
struct TypedValue fnc_CURSOR(struct Core *core);
enum ErrorCode cmd_CLW(struct Core *core);
enum ErrorCode cmd_TRACE(struct Core *core);
enum ErrorCode cmd_MESSAGE(struct Core *core);

#endif /* cmd_text_h */

//
// Copyright 2016 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef default_characters_h
#define default_characters_h

extern uint8_t DefaultCharacters[][16];

#endif /* default_characters_h */

