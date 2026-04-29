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

#if defined(_WIN32)
#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>
#elif defined(__ANDROID__)
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_stdinc.h>
#else
#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>
#endif

#ifndef SL_GLOBMATCH_H
#define SL_GLOBMATCH_H 1

#ifdef  __cplusplus
extern "C" {
#endif

  
#ifndef SL_GLOBMATCH_NEGATE
#define SL_GLOBMATCH_NEGATE '^'       /* std char set negation char */
#endif


int sl_globmatch(char *string, char *pattern);


#ifdef  __cplusplus
}
#endif

#endif /* globmatch.h  */
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

#include <stdio.h>

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
    ErrorAutomaticPauseNotDisabled,
    ErrorNotAllowedOutsideOfInterrupt,
		ErrorUserDeviceDiskFull,
		ErrorRandAddressNotSeeded,

		ErrorMax
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

#include <stdint.h>

union IOStatus
{
	struct
	{
		uint8_t touch : 1;
		uint8_t keyboardVisible : 1;
		uint8_t touchTap : 1;
		uint8_t touchDrag : 1;
		uint8_t touchLong : 1;
		uint8_t touchChange : 1;
	};
	uint8_t value;
};

struct IORegisters
{
	// 0x0ff70
	float touchX;
	// 0x0ff74
	float touchY;
	// 0x0ff78
	struct
	{
		uint16_t width, height;
	} shown;
	// 0x0ff7c
	struct
	{
		uint16_t left, top, right, bottom;
	} safe;
	// 0x0ff84
	char key;
	// 0x0ff85
	union IOStatus status;
	// 0x0ff86 + padding
	uint16_t haptic;
	// 0x0ff88
	int keyboardHeight;
	// 0x0ff8c
	float pressedX;
	// 0x0ff90
	float pressedY;
	// 0x0ff94
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

#include <stdio.h>
#include <stdint.h>

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
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define ABGR 1
#endif
#endif
#if __ANDROID__
#define ABGR 1
#endif
#if __linux__
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
    struct Sprite sprites[NUM_SPRITES]; // 1020 bytes
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
				uint8_t planeADoubled:1;
				uint8_t planeBDoubled:1;
				uint8_t planeCDoubled:1;
				uint8_t planeDDoubled:1;
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

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

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

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define VM_SIZE 0x20000
#define VM_MAX 0x1FFFF
#define PERSISTENT_RAM_SIZE 6144
#define MAX_MEMORY_TRACK 32

struct DmaRegisters {
  uint16_t src_addr; // Support RAM or ROM
  uint16_t bytes_count;
  uint16_t dst_addr; // Only RAM
};

struct MemoryTrack {
	uint16_t address;
	int read:1;
	int write:1;
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
    struct SpriteRegisters spriteRegisters; // 1020 Bytes
    uint8_t nothing2[0x400 - sizeof(struct SpriteRegisters)]; // 4 bytes

    // 0x0FF00
    struct ColorRegisters colorRegisters; // 32 Bytes

    // 0x0FF20..0x0FF40
    struct VideoRegisters videoRegisters; // 20 Bytes
    uint8_t nothing3[0x20 - sizeof(struct VideoRegisters)]; // 12 Bytes

    // 0x0FF40..0x0FF70
    struct AudioRegisters audioRegisters;

    // 0x0FF70..0x0FFA0
    struct IORegisters ioRegisters; // 36 Bytes
    uint8_t nothing4[0x30 - sizeof(struct IORegisters)]; // 12 Bytes

    // 0x0FFA0..0x0FFB0
    struct DmaRegisters dmaRegisters; // 6 Bytes
    uint8_t manually_mapped[0x10 - sizeof(struct DmaRegisters)]; // 10 Bytes

    // 0x0FFB0.0x10000
    uint8_t nothing5[0x10000 - 0xFFB0]; // 80 Bytes !!!!!

    // 0x10000..0x20000
    uint8_t cartridgeRom[0x10000]; // 64Kibi
};

struct MachineInternals {
    struct AudioInternals audioInternals;
		struct MemoryTrack memoryTracks[MAX_MEMORY_TRACK];
    int energySavingTimer;
		int numMemoryTracks;
    bool hasAccessedPersistent;
    bool hasChangedPersistent;
    bool isEnergySaving;
		bool planeColor0IsOpaque[4]; // TODO: should be mapped
		bool gesturePressed,gestureLonged,longEnabled,gestureDragged,hasDrag;
		float gesturePressedTimer;
};

void machine_init(struct Core *core);
void machine_reset(struct Core *core, bool resetPersistent);
int machine_peek(struct Core *core, int address);
int16_t machine_peek_short(struct Core *core, int address, enum ErrorCode *errorCode);
int32_t machine_peek_long(struct Core *core, int address, enum ErrorCode *errorCode);
bool machine_poke(struct Core *core, int address, int value);
bool machine_poke_short(struct Core *core, int address, int16_t value);
bool machine_poke_long(struct Core *core, int address, int32_t value);
void machine_enableAudio(struct Core *core);
void machine_suspendEnergySaving(struct Core *core, int numUpdates);
void machine_trackMemory(struct Core *core, uint16_t address, bool read, bool write);
void machine_checkForTrakedMemoryAccess(struct Core *core, uint16_t address, bool read, bool write);

#endif /* machine_h */
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

#include <stdio.h>
#include <stdint.h>

extern uint8_t overlayColors[];
extern uint8_t overlayCharacters[];

#endif /* overlay_data_h */
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

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define INPUT_BUFFER_SIZE 256
#define OVERLAY_BG 4

struct Core;

struct TextLib
{
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

struct Plane *txtlib_getBackground(struct TextLib *lib, int bg);
void txtlib_scroll(struct Plane *plane, int fromX, int fromY, int toX, int toY, int deltaX, int deltaY);
void txtlib_setCellAt(struct Plane *plane, int x, int y, int character, union CharacterAttributes attr);
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
void txtlib_scrollWindowIfNeeded(struct TextLib *lib);

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

#include <stdio.h>
#include <stdbool.h>

struct Core;
struct CoreInput;

struct Overlay
{
	struct Plane plane;
	struct TextLib textLib;
	int timer;
	int messageTimer;
	char commandLine[27];
	char previousCommandLine[9][27];
	int previouscommandLineWriteIndex, previouscommandLineReadIndex;
};

void overlay_init(struct Core *core);
void overlay_clear(struct Core *core);
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

#define MAX_TOKENS 32768
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
#define MAX_CYCLES_TOTAL_PER_FRAME 52668 // 17556*317
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

#include <stdio.h>

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

#include <stdio.h>

enum TokenType
{
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
	TokenADD,
	TokenASC,
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
	// TokenDOWN,
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
	// TokenGAMEPAD,
	TokenGLOBAL,
	TokenGOSUB,
	TokenGOTO,
	TokenHEX,
	TokenHIT,
	TokenIF,
	TokenINC,
	TokenINKEY,
	TokenINPUT,
	TokenINSTR,
	TokenINT,
	TokenKEYBOARD,
	TokenLEFTStr,
	// TokenLEFT,
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
	TokenREPEAT,
	TokenRESTORE,
	TokenRETURN,
	TokenRIGHTStr,
	// TokenRIGHT,
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
	// TokenTOUCHSCREEN,
	TokenTOUCHPX,
	TokenTOUCHPY,
	TokenTOUCHTAP,
	TokenTOUCHDRAG,
	TokenTOUCHLONG,
	TokenTOUCHCHANGE,
	TokenTOUCHX,
	TokenTOUCHY,
	TokenTOUCH,
	TokenTO,
	TokenTRACE,
	TokenTRACK,
	TokenUBOUND,
	TokenUNTIL,
	// TokenUP,
	TokenVAL,
	TokenVBL,
	TokenVIEW,
	TokenVOLUME,
	TokenWAIT,
	TokenWAVE,
	TokenWEND,
	TokenWHILE,
	TokenWINDOWX,
	TokenWINDOWY,
	TokenWINDOWW,
	TokenWINDOWH,
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
	TokenFLOOR,
	TokenHAPTIC,


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

struct Token
{
	enum TokenType type;
	union
	{
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

#include <stdio.h>

struct Symbol
{
	char name[SYMBOL_NAME_SIZE];
};

struct JumpLabelItem
{
	int symbolIndex;
	struct Token *token;
};

struct SubItem
{
	int symbolIndex;
	struct Token *token;
};

struct Tokenizer
{
	struct Token tokens[MAX_TOKENS];
	int numTokens;
	struct Symbol symbols[MAX_SYMBOLS];
	int numSymbols;

	struct JumpLabelItem jumpLabelItems[MAX_SYMBOLS];
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

#ifndef value_h
#define value_h

#include <stdio.h>

enum ValueType
{
	ValueTypeNull,
	ValueTypeError,
	ValueTypeFloat,
	ValueTypeString
};

union Value
{
	float floatValue;
	struct RCString *stringValue;
	union Value *reference;
	enum ErrorCode errorCode;
};

struct TypedValue
{
	enum ValueType type;
	union Value v;
};

enum TypeClass
{
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

#include <stdio.h>

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

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

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

#ifndef data_h
#define data_h

#include <stdio.h>
#include <stdint.h>

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

#include <stdio.h>
#include <stdbool.h>

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

#include <stdio.h>
#include <stdbool.h>

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

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

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
#ifndef particles_lib_h
#define particles_lib_h

#include <stdbool.h>


#define EMITTER_MAX 16
#define APPEARANCE_MAX 24
#define SPAWNER_MAX 16

#define PARTICLE_MEM_X 0
#define PARTICLE_MEM_Y 2
#define PARTICLE_MEM_LIFETIME 4
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
// void prtclib_setApperanceLabel(struct ParticlesLib *lib,int apperanceId,struct Token *label);

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

#include <inttypes.h>

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

#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdbool.h>

#define BAS_TRUE -1.0f
#define BAS_FALSE 0.0f

struct Core;

enum Pass
{
	PassPrepare,
	PassRun
};

enum State
{
	StateNoProgram,
	StateEvaluate,
	StateInput,
	StatePaused,
	StateWaitForDisk,
	StateEnd
};

enum Mode
{
	ModeNone,
	ModeMain,
	ModeInterrupt
};

enum InterruptType
{
	InterruptTypeRaster,
	InterruptTypeVBL,
	InterruptTypeParticle,
	InterruptTypeEmitter,
};

struct Interpreter
{
	const char *sourceCode;

	enum Pass pass;
	enum State state;
	enum Mode mode;
	struct Token *pc;
	int subLevel;
	int cycles;
	int maxCycles;
	int interruptOverCycles;
	bool debug;
	bool pauseAtWait;
	bool logGoto,logGosub;
	int cpuLoadDisplay;
	int cpuLoadMax;
	int cpuLoadTimer;

	bool compat;
	bool simulatedKeyboardOn;

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
	union IOStatus lastFrameIOStatus;
	float timer;
	int seed;
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
enum ErrorCode itp_evaluateCommand(struct Core *core);
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

bool is_equal_approx(float x, float y);
bool is_zero_approx(float x);

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

#ifndef disk_drive_h
#define disk_drive_h

#include <stdio.h>
#include <stdbool.h>

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

#ifndef core_delegate_h
#define core_delegate_h


struct Core;

enum KeyboardMode {
    KeyboardModeOff,
    KeyboardModeOn,
    KeyboardModeOptional
};

enum HapticMode {
		None,
		Error,
		Warning,
		Success,
		Heavy,
		Light,
		Medium,
		Rigid,
		Soft,
		Selection,
};

struct ControlsInfo {
    enum KeyboardMode keyboardMode;
		enum HapticMode hapticMode;
    bool isAudioEnabled;
		bool isInputState;
		bool isCompatMode;
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

    /** Called when keyboard settings changed */
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

#include <stdio.h>
#include <stdbool.h>

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
extern const char CoreInputKeyDelete;

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
void core_setKeyboardEnabled(struct Core *core, bool enabled);
void core_setKeyboardHeight(struct Core *core, int height);
bool core_shouldRender(struct Core *core);
void core_orientationChanged(struct Core *core);

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

#include <stdio.h>

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

#include <stdio.h>

const char *uppercaseString(const char *source);
#if defined(__ANDROID__)
char uppercaseChar(const char sourceChar);
#else
const char uppercaseChar(const char sourceChar);
#endif
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

#include <stdio.h>

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

#include <stdio.h>
#include <stdbool.h>

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

#include <stdio.h>
#include <stdbool.h>

struct Core;

enum ErrorCode cmd_PRINT(struct Core *core);
enum ErrorCode cmd_INPUT(struct Core *core);
enum ErrorCode cmd_endINPUT(struct Core *core);
enum ErrorCode cmd_TEXT(struct Core *core);
enum ErrorCode cmd_NUMBER(struct Core *core);
enum ErrorCode cmd_CLS(struct Core *core);
enum ErrorCode cmd_WINDOW(struct Core *core);
struct TypedValue fnc_WINDOW(struct Core *core);
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

#include <stdio.h>
#include <stdint.h>

extern uint8_t DefaultCharacters[][16];

#endif /* default_characters_h */
#ifndef overlay_debugger_h
#define overlay_debugger_h

#include <stdbool.h>

struct Core;

void trigger_debugger(struct Core *core);
void overlay_debugger(struct Core *core);
void log_goto(struct Core *core,int symbolIndex);
void log_gosub(struct Core *core,int symbolIndex);
void log_return(struct Core *core,bool clear);

#endif /* overlay_debugger_h */
/**
 * Copyright (c) 2020 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>

#define LOG_VERSION "0.1.0"

typedef struct {
  va_list ap;
  const char *fmt;
  const char *file;
  struct tm *time;
  void *udata;
  int line;
  int level;
} log_Event;

typedef void (*log_LogFn)(log_Event *ev);
typedef void (*log_LockFn)(bool lock, void *udata);

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

#define log_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

const char* log_level_string(int level);
void log_set_lock(log_LockFn fn, void *udata);
void log_set_level(int level);
void log_set_quiet(bool enable);
int log_add_callback(log_LogFn fn, void *udata, int level);
int log_add_fp(FILE *fp, int level);

void log_log(int level, const char *file, int line, const char *fmt, ...);

#endif
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

#ifndef config_h
#define config_h

#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
#define SIMULATED_KEYBOARD 0
#define DEV_MENU 0
#define SCREENSHOTS 0
#define HOT_KEYS 0
#define SETTINGS_FILE 0
#else
#define SIMULATED_KEYBOARD 1
#define DEV_MENU 1
#define SCREENSHOTS 1
#define HOT_KEYS 1
#define SETTINGS_FILE 1
#endif

#endif /* config_h */
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

#ifndef runner_h
#define runner_h

#include <stdio.h>
#include <stdbool.h>

struct Runner {
    struct Core *core;
    struct CoreDelegate coreDelegate;
    bool messageShownUsingDisk;
};

void runner_init(struct Runner *runner);
void runner_deinit(struct Runner *runner);
bool runner_isOkay(struct Runner *runner);
struct CoreError runner_loadProgram(struct Runner *runner, const char *filename);

#endif /* runner_h */
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

#ifndef screenshot_h
#define screenshot_h


#if SCREENSHOTS

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

bool screenshot_save(uint32_t *pixels, int scale);

#endif

#endif /* screenshot_h */
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

#ifndef settings_h
#define settings_h

#include <stdio.h>
#include <stdbool.h>

#define MAX_TOOLS 4
#define TOOL_NAME_SIZE 21

struct Parameters {
    bool fullscreen;
    int zoom;
    bool disabledev;
    int mapping;
    int disabledelay;
};

struct Settings {
    struct Parameters file;
    struct Parameters session;
    int numTools;
    char tools[MAX_TOOLS][FILENAME_MAX];
    char toolNames[MAX_TOOLS][TOOL_NAME_SIZE];
};

void settings_init(struct Settings *settings, char *filenameOut, int argc, const char * argv[]);
void settings_save(struct Settings *settings);
bool settings_addTool(struct Settings *settings, const char *filename);
void settings_removeTool(struct Settings *settings, int index);

#endif /* settings_h */
/* stb_image_write - v1.09 - public domain - http://nothings.org/stb/stb_image_write.h
   writes out PNG/BMP/TGA/JPEG/HDR images to C stdio - Sean Barrett 2010-2015
                                     no warranty implied; use at your own risk

   Before #including,

       #define STB_IMAGE_WRITE_IMPLEMENTATION

   in the file that you want to have the implementation.

   Will probably not work correctly with strict-aliasing optimizations.

   If using a modern Microsoft Compiler, non-safe versions of CRT calls may cause
   compilation warnings or even errors. To avoid this, also before #including,

       #define STBI_MSC_SECURE_CRT

ABOUT:

   This header file is a library for writing images to C stdio. It could be
   adapted to write to memory or a general streaming interface; let me know.

   The PNG output is not optimal; it is 20-50% larger than the file
   written by a decent optimizing implementation; though providing a custom
   zlib compress function (see STBIW_ZLIB_COMPRESS) can mitigate that.
   This library is designed for source code compactness and simplicity,
   not optimal image file size or run-time performance.

BUILDING:

   You can #define STBIW_ASSERT(x) before the #include to avoid using assert.h.
   You can #define STBIW_MALLOC(), STBIW_REALLOC(), and STBIW_FREE() to replace
   malloc,realloc,free.
   You can #define STBIW_MEMMOVE() to replace memmove()
   You can #define STBIW_ZLIB_COMPRESS to use a custom zlib-style compress function
   for PNG compression (instead of the builtin one), it must have the following signature:
   unsigned char * my_compress(unsigned char *data, int data_len, int *out_len, int quality);
   The returned data will be freed with STBIW_FREE() (free() by default),
   so it must be heap allocated with STBIW_MALLOC() (malloc() by default),

USAGE:

   There are five functions, one for each image file format:

     int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
     int stbi_write_bmp(char const *filename, int w, int h, int comp, const void *data);
     int stbi_write_tga(char const *filename, int w, int h, int comp, const void *data);
     int stbi_write_jpg(char const *filename, int w, int h, int comp, const void *data, int quality);
     int stbi_write_hdr(char const *filename, int w, int h, int comp, const float *data);

     void stbi_flip_vertically_on_write(int flag); // flag is non-zero to flip data vertically

   There are also five equivalent functions that use an arbitrary write function. You are
   expected to open/close your file-equivalent before and after calling these:

     int stbi_write_png_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void  *data, int stride_in_bytes);
     int stbi_write_bmp_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void  *data);
     int stbi_write_tga_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void  *data);
     int stbi_write_hdr_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const float *data);
     int stbi_write_jpg_to_func(stbi_write_func *func, void *context, int x, int y, int comp, const void *data, int quality);

   where the callback is:
      void stbi_write_func(void *context, void *data, int size);

   You can configure it with these global variables:
      int stbi_write_tga_with_rle;             // defaults to true; set to 0 to disable RLE
      int stbi_write_png_compression_level;    // defaults to 8; set to higher for more compression
      int stbi_write_force_png_filter;         // defaults to -1; set to 0..5 to force a filter mode


   You can define STBI_WRITE_NO_STDIO to disable the file variant of these
   functions, so the library will not use stdio.h at all. However, this will
   also disable HDR writing, because it requires stdio for formatted output.

   Each function returns 0 on failure and non-0 on success.

   The functions create an image file defined by the parameters. The image
   is a rectangle of pixels stored from left-to-right, top-to-bottom.
   Each pixel contains 'comp' channels of data stored interleaved with 8-bits
   per channel, in the following order: 1=Y, 2=YA, 3=RGB, 4=RGBA. (Y is
   monochrome color.) The rectangle is 'w' pixels wide and 'h' pixels tall.
   The *data pointer points to the first byte of the top-left-most pixel.
   For PNG, "stride_in_bytes" is the distance in bytes from the first byte of
   a row of pixels to the first byte of the next row of pixels.

   PNG creates output files with the same number of components as the input.
   The BMP format expands Y to RGB in the file format and does not
   output alpha.

   PNG supports writing rectangles of data even when the bytes storing rows of
   data are not consecutive in memory (e.g. sub-rectangles of a larger image),
   by supplying the stride between the beginning of adjacent rows. The other
   formats do not. (Thus you cannot write a native-format BMP through the BMP
   writer, both because it is in ABGR order and because it may have padding
   at the end of the line.)

   PNG allows you to set the deflate compression level by setting the global
   variable 'stbi_write_png_compression_level' (it defaults to 8).

   HDR expects linear float data. Since the format is always 32-bit rgb(e)
   data, alpha (if provided) is discarded, and for monochrome data it is
   replicated across all three channels.

   TGA supports RLE or non-RLE compressed data. To use non-RLE-compressed
   data, set the global variable 'stbi_write_tga_with_rle' to 0.

   JPEG does ignore alpha channels in input data; quality is between 1 and 100.
   Higher quality looks better but results in a bigger image.
   JPEG baseline (no JPEG progressive).

CREDITS:


   Sean Barrett           -    PNG/BMP/TGA
   Baldur Karlsson        -    HDR
   Jean-Sebastien Guay    -    TGA monochrome
   Tim Kelsey             -    misc enhancements
   Alan Hickman           -    TGA RLE
   Emmanuel Julien        -    initial file IO callback implementation
   Jon Olick              -    original jo_jpeg.cpp code
   Daniel Gibson          -    integrate JPEG, allow external zlib
   Aarni Koskela          -    allow choosing PNG filter

   bugfixes:
      github:Chribba
      Guillaume Chereau
      github:jry2
      github:romigrou
      Sergio Gonzalez
      Jonas Karlsson
      Filip Wasil
      Thatcher Ulrich
      github:poppolopoppo
      Patrick Boettcher
      github:xeekworx
      Cap Petschulat
      Simon Rodriguez
      Ivan Tikhonov
      github:ignotion
      Adam Schackart

LICENSE

  See end of file for license information.

*/

#ifndef INCLUDE_STB_IMAGE_WRITE_H
#define INCLUDE_STB_IMAGE_WRITE_H

// if STB_IMAGE_WRITE_STATIC causes problems, try defining STBIWDEF to 'inline' or 'static inline'
#ifndef STBIWDEF
#ifdef STB_IMAGE_WRITE_STATIC
#define STBIWDEF  static
#else
#ifdef __cplusplus
#define STBIWDEF  extern "C"
#else
#define STBIWDEF  extern
#endif
#endif
#endif

#ifndef STB_IMAGE_WRITE_STATIC  // C++ forbids static forward declarations
extern int stbi_write_tga_with_rle;
extern int stbi_write_png_compression_level;
extern int stbi_write_force_png_filter;
#endif

#ifndef STBI_WRITE_NO_STDIO
STBIWDEF int stbi_write_png(char const *filename, int w, int h, int comp, const void  *data, int stride_in_bytes);
STBIWDEF int stbi_write_bmp(char const *filename, int w, int h, int comp, const void  *data);
STBIWDEF int stbi_write_tga(char const *filename, int w, int h, int comp, const void  *data);
STBIWDEF int stbi_write_hdr(char const *filename, int w, int h, int comp, const float *data);
STBIWDEF int stbi_write_jpg(char const *filename, int x, int y, int comp, const void  *data, int quality);
#endif

typedef void stbi_write_func(void *context, void *data, int size);

STBIWDEF int stbi_write_png_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void  *data, int stride_in_bytes);
STBIWDEF int stbi_write_bmp_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void  *data);
STBIWDEF int stbi_write_tga_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const void  *data);
STBIWDEF int stbi_write_hdr_to_func(stbi_write_func *func, void *context, int w, int h, int comp, const float *data);
STBIWDEF int stbi_write_jpg_to_func(stbi_write_func *func, void *context, int x, int y, int comp, const void  *data, int quality);

STBIWDEF void stbi_flip_vertically_on_write(int flip_boolean);

#endif//INCLUDE_STB_IMAGE_WRITE_H

#ifdef STB_IMAGE_WRITE_IMPLEMENTATION

#ifdef _WIN32
   #ifndef _CRT_SECURE_NO_WARNINGS
   #define _CRT_SECURE_NO_WARNINGS
   #endif
   #ifndef _CRT_NONSTDC_NO_DEPRECATE
   #define _CRT_NONSTDC_NO_DEPRECATE
   #endif
#endif

#ifndef STBI_WRITE_NO_STDIO
#include <stdio.h>
#endif // STBI_WRITE_NO_STDIO

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if defined(STBIW_MALLOC) && defined(STBIW_FREE) && (defined(STBIW_REALLOC) || defined(STBIW_REALLOC_SIZED))
// ok
#elif !defined(STBIW_MALLOC) && !defined(STBIW_FREE) && !defined(STBIW_REALLOC) && !defined(STBIW_REALLOC_SIZED)
// ok
#else
#error "Must define all or none of STBIW_MALLOC, STBIW_FREE, and STBIW_REALLOC (or STBIW_REALLOC_SIZED)."
#endif

#ifndef STBIW_MALLOC
#define STBIW_MALLOC(sz)        malloc(sz)
#define STBIW_REALLOC(p,newsz)  realloc(p,newsz)
#define STBIW_FREE(p)           free(p)
#endif

#ifndef STBIW_REALLOC_SIZED
#define STBIW_REALLOC_SIZED(p,oldsz,newsz) STBIW_REALLOC(p,newsz)
#endif


#ifndef STBIW_MEMMOVE
#define STBIW_MEMMOVE(a,b,sz) memmove(a,b,sz)
#endif


#ifndef STBIW_ASSERT
#include <assert.h>
#define STBIW_ASSERT(x) assert(x)
#endif

#define STBIW_UCHAR(x) (unsigned char) ((x) & 0xff)

#ifdef STB_IMAGE_WRITE_STATIC
static int stbi__flip_vertically_on_write=0;
static int stbi_write_png_compression_level = 8;
static int stbi_write_tga_with_rle = 1;
static int stbi_write_force_png_filter = -1;
#else
int stbi_write_png_compression_level = 8;
int stbi__flip_vertically_on_write=0;
int stbi_write_tga_with_rle = 1;
int stbi_write_force_png_filter = -1;
#endif

STBIWDEF void stbi_flip_vertically_on_write(int flag)
{
   stbi__flip_vertically_on_write = flag;
}

typedef struct
{
   stbi_write_func *func;
   void *context;
} stbi__write_context;

// initialize a callback-based context
static void stbi__start_write_callbacks(stbi__write_context *s, stbi_write_func *c, void *context)
{
   s->func    = c;
   s->context = context;
}

#ifndef STBI_WRITE_NO_STDIO

static void stbi__stdio_write(void *context, void *data, int size)
{
   fwrite(data,1,size,(FILE*) context);
}

static int stbi__start_write_file(stbi__write_context *s, const char *filename)
{
   FILE *f;
#ifdef STBI_MSC_SECURE_CRT
   if (fopen_s(&f, filename, "wb"))
      f = NULL;
#else
   f = fopen(filename, "wb");
#endif
   stbi__start_write_callbacks(s, stbi__stdio_write, (void *) f);
   return f != NULL;
}

static void stbi__end_write_file(stbi__write_context *s)
{
   fclose((FILE *)s->context);
}

#endif // !STBI_WRITE_NO_STDIO

typedef unsigned int stbiw_uint32;
typedef int stb_image_write_test[sizeof(stbiw_uint32)==4 ? 1 : -1];

static void stbiw__writefv(stbi__write_context *s, const char *fmt, va_list v)
{
   while (*fmt) {
      switch (*fmt++) {
         case ' ': break;
         case '1': { unsigned char x = STBIW_UCHAR(va_arg(v, int));
                     s->func(s->context,&x,1);
                     break; }
         case '2': { int x = va_arg(v,int);
                     unsigned char b[2];
                     b[0] = STBIW_UCHAR(x);
                     b[1] = STBIW_UCHAR(x>>8);
                     s->func(s->context,b,2);
                     break; }
         case '4': { stbiw_uint32 x = va_arg(v,int);
                     unsigned char b[4];
                     b[0]=STBIW_UCHAR(x);
                     b[1]=STBIW_UCHAR(x>>8);
                     b[2]=STBIW_UCHAR(x>>16);
                     b[3]=STBIW_UCHAR(x>>24);
                     s->func(s->context,b,4);
                     break; }
         default:
            STBIW_ASSERT(0);
            return;
      }
   }
}

static void stbiw__writef(stbi__write_context *s, const char *fmt, ...)
{
   va_list v;
   va_start(v, fmt);
   stbiw__writefv(s, fmt, v);
   va_end(v);
}

static void stbiw__putc(stbi__write_context *s, unsigned char c)
{
   s->func(s->context, &c, 1);
}

static void stbiw__write3(stbi__write_context *s, unsigned char a, unsigned char b, unsigned char c)
{
   unsigned char arr[3];
   arr[0] = a;
   arr[1] = b;
   arr[2] = c;
   s->func(s->context, arr, 3);
}

static void stbiw__write_pixel(stbi__write_context *s, int rgb_dir, int comp, int write_alpha, int expand_mono, unsigned char *d)
{
   unsigned char bg[3] = { 255, 0, 255}, px[3];
   int k;

   if (write_alpha < 0)
      s->func(s->context, &d[comp - 1], 1);

   switch (comp) {
      case 2: // 2 pixels = mono + alpha, alpha is written separately, so same as 1-channel case
      case 1:
         if (expand_mono)
            stbiw__write3(s, d[0], d[0], d[0]); // monochrome bmp
         else
            s->func(s->context, d, 1);  // monochrome TGA
         break;
      case 4:
         if (!write_alpha) {
            // composite against pink background
            for (k = 0; k < 3; ++k)
               px[k] = bg[k] + ((d[k] - bg[k]) * d[3]) / 255;
            stbiw__write3(s, px[1 - rgb_dir], px[1], px[1 + rgb_dir]);
            break;
         }
         /* FALLTHROUGH */
      case 3:
         stbiw__write3(s, d[1 - rgb_dir], d[1], d[1 + rgb_dir]);
         break;
   }
   if (write_alpha > 0)
      s->func(s->context, &d[comp - 1], 1);
}

static void stbiw__write_pixels(stbi__write_context *s, int rgb_dir, int vdir, int x, int y, int comp, void *data, int write_alpha, int scanline_pad, int expand_mono)
{
   stbiw_uint32 zero = 0;
   int i,j, j_end;

   if (y <= 0)
      return;

   if (stbi__flip_vertically_on_write)
      vdir *= -1;

   if (vdir < 0) {
      j_end = -1;
      j = y-1;
   } else {
      j_end = y;
      j = 0;
   }

   for (; j != j_end; j += vdir) {
      for (i=0; i < x; ++i) {
         unsigned char *d = (unsigned char *) data + (j*x+i)*comp;
         stbiw__write_pixel(s, rgb_dir, comp, write_alpha, expand_mono, d);
      }
      s->func(s->context, &zero, scanline_pad);
   }
}

static int stbiw__outfile(stbi__write_context *s, int rgb_dir, int vdir, int x, int y, int comp, int expand_mono, void *data, int alpha, int pad, const char *fmt, ...)
{
   if (y < 0 || x < 0) {
      return 0;
   } else {
      va_list v;
      va_start(v, fmt);
      stbiw__writefv(s, fmt, v);
      va_end(v);
      stbiw__write_pixels(s,rgb_dir,vdir,x,y,comp,data,alpha,pad, expand_mono);
      return 1;
   }
}

static int stbi_write_bmp_core(stbi__write_context *s, int x, int y, int comp, const void *data)
{
   int pad = (-x*3) & 3;
   return stbiw__outfile(s,-1,-1,x,y,comp,1,(void *) data,0,pad,
           "11 4 22 4" "4 44 22 444444",
           'B', 'M', 14+40+(x*3+pad)*y, 0,0, 14+40,  // file header
            40, x,y, 1,24, 0,0,0,0,0,0);             // bitmap header
}

STBIWDEF int stbi_write_bmp_to_func(stbi_write_func *func, void *context, int x, int y, int comp, const void *data)
{
   stbi__write_context s;
   stbi__start_write_callbacks(&s, func, context);
   return stbi_write_bmp_core(&s, x, y, comp, data);
}

#ifndef STBI_WRITE_NO_STDIO
STBIWDEF int stbi_write_bmp(char const *filename, int x, int y, int comp, const void *data)
{
   stbi__write_context s;
   if (stbi__start_write_file(&s,filename)) {
      int r = stbi_write_bmp_core(&s, x, y, comp, data);
      stbi__end_write_file(&s);
      return r;
   } else
      return 0;
}
#endif //!STBI_WRITE_NO_STDIO

static int stbi_write_tga_core(stbi__write_context *s, int x, int y, int comp, void *data)
{
   int has_alpha = (comp == 2 || comp == 4);
   int colorbytes = has_alpha ? comp-1 : comp;
   int format = colorbytes < 2 ? 3 : 2; // 3 color channels (RGB/RGBA) = 2, 1 color channel (Y/YA) = 3

   if (y < 0 || x < 0)
      return 0;

   if (!stbi_write_tga_with_rle) {
      return stbiw__outfile(s, -1, -1, x, y, comp, 0, (void *) data, has_alpha, 0,
         "111 221 2222 11", 0, 0, format, 0, 0, 0, 0, 0, x, y, (colorbytes + has_alpha) * 8, has_alpha * 8);
   } else {
      int i,j,k;
      int jend, jdir;

      stbiw__writef(s, "111 221 2222 11", 0,0,format+8, 0,0,0, 0,0,x,y, (colorbytes + has_alpha) * 8, has_alpha * 8);

      if (stbi__flip_vertically_on_write) {
         j = 0;
         jend = y;
         jdir = 1;
      } else {
         j = y-1;
         jend = -1;
         jdir = -1;
      }
      for (; j != jend; j += jdir) {
         unsigned char *row = (unsigned char *) data + j * x * comp;
         int len;

         for (i = 0; i < x; i += len) {
            unsigned char *begin = row + i * comp;
            int diff = 1;
            len = 1;

            if (i < x - 1) {
               ++len;
               diff = memcmp(begin, row + (i + 1) * comp, comp);
               if (diff) {
                  const unsigned char *prev = begin;
                  for (k = i + 2; k < x && len < 128; ++k) {
                     if (memcmp(prev, row + k * comp, comp)) {
                        prev += comp;
                        ++len;
                     } else {
                        --len;
                        break;
                     }
                  }
               } else {
                  for (k = i + 2; k < x && len < 128; ++k) {
                     if (!memcmp(begin, row + k * comp, comp)) {
                        ++len;
                     } else {
                        break;
                     }
                  }
               }
            }

            if (diff) {
               unsigned char header = STBIW_UCHAR(len - 1);
               s->func(s->context, &header, 1);
               for (k = 0; k < len; ++k) {
                  stbiw__write_pixel(s, -1, comp, has_alpha, 0, begin + k * comp);
               }
            } else {
               unsigned char header = STBIW_UCHAR(len - 129);
               s->func(s->context, &header, 1);
               stbiw__write_pixel(s, -1, comp, has_alpha, 0, begin);
            }
         }
      }
   }
   return 1;
}

STBIWDEF int stbi_write_tga_to_func(stbi_write_func *func, void *context, int x, int y, int comp, const void *data)
{
   stbi__write_context s;
   stbi__start_write_callbacks(&s, func, context);
   return stbi_write_tga_core(&s, x, y, comp, (void *) data);
}

#ifndef STBI_WRITE_NO_STDIO
STBIWDEF int stbi_write_tga(char const *filename, int x, int y, int comp, const void *data)
{
   stbi__write_context s;
   if (stbi__start_write_file(&s,filename)) {
      int r = stbi_write_tga_core(&s, x, y, comp, (void *) data);
      stbi__end_write_file(&s);
      return r;
   } else
      return 0;
}
#endif

// *************************************************************************************************
// Radiance RGBE HDR writer
// by Baldur Karlsson

#define stbiw__max(a, b)  ((a) > (b) ? (a) : (b))

void stbiw__linear_to_rgbe(unsigned char *rgbe, float *linear)
{
   int exponent;
   float maxcomp = stbiw__max(linear[0], stbiw__max(linear[1], linear[2]));

   if (maxcomp < 1e-32f) {
      rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
   } else {
      float normalize = (float) frexp(maxcomp, &exponent) * 256.0f/maxcomp;

      rgbe[0] = (unsigned char)(linear[0] * normalize);
      rgbe[1] = (unsigned char)(linear[1] * normalize);
      rgbe[2] = (unsigned char)(linear[2] * normalize);
      rgbe[3] = (unsigned char)(exponent + 128);
   }
}

void stbiw__write_run_data(stbi__write_context *s, int length, unsigned char databyte)
{
   unsigned char lengthbyte = STBIW_UCHAR(length+128);
   STBIW_ASSERT(length+128 <= 255);
   s->func(s->context, &lengthbyte, 1);
   s->func(s->context, &databyte, 1);
}

void stbiw__write_dump_data(stbi__write_context *s, int length, unsigned char *data)
{
   unsigned char lengthbyte = STBIW_UCHAR(length);
   STBIW_ASSERT(length <= 128); // inconsistent with spec but consistent with official code
   s->func(s->context, &lengthbyte, 1);
   s->func(s->context, data, length);
}

void stbiw__write_hdr_scanline(stbi__write_context *s, int width, int ncomp, unsigned char *scratch, float *scanline)
{
   unsigned char scanlineheader[4] = { 2, 2, 0, 0 };
   unsigned char rgbe[4];
   float linear[3];
   int x;

   scanlineheader[2] = (width&0xff00)>>8;
   scanlineheader[3] = (width&0x00ff);

   /* skip RLE for images too small or large */
   if (width < 8 || width >= 32768) {
      for (x=0; x < width; x++) {
         switch (ncomp) {
            case 4: /* fallthrough */
            case 3: linear[2] = scanline[x*ncomp + 2];
                    linear[1] = scanline[x*ncomp + 1];
                    linear[0] = scanline[x*ncomp + 0];
                    break;
            default:
                    linear[0] = linear[1] = linear[2] = scanline[x*ncomp + 0];
                    break;
         }
         stbiw__linear_to_rgbe(rgbe, linear);
         s->func(s->context, rgbe, 4);
      }
   } else {
      int c,r;
      /* encode into scratch buffer */
      for (x=0; x < width; x++) {
         switch(ncomp) {
            case 4: /* fallthrough */
            case 3: linear[2] = scanline[x*ncomp + 2];
                    linear[1] = scanline[x*ncomp + 1];
                    linear[0] = scanline[x*ncomp + 0];
                    break;
            default:
                    linear[0] = linear[1] = linear[2] = scanline[x*ncomp + 0];
                    break;
         }
         stbiw__linear_to_rgbe(rgbe, linear);
         scratch[x + width*0] = rgbe[0];
         scratch[x + width*1] = rgbe[1];
         scratch[x + width*2] = rgbe[2];
         scratch[x + width*3] = rgbe[3];
      }

      s->func(s->context, scanlineheader, 4);

      /* RLE each component separately */
      for (c=0; c < 4; c++) {
         unsigned char *comp = &scratch[width*c];

         x = 0;
         while (x < width) {
            // find first run
            r = x;
            while (r+2 < width) {
               if (comp[r] == comp[r+1] && comp[r] == comp[r+2])
                  break;
               ++r;
            }
            if (r+2 >= width)
               r = width;
            // dump up to first run
            while (x < r) {
               int len = r-x;
               if (len > 128) len = 128;
               stbiw__write_dump_data(s, len, &comp[x]);
               x += len;
            }
            // if there's a run, output it
            if (r+2 < width) { // same test as what we break out of in search loop, so only true if we break'd
               // find next byte after run
               while (r < width && comp[r] == comp[x])
                  ++r;
               // output run up to r
               while (x < r) {
                  int len = r-x;
                  if (len > 127) len = 127;
                  stbiw__write_run_data(s, len, comp[x]);
                  x += len;
               }
            }
         }
      }
   }
}

static int stbi_write_hdr_core(stbi__write_context *s, int x, int y, int comp, float *data)
{
   if (y <= 0 || x <= 0 || data == NULL)
      return 0;
   else {
      // Each component is stored separately. Allocate scratch space for full output scanline.
      unsigned char *scratch = (unsigned char *) STBIW_MALLOC(x*4);
      int i, len;
      char buffer[128];
      char header[] = "#?RADIANCE\n# Written by stb_image_write.h\nFORMAT=32-bit_rle_rgbe\n";
      s->func(s->context, header, sizeof(header)-1);

#ifdef STBI_MSC_SECURE_CRT
      len = sprintf_s(buffer, "EXPOSURE=          1.0000000000000\n\n-Y %d +X %d\n", y, x);
#else
      len = sprintf(buffer, "EXPOSURE=          1.0000000000000\n\n-Y %d +X %d\n", y, x);
#endif
      s->func(s->context, buffer, len);

      for(i=0; i < y; i++)
         stbiw__write_hdr_scanline(s, x, comp, scratch, data + comp*x*(stbi__flip_vertically_on_write ? y-1-i : i)*x);
      STBIW_FREE(scratch);
      return 1;
   }
}

STBIWDEF int stbi_write_hdr_to_func(stbi_write_func *func, void *context, int x, int y, int comp, const float *data)
{
   stbi__write_context s;
   stbi__start_write_callbacks(&s, func, context);
   return stbi_write_hdr_core(&s, x, y, comp, (float *) data);
}

#ifndef STBI_WRITE_NO_STDIO
STBIWDEF int stbi_write_hdr(char const *filename, int x, int y, int comp, const float *data)
{
   stbi__write_context s;
   if (stbi__start_write_file(&s,filename)) {
      int r = stbi_write_hdr_core(&s, x, y, comp, (float *) data);
      stbi__end_write_file(&s);
      return r;
   } else
      return 0;
}
#endif // STBI_WRITE_NO_STDIO


//////////////////////////////////////////////////////////////////////////////
//
// PNG writer
//

#ifndef STBIW_ZLIB_COMPRESS
// stretchy buffer; stbiw__sbpush() == vector<>::push_back() -- stbiw__sbcount() == vector<>::size()
#define stbiw__sbraw(a) ((int *) (a) - 2)
#define stbiw__sbm(a)   stbiw__sbraw(a)[0]
#define stbiw__sbn(a)   stbiw__sbraw(a)[1]

#define stbiw__sbneedgrow(a,n)  ((a)==0 || stbiw__sbn(a)+n >= stbiw__sbm(a))
#define stbiw__sbmaybegrow(a,n) (stbiw__sbneedgrow(a,(n)) ? stbiw__sbgrow(a,n) : 0)
#define stbiw__sbgrow(a,n)  stbiw__sbgrowf((void **) &(a), (n), sizeof(*(a)))

#define stbiw__sbpush(a, v)      (stbiw__sbmaybegrow(a,1), (a)[stbiw__sbn(a)++] = (v))
#define stbiw__sbcount(a)        ((a) ? stbiw__sbn(a) : 0)
#define stbiw__sbfree(a)         ((a) ? STBIW_FREE(stbiw__sbraw(a)),0 : 0)

static void *stbiw__sbgrowf(void **arr, int increment, int itemsize)
{
   int m = *arr ? 2*stbiw__sbm(*arr)+increment : increment+1;
   void *p = STBIW_REALLOC_SIZED(*arr ? stbiw__sbraw(*arr) : 0, *arr ? (stbiw__sbm(*arr)*itemsize + sizeof(int)*2) : 0, itemsize * m + sizeof(int)*2);
   STBIW_ASSERT(p);
   if (p) {
      if (!*arr) ((int *) p)[1] = 0;
      *arr = (void *) ((int *) p + 2);
      stbiw__sbm(*arr) = m;
   }
   return *arr;
}

static unsigned char *stbiw__zlib_flushf(unsigned char *data, unsigned int *bitbuffer, int *bitcount)
{
   while (*bitcount >= 8) {
      stbiw__sbpush(data, STBIW_UCHAR(*bitbuffer));
      *bitbuffer >>= 8;
      *bitcount -= 8;
   }
   return data;
}

static int stbiw__zlib_bitrev(int code, int codebits)
{
   int res=0;
   while (codebits--) {
      res = (res << 1) | (code & 1);
      code >>= 1;
   }
   return res;
}

static unsigned int stbiw__zlib_countm(unsigned char *a, unsigned char *b, int limit)
{
   int i;
   for (i=0; i < limit && i < 258; ++i)
      if (a[i] != b[i]) break;
   return i;
}

static unsigned int stbiw__zhash(unsigned char *data)
{
   stbiw_uint32 hash = data[0] + (data[1] << 8) + (data[2] << 16);
   hash ^= hash << 3;
   hash += hash >> 5;
   hash ^= hash << 4;
   hash += hash >> 17;
   hash ^= hash << 25;
   hash += hash >> 6;
   return hash;
}

#define stbiw__zlib_flush() (out = stbiw__zlib_flushf(out, &bitbuf, &bitcount))
#define stbiw__zlib_add(code,codebits) \
      (bitbuf |= (code) << bitcount, bitcount += (codebits), stbiw__zlib_flush())
#define stbiw__zlib_huffa(b,c)  stbiw__zlib_add(stbiw__zlib_bitrev(b,c),c)
// default huffman tables
#define stbiw__zlib_huff1(n)  stbiw__zlib_huffa(0x30 + (n), 8)
#define stbiw__zlib_huff2(n)  stbiw__zlib_huffa(0x190 + (n)-144, 9)
#define stbiw__zlib_huff3(n)  stbiw__zlib_huffa(0 + (n)-256,7)
#define stbiw__zlib_huff4(n)  stbiw__zlib_huffa(0xc0 + (n)-280,8)
#define stbiw__zlib_huff(n)  ((n) <= 143 ? stbiw__zlib_huff1(n) : (n) <= 255 ? stbiw__zlib_huff2(n) : (n) <= 279 ? stbiw__zlib_huff3(n) : stbiw__zlib_huff4(n))
#define stbiw__zlib_huffb(n) ((n) <= 143 ? stbiw__zlib_huff1(n) : stbiw__zlib_huff2(n))

#define stbiw__ZHASH   16384

#endif // STBIW_ZLIB_COMPRESS

unsigned char * stbi_zlib_compress(unsigned char *data, int data_len, int *out_len, int quality)
{
#ifdef STBIW_ZLIB_COMPRESS
   // user provided a zlib compress implementation, use that
   return STBIW_ZLIB_COMPRESS(data, data_len, out_len, quality);
#else // use builtin
   static unsigned short lengthc[] = { 3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258, 259 };
   static unsigned char  lengtheb[]= { 0,0,0,0,0,0,0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4,  4,  5,  5,  5,  5,  0 };
   static unsigned short distc[]   = { 1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577, 32768 };
   static unsigned char  disteb[]  = { 0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13 };
   unsigned int bitbuf=0;
   int i,j, bitcount=0;
   unsigned char *out = NULL;
   unsigned char ***hash_table = (unsigned char***) STBIW_MALLOC(stbiw__ZHASH * sizeof(char**));
   if (hash_table == NULL)
      return NULL;
   if (quality < 5) quality = 5;

   stbiw__sbpush(out, 0x78);   // DEFLATE 32K window
   stbiw__sbpush(out, 0x5e);   // FLEVEL = 1
   stbiw__zlib_add(1,1);  // BFINAL = 1
   stbiw__zlib_add(1,2);  // BTYPE = 1 -- fixed huffman

   for (i=0; i < stbiw__ZHASH; ++i)
      hash_table[i] = NULL;

   i=0;
   while (i < data_len-3) {
      // hash next 3 bytes of data to be compressed
      int h = stbiw__zhash(data+i)&(stbiw__ZHASH-1), best=3;
      unsigned char *bestloc = 0;
      unsigned char **hlist = hash_table[h];
      int n = stbiw__sbcount(hlist);
      for (j=0; j < n; ++j) {
         if (hlist[j]-data > i-32768) { // if entry lies within window
            int d = stbiw__zlib_countm(hlist[j], data+i, data_len-i);
            if (d >= best) {
               best=d; bestloc=hlist[j];
            }
         }
      }
      // when hash table entry is too long, delete half the entries
      if (hash_table[h] && stbiw__sbn(hash_table[h]) == 2*quality) {
         STBIW_MEMMOVE(hash_table[h], hash_table[h]+quality, sizeof(hash_table[h][0])*quality);
         stbiw__sbn(hash_table[h]) = quality;
      }
      stbiw__sbpush(hash_table[h],data+i);

      if (bestloc) {
         // "lazy matching" - check match at *next* byte, and if it's better, do cur byte as literal
         h = stbiw__zhash(data+i+1)&(stbiw__ZHASH-1);
         hlist = hash_table[h];
         n = stbiw__sbcount(hlist);
         for (j=0; j < n; ++j) {
            if (hlist[j]-data > i-32767) {
               int e = stbiw__zlib_countm(hlist[j], data+i+1, data_len-i-1);
               if (e > best) { // if next match is better, bail on current match
                  bestloc = NULL;
                  break;
               }
            }
         }
      }

      if (bestloc) {
         int d = (int) (data+i - bestloc); // distance back
         STBIW_ASSERT(d <= 32767 && best <= 258);
         for (j=0; best > lengthc[j+1]-1; ++j);
         stbiw__zlib_huff(j+257);
         if (lengtheb[j]) stbiw__zlib_add(best - lengthc[j], lengtheb[j]);
         for (j=0; d > distc[j+1]-1; ++j);
         stbiw__zlib_add(stbiw__zlib_bitrev(j,5),5);
         if (disteb[j]) stbiw__zlib_add(d - distc[j], disteb[j]);
         i += best;
      } else {
         stbiw__zlib_huffb(data[i]);
         ++i;
      }
   }
   // write out final bytes
   for (;i < data_len; ++i)
      stbiw__zlib_huffb(data[i]);
   stbiw__zlib_huff(256); // end of block
   // pad with 0 bits to byte boundary
   while (bitcount)
      stbiw__zlib_add(0,1);

   for (i=0; i < stbiw__ZHASH; ++i)
      (void) stbiw__sbfree(hash_table[i]);
   STBIW_FREE(hash_table);

   {
      // compute adler32 on input
      unsigned int s1=1, s2=0;
      int blocklen = (int) (data_len % 5552);
      j=0;
      while (j < data_len) {
         for (i=0; i < blocklen; ++i) {
            s1 += data[j+i];
            s2 += s1;
         }
         s1 %= 65521;
         s2 %= 65521;
         j += blocklen;
         blocklen = 5552;
      }
      stbiw__sbpush(out, STBIW_UCHAR(s2 >> 8));
      stbiw__sbpush(out, STBIW_UCHAR(s2));
      stbiw__sbpush(out, STBIW_UCHAR(s1 >> 8));
      stbiw__sbpush(out, STBIW_UCHAR(s1));
   }
   *out_len = stbiw__sbn(out);
   // make returned pointer freeable
   STBIW_MEMMOVE(stbiw__sbraw(out), out, *out_len);
   return (unsigned char *) stbiw__sbraw(out);
#endif // STBIW_ZLIB_COMPRESS
}

static unsigned int stbiw__crc32(unsigned char *buffer, int len)
{
   static unsigned int crc_table[256] =
   {
      0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
      0x0eDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
      0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
      0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
      0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
      0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
      0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
      0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
      0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
      0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
      0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
      0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
      0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
      0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
      0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
      0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
      0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
      0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
      0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
      0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
      0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
      0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
      0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
      0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
      0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
      0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
      0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
      0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
      0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
      0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
      0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
      0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
   };

   unsigned int crc = ~0u;
   int i;
   for (i=0; i < len; ++i)
      crc = (crc >> 8) ^ crc_table[buffer[i] ^ (crc & 0xff)];
   return ~crc;
}

#define stbiw__wpng4(o,a,b,c,d) ((o)[0]=STBIW_UCHAR(a),(o)[1]=STBIW_UCHAR(b),(o)[2]=STBIW_UCHAR(c),(o)[3]=STBIW_UCHAR(d),(o)+=4)
#define stbiw__wp32(data,v) stbiw__wpng4(data, (v)>>24,(v)>>16,(v)>>8,(v));
#define stbiw__wptag(data,s) stbiw__wpng4(data, s[0],s[1],s[2],s[3])

static void stbiw__wpcrc(unsigned char **data, int len)
{
   unsigned int crc = stbiw__crc32(*data - len - 4, len+4);
   stbiw__wp32(*data, crc);
}

static unsigned char stbiw__paeth(int a, int b, int c)
{
   int p = a + b - c, pa = abs(p-a), pb = abs(p-b), pc = abs(p-c);
   if (pa <= pb && pa <= pc) return STBIW_UCHAR(a);
   if (pb <= pc) return STBIW_UCHAR(b);
   return STBIW_UCHAR(c);
}

// @OPTIMIZE: provide an option that always forces left-predict or paeth predict
static void stbiw__encode_png_line(unsigned char *pixels, int stride_bytes, int width, int height, int y, int n, int filter_type, signed char *line_buffer)
{
   static int mapping[] = { 0,1,2,3,4 };
   static int firstmap[] = { 0,1,0,5,6 };
   int *mymap = (y != 0) ? mapping : firstmap;
   int i;
   int type = mymap[filter_type];
   unsigned char *z = pixels + stride_bytes * (stbi__flip_vertically_on_write ? height-1-y : y);
   int signed_stride = stbi__flip_vertically_on_write ? -stride_bytes : stride_bytes;
   for (i = 0; i < n; ++i) {
      switch (type) {
         case 0: line_buffer[i] = z[i]; break;
         case 1: line_buffer[i] = z[i]; break;
         case 2: line_buffer[i] = z[i] - z[i-signed_stride]; break;
         case 3: line_buffer[i] = z[i] - (z[i-signed_stride]>>1); break;
         case 4: line_buffer[i] = (signed char) (z[i] - stbiw__paeth(0,z[i-signed_stride],0)); break;
         case 5: line_buffer[i] = z[i]; break;
         case 6: line_buffer[i] = z[i]; break;
      }
   }
   for (i=n; i < width*n; ++i) {
      switch (type) {
         case 0: line_buffer[i] = z[i]; break;
         case 1: line_buffer[i] = z[i] - z[i-n]; break;
         case 2: line_buffer[i] = z[i] - z[i-signed_stride]; break;
         case 3: line_buffer[i] = z[i] - ((z[i-n] + z[i-signed_stride])>>1); break;
         case 4: line_buffer[i] = z[i] - stbiw__paeth(z[i-n], z[i-signed_stride], z[i-signed_stride-n]); break;
         case 5: line_buffer[i] = z[i] - (z[i-n]>>1); break;
         case 6: line_buffer[i] = z[i] - stbiw__paeth(z[i-n], 0,0); break;
      }
   }
}

unsigned char *stbi_write_png_to_mem(unsigned char *pixels, int stride_bytes, int x, int y, int n, int *out_len)
{
   int force_filter = stbi_write_force_png_filter;
   int ctype[5] = { -1, 0, 4, 2, 6 };
   unsigned char sig[8] = { 137,80,78,71,13,10,26,10 };
   unsigned char *out,*o, *filt, *zlib;
   signed char *line_buffer;
   int j,zlen;

   if (stride_bytes == 0)
      stride_bytes = x * n;

   if (force_filter >= 5) {
      force_filter = -1;
   }

   filt = (unsigned char *) STBIW_MALLOC((x*n+1) * y); if (!filt) return 0;
   line_buffer = (signed char *) STBIW_MALLOC(x * n); if (!line_buffer) { STBIW_FREE(filt); return 0; }
   for (j=0; j < y; ++j) {
      int filter_type;
      if (force_filter > -1) {
         filter_type = force_filter;
         stbiw__encode_png_line(pixels, stride_bytes, x, y, j, n, force_filter, line_buffer);
      } else { // Estimate the best filter by running through all of them:
         int best_filter = 0, best_filter_val = 0x7fffffff, est, i;
         for (filter_type = 0; filter_type < 5; filter_type++) {
            stbiw__encode_png_line(pixels, stride_bytes, x, y, j, n, filter_type, line_buffer);

            // Estimate the entropy of the line using this filter; the less, the better.
            est = 0;
            for (i = 0; i < x*n; ++i) {
               est += abs((signed char) line_buffer[i]);
            }
            if (est < best_filter_val) {
               best_filter_val = est;
               best_filter = filter_type;
            }
         }
         if (filter_type != best_filter) {  // If the last iteration already got us the best filter, don't redo it
            stbiw__encode_png_line(pixels, stride_bytes, x, y, j, n, best_filter, line_buffer);
            filter_type = best_filter;
         }
      }
      // when we get here, filter_type contains the filter type, and line_buffer contains the data
      filt[j*(x*n+1)] = (unsigned char) filter_type;
      STBIW_MEMMOVE(filt+j*(x*n+1)+1, line_buffer, x*n);
   }
   STBIW_FREE(line_buffer);
   zlib = stbi_zlib_compress(filt, y*( x*n+1), &zlen, stbi_write_png_compression_level);
   STBIW_FREE(filt);
   if (!zlib) return 0;

   // each tag requires 12 bytes of overhead
   out = (unsigned char *) STBIW_MALLOC(8 + 12+13 + 12+zlen + 12);
   if (!out) return 0;
   *out_len = 8 + 12+13 + 12+zlen + 12;

   o=out;
   STBIW_MEMMOVE(o,sig,8); o+= 8;
   stbiw__wp32(o, 13); // header length
   stbiw__wptag(o, "IHDR");
   stbiw__wp32(o, x);
   stbiw__wp32(o, y);
   *o++ = 8;
   *o++ = STBIW_UCHAR(ctype[n]);
   *o++ = 0;
   *o++ = 0;
   *o++ = 0;
   stbiw__wpcrc(&o,13);

   stbiw__wp32(o, zlen);
   stbiw__wptag(o, "IDAT");
   STBIW_MEMMOVE(o, zlib, zlen);
   o += zlen;
   STBIW_FREE(zlib);
   stbiw__wpcrc(&o, zlen);

   stbiw__wp32(o,0);
   stbiw__wptag(o, "IEND");
   stbiw__wpcrc(&o,0);

   STBIW_ASSERT(o == out + *out_len);

   return out;
}

#ifndef STBI_WRITE_NO_STDIO
STBIWDEF int stbi_write_png(char const *filename, int x, int y, int comp, const void *data, int stride_bytes)
{
   FILE *f;
   int len;
   unsigned char *png = stbi_write_png_to_mem((unsigned char *) data, stride_bytes, x, y, comp, &len);
   if (png == NULL) return 0;
#ifdef STBI_MSC_SECURE_CRT
   if (fopen_s(&f, filename, "wb"))
      f = NULL;
#else
   f = fopen(filename, "wb");
#endif
   if (!f) { STBIW_FREE(png); return 0; }
   fwrite(png, 1, len, f);
   fclose(f);
   STBIW_FREE(png);
   return 1;
}
#endif

STBIWDEF int stbi_write_png_to_func(stbi_write_func *func, void *context, int x, int y, int comp, const void *data, int stride_bytes)
{
   int len;
   unsigned char *png = stbi_write_png_to_mem((unsigned char *) data, stride_bytes, x, y, comp, &len);
   if (png == NULL) return 0;
   func(context, png, len);
   STBIW_FREE(png);
   return 1;
}


/* ***************************************************************************
 *
 * JPEG writer
 *
 * This is based on Jon Olick's jo_jpeg.cpp:
 * public domain Simple, Minimalistic JPEG writer - http://www.jonolick.com/code.html
 */

static const unsigned char stbiw__jpg_ZigZag[] = { 0,1,5,6,14,15,27,28,2,4,7,13,16,26,29,42,3,8,12,17,25,30,41,43,9,11,18,
      24,31,40,44,53,10,19,23,32,39,45,52,54,20,22,33,38,46,51,55,60,21,34,37,47,50,56,59,61,35,36,48,49,57,58,62,63 };

static void stbiw__jpg_writeBits(stbi__write_context *s, int *bitBufP, int *bitCntP, const unsigned short *bs) {
   int bitBuf = *bitBufP, bitCnt = *bitCntP;
   bitCnt += bs[1];
   bitBuf |= bs[0] << (24 - bitCnt);
   while(bitCnt >= 8) {
      unsigned char c = (bitBuf >> 16) & 255;
      stbiw__putc(s, c);
      if(c == 255) {
         stbiw__putc(s, 0);
      }
      bitBuf <<= 8;
      bitCnt -= 8;
   }
   *bitBufP = bitBuf;
   *bitCntP = bitCnt;
}

static void stbiw__jpg_DCT(float *d0p, float *d1p, float *d2p, float *d3p, float *d4p, float *d5p, float *d6p, float *d7p) {
   float d0 = *d0p, d1 = *d1p, d2 = *d2p, d3 = *d3p, d4 = *d4p, d5 = *d5p, d6 = *d6p, d7 = *d7p;
   float z1, z2, z3, z4, z5, z11, z13;

   float tmp0 = d0 + d7;
   float tmp7 = d0 - d7;
   float tmp1 = d1 + d6;
   float tmp6 = d1 - d6;
   float tmp2 = d2 + d5;
   float tmp5 = d2 - d5;
   float tmp3 = d3 + d4;
   float tmp4 = d3 - d4;

   // Even part
   float tmp10 = tmp0 + tmp3;   // phase 2
   float tmp13 = tmp0 - tmp3;
   float tmp11 = tmp1 + tmp2;
   float tmp12 = tmp1 - tmp2;

   d0 = tmp10 + tmp11;       // phase 3
   d4 = tmp10 - tmp11;

   z1 = (tmp12 + tmp13) * 0.707106781f; // c4
   d2 = tmp13 + z1;       // phase 5
   d6 = tmp13 - z1;

   // Odd part
   tmp10 = tmp4 + tmp5;       // phase 2
   tmp11 = tmp5 + tmp6;
   tmp12 = tmp6 + tmp7;

   // The rotator is modified from fig 4-8 to avoid extra negations.
   z5 = (tmp10 - tmp12) * 0.382683433f; // c6
   z2 = tmp10 * 0.541196100f + z5; // c2-c6
   z4 = tmp12 * 1.306562965f + z5; // c2+c6
   z3 = tmp11 * 0.707106781f; // c4

   z11 = tmp7 + z3;      // phase 5
   z13 = tmp7 - z3;

   *d5p = z13 + z2;         // phase 6
   *d3p = z13 - z2;
   *d1p = z11 + z4;
   *d7p = z11 - z4;

   *d0p = d0;  *d2p = d2;  *d4p = d4;  *d6p = d6;
}

static void stbiw__jpg_calcBits(int val, unsigned short bits[2]) {
   int tmp1 = val < 0 ? -val : val;
   val = val < 0 ? val-1 : val;
   bits[1] = 1;
   while(tmp1 >>= 1) {
      ++bits[1];
   }
   bits[0] = val & ((1<<bits[1])-1);
}

static int stbiw__jpg_processDU(stbi__write_context *s, int *bitBuf, int *bitCnt, float *CDU, float *fdtbl, int DC, const unsigned short HTDC[256][2], const unsigned short HTAC[256][2]) {
   const unsigned short EOB[2] = { HTAC[0x00][0], HTAC[0x00][1] };
   const unsigned short M16zeroes[2] = { HTAC[0xF0][0], HTAC[0xF0][1] };
   int dataOff, i, diff, end0pos;
   int DU[64];

   // DCT rows
   for(dataOff=0; dataOff<64; dataOff+=8) {
      stbiw__jpg_DCT(&CDU[dataOff], &CDU[dataOff+1], &CDU[dataOff+2], &CDU[dataOff+3], &CDU[dataOff+4], &CDU[dataOff+5], &CDU[dataOff+6], &CDU[dataOff+7]);
   }
   // DCT columns
   for(dataOff=0; dataOff<8; ++dataOff) {
      stbiw__jpg_DCT(&CDU[dataOff], &CDU[dataOff+8], &CDU[dataOff+16], &CDU[dataOff+24], &CDU[dataOff+32], &CDU[dataOff+40], &CDU[dataOff+48], &CDU[dataOff+56]);
   }
   // Quantize/descale/zigzag the coefficients
   for(i=0; i<64; ++i) {
      float v = CDU[i]*fdtbl[i];
      // DU[stbiw__jpg_ZigZag[i]] = (int)(v < 0 ? ceilf(v - 0.5f) : floorf(v + 0.5f));
      // ceilf() and floorf() are C99, not C89, but I /think/ they're not needed here anyway?
      DU[stbiw__jpg_ZigZag[i]] = (int)(v < 0 ? v - 0.5f : v + 0.5f);
   }

   // Encode DC
   diff = DU[0] - DC;
   if (diff == 0) {
      stbiw__jpg_writeBits(s, bitBuf, bitCnt, HTDC[0]);
   } else {
      unsigned short bits[2];
      stbiw__jpg_calcBits(diff, bits);
      stbiw__jpg_writeBits(s, bitBuf, bitCnt, HTDC[bits[1]]);
      stbiw__jpg_writeBits(s, bitBuf, bitCnt, bits);
   }
   // Encode ACs
   end0pos = 63;
   for(; (end0pos>0)&&(DU[end0pos]==0); --end0pos) {
   }
   // end0pos = first element in reverse order !=0
   if(end0pos == 0) {
      stbiw__jpg_writeBits(s, bitBuf, bitCnt, EOB);
      return DU[0];
   }
   for(i = 1; i <= end0pos; ++i) {
      int startpos = i;
      int nrzeroes;
      unsigned short bits[2];
      for (; DU[i]==0 && i<=end0pos; ++i) {
      }
      nrzeroes = i-startpos;
      if ( nrzeroes >= 16 ) {
         int lng = nrzeroes>>4;
         int nrmarker;
         for (nrmarker=1; nrmarker <= lng; ++nrmarker)
            stbiw__jpg_writeBits(s, bitBuf, bitCnt, M16zeroes);
         nrzeroes &= 15;
      }
      stbiw__jpg_calcBits(DU[i], bits);
      stbiw__jpg_writeBits(s, bitBuf, bitCnt, HTAC[(nrzeroes<<4)+bits[1]]);
      stbiw__jpg_writeBits(s, bitBuf, bitCnt, bits);
   }
   if(end0pos != 63) {
      stbiw__jpg_writeBits(s, bitBuf, bitCnt, EOB);
   }
   return DU[0];
}

static int stbi_write_jpg_core(stbi__write_context *s, int width, int height, int comp, const void* data, int quality) {
   // Constants that don't pollute global namespace
   static const unsigned char std_dc_luminance_nrcodes[] = {0,0,1,5,1,1,1,1,1,1,0,0,0,0,0,0,0};
   static const unsigned char std_dc_luminance_values[] = {0,1,2,3,4,5,6,7,8,9,10,11};
   static const unsigned char std_ac_luminance_nrcodes[] = {0,0,2,1,3,3,2,4,3,5,5,4,4,0,0,1,0x7d};
   static const unsigned char std_ac_luminance_values[] = {
      0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,0x21,0x31,0x41,0x06,0x13,0x51,0x61,0x07,0x22,0x71,0x14,0x32,0x81,0x91,0xa1,0x08,
      0x23,0x42,0xb1,0xc1,0x15,0x52,0xd1,0xf0,0x24,0x33,0x62,0x72,0x82,0x09,0x0a,0x16,0x17,0x18,0x19,0x1a,0x25,0x26,0x27,0x28,
      0x29,0x2a,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x53,0x54,0x55,0x56,0x57,0x58,0x59,
      0x5a,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x83,0x84,0x85,0x86,0x87,0x88,0x89,
      0x8a,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xb2,0xb3,0xb4,0xb5,0xb6,
      0xb7,0xb8,0xb9,0xba,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,0xe1,0xe2,
      0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa
   };
   static const unsigned char std_dc_chrominance_nrcodes[] = {0,0,3,1,1,1,1,1,1,1,1,1,0,0,0,0,0};
   static const unsigned char std_dc_chrominance_values[] = {0,1,2,3,4,5,6,7,8,9,10,11};
   static const unsigned char std_ac_chrominance_nrcodes[] = {0,0,2,1,2,4,4,3,4,7,5,4,4,0,1,2,0x77};
   static const unsigned char std_ac_chrominance_values[] = {
      0x00,0x01,0x02,0x03,0x11,0x04,0x05,0x21,0x31,0x06,0x12,0x41,0x51,0x07,0x61,0x71,0x13,0x22,0x32,0x81,0x08,0x14,0x42,0x91,
      0xa1,0xb1,0xc1,0x09,0x23,0x33,0x52,0xf0,0x15,0x62,0x72,0xd1,0x0a,0x16,0x24,0x34,0xe1,0x25,0xf1,0x17,0x18,0x19,0x1a,0x26,
      0x27,0x28,0x29,0x2a,0x35,0x36,0x37,0x38,0x39,0x3a,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x53,0x54,0x55,0x56,0x57,0x58,
      0x59,0x5a,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x82,0x83,0x84,0x85,0x86,0x87,
      0x88,0x89,0x8a,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xb2,0xb3,0xb4,
      0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,0xda,
      0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa
   };
   // Huffman tables
   static const unsigned short YDC_HT[256][2] = { {0,2},{2,3},{3,3},{4,3},{5,3},{6,3},{14,4},{30,5},{62,6},{126,7},{254,8},{510,9}};
   static const unsigned short UVDC_HT[256][2] = { {0,2},{1,2},{2,2},{6,3},{14,4},{30,5},{62,6},{126,7},{254,8},{510,9},{1022,10},{2046,11}};
   static const unsigned short YAC_HT[256][2] = {
      {10,4},{0,2},{1,2},{4,3},{11,4},{26,5},{120,7},{248,8},{1014,10},{65410,16},{65411,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {12,4},{27,5},{121,7},{502,9},{2038,11},{65412,16},{65413,16},{65414,16},{65415,16},{65416,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {28,5},{249,8},{1015,10},{4084,12},{65417,16},{65418,16},{65419,16},{65420,16},{65421,16},{65422,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {58,6},{503,9},{4085,12},{65423,16},{65424,16},{65425,16},{65426,16},{65427,16},{65428,16},{65429,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {59,6},{1016,10},{65430,16},{65431,16},{65432,16},{65433,16},{65434,16},{65435,16},{65436,16},{65437,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {122,7},{2039,11},{65438,16},{65439,16},{65440,16},{65441,16},{65442,16},{65443,16},{65444,16},{65445,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {123,7},{4086,12},{65446,16},{65447,16},{65448,16},{65449,16},{65450,16},{65451,16},{65452,16},{65453,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {250,8},{4087,12},{65454,16},{65455,16},{65456,16},{65457,16},{65458,16},{65459,16},{65460,16},{65461,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {504,9},{32704,15},{65462,16},{65463,16},{65464,16},{65465,16},{65466,16},{65467,16},{65468,16},{65469,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {505,9},{65470,16},{65471,16},{65472,16},{65473,16},{65474,16},{65475,16},{65476,16},{65477,16},{65478,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {506,9},{65479,16},{65480,16},{65481,16},{65482,16},{65483,16},{65484,16},{65485,16},{65486,16},{65487,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {1017,10},{65488,16},{65489,16},{65490,16},{65491,16},{65492,16},{65493,16},{65494,16},{65495,16},{65496,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {1018,10},{65497,16},{65498,16},{65499,16},{65500,16},{65501,16},{65502,16},{65503,16},{65504,16},{65505,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {2040,11},{65506,16},{65507,16},{65508,16},{65509,16},{65510,16},{65511,16},{65512,16},{65513,16},{65514,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {65515,16},{65516,16},{65517,16},{65518,16},{65519,16},{65520,16},{65521,16},{65522,16},{65523,16},{65524,16},{0,0},{0,0},{0,0},{0,0},{0,0},
      {2041,11},{65525,16},{65526,16},{65527,16},{65528,16},{65529,16},{65530,16},{65531,16},{65532,16},{65533,16},{65534,16},{0,0},{0,0},{0,0},{0,0},{0,0}
   };
   static const unsigned short UVAC_HT[256][2] = {
      {0,2},{1,2},{4,3},{10,4},{24,5},{25,5},{56,6},{120,7},{500,9},{1014,10},{4084,12},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {11,4},{57,6},{246,8},{501,9},{2038,11},{4085,12},{65416,16},{65417,16},{65418,16},{65419,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {26,5},{247,8},{1015,10},{4086,12},{32706,15},{65420,16},{65421,16},{65422,16},{65423,16},{65424,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {27,5},{248,8},{1016,10},{4087,12},{65425,16},{65426,16},{65427,16},{65428,16},{65429,16},{65430,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {58,6},{502,9},{65431,16},{65432,16},{65433,16},{65434,16},{65435,16},{65436,16},{65437,16},{65438,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {59,6},{1017,10},{65439,16},{65440,16},{65441,16},{65442,16},{65443,16},{65444,16},{65445,16},{65446,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {121,7},{2039,11},{65447,16},{65448,16},{65449,16},{65450,16},{65451,16},{65452,16},{65453,16},{65454,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {122,7},{2040,11},{65455,16},{65456,16},{65457,16},{65458,16},{65459,16},{65460,16},{65461,16},{65462,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {249,8},{65463,16},{65464,16},{65465,16},{65466,16},{65467,16},{65468,16},{65469,16},{65470,16},{65471,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {503,9},{65472,16},{65473,16},{65474,16},{65475,16},{65476,16},{65477,16},{65478,16},{65479,16},{65480,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {504,9},{65481,16},{65482,16},{65483,16},{65484,16},{65485,16},{65486,16},{65487,16},{65488,16},{65489,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {505,9},{65490,16},{65491,16},{65492,16},{65493,16},{65494,16},{65495,16},{65496,16},{65497,16},{65498,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {506,9},{65499,16},{65500,16},{65501,16},{65502,16},{65503,16},{65504,16},{65505,16},{65506,16},{65507,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {2041,11},{65508,16},{65509,16},{65510,16},{65511,16},{65512,16},{65513,16},{65514,16},{65515,16},{65516,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
      {16352,14},{65517,16},{65518,16},{65519,16},{65520,16},{65521,16},{65522,16},{65523,16},{65524,16},{65525,16},{0,0},{0,0},{0,0},{0,0},{0,0},
      {1018,10},{32707,15},{65526,16},{65527,16},{65528,16},{65529,16},{65530,16},{65531,16},{65532,16},{65533,16},{65534,16},{0,0},{0,0},{0,0},{0,0},{0,0}
   };
   static const int YQT[] = {16,11,10,16,24,40,51,61,12,12,14,19,26,58,60,55,14,13,16,24,40,57,69,56,14,17,22,29,51,87,80,62,18,22,
                             37,56,68,109,103,77,24,35,55,64,81,104,113,92,49,64,78,87,103,121,120,101,72,92,95,98,112,100,103,99};
   static const int UVQT[] = {17,18,24,47,99,99,99,99,18,21,26,66,99,99,99,99,24,26,56,99,99,99,99,99,47,66,99,99,99,99,99,99,
                              99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99};
   static const float aasf[] = { 1.0f * 2.828427125f, 1.387039845f * 2.828427125f, 1.306562965f * 2.828427125f, 1.175875602f * 2.828427125f,
                                 1.0f * 2.828427125f, 0.785694958f * 2.828427125f, 0.541196100f * 2.828427125f, 0.275899379f * 2.828427125f };

   int row, col, i, k;
   float fdtbl_Y[64], fdtbl_UV[64];
   unsigned char YTable[64], UVTable[64];

   if(!data || !width || !height || comp > 4 || comp < 1) {
      return 0;
   }

   quality = quality ? quality : 90;
   quality = quality < 1 ? 1 : quality > 100 ? 100 : quality;
   quality = quality < 50 ? 5000 / quality : 200 - quality * 2;

   for(i = 0; i < 64; ++i) {
      int uvti, yti = (YQT[i]*quality+50)/100;
      YTable[stbiw__jpg_ZigZag[i]] = (unsigned char) (yti < 1 ? 1 : yti > 255 ? 255 : yti);
      uvti = (UVQT[i]*quality+50)/100;
      UVTable[stbiw__jpg_ZigZag[i]] = (unsigned char) (uvti < 1 ? 1 : uvti > 255 ? 255 : uvti);
   }

   for(row = 0, k = 0; row < 8; ++row) {
      for(col = 0; col < 8; ++col, ++k) {
         fdtbl_Y[k]  = 1 / (YTable [stbiw__jpg_ZigZag[k]] * aasf[row] * aasf[col]);
         fdtbl_UV[k] = 1 / (UVTable[stbiw__jpg_ZigZag[k]] * aasf[row] * aasf[col]);
      }
   }

   // Write Headers
   {
      static const unsigned char head0[] = { 0xFF,0xD8,0xFF,0xE0,0,0x10,'J','F','I','F',0,1,1,0,0,1,0,1,0,0,0xFF,0xDB,0,0x84,0 };
      static const unsigned char head2[] = { 0xFF,0xDA,0,0xC,3,1,0,2,0x11,3,0x11,0,0x3F,0 };
      const unsigned char head1[] = { 0xFF,0xC0,0,0x11,8,(unsigned char)(height>>8),STBIW_UCHAR(height),(unsigned char)(width>>8),STBIW_UCHAR(width),
                                      3,1,0x11,0,2,0x11,1,3,0x11,1,0xFF,0xC4,0x01,0xA2,0 };
      s->func(s->context, (void*)head0, sizeof(head0));
      s->func(s->context, (void*)YTable, sizeof(YTable));
      stbiw__putc(s, 1);
      s->func(s->context, UVTable, sizeof(UVTable));
      s->func(s->context, (void*)head1, sizeof(head1));
      s->func(s->context, (void*)(std_dc_luminance_nrcodes+1), sizeof(std_dc_luminance_nrcodes)-1);
      s->func(s->context, (void*)std_dc_luminance_values, sizeof(std_dc_luminance_values));
      stbiw__putc(s, 0x10); // HTYACinfo
      s->func(s->context, (void*)(std_ac_luminance_nrcodes+1), sizeof(std_ac_luminance_nrcodes)-1);
      s->func(s->context, (void*)std_ac_luminance_values, sizeof(std_ac_luminance_values));
      stbiw__putc(s, 1); // HTUDCinfo
      s->func(s->context, (void*)(std_dc_chrominance_nrcodes+1), sizeof(std_dc_chrominance_nrcodes)-1);
      s->func(s->context, (void*)std_dc_chrominance_values, sizeof(std_dc_chrominance_values));
      stbiw__putc(s, 0x11); // HTUACinfo
      s->func(s->context, (void*)(std_ac_chrominance_nrcodes+1), sizeof(std_ac_chrominance_nrcodes)-1);
      s->func(s->context, (void*)std_ac_chrominance_values, sizeof(std_ac_chrominance_values));
      s->func(s->context, (void*)head2, sizeof(head2));
   }

   // Encode 8x8 macroblocks
   {
      static const unsigned short fillBits[] = {0x7F, 7};
      const unsigned char *imageData = (const unsigned char *)data;
      int DCY=0, DCU=0, DCV=0;
      int bitBuf=0, bitCnt=0;
      // comp == 2 is grey+alpha (alpha is ignored)
      int ofsG = comp > 2 ? 1 : 0, ofsB = comp > 2 ? 2 : 0;
      int x, y, pos;
      for(y = 0; y < height; y += 8) {
         for(x = 0; x < width; x += 8) {
            float YDU[64], UDU[64], VDU[64];
            for(row = y, pos = 0; row < y+8; ++row) {
               for(col = x; col < x+8; ++col, ++pos) {
                  int p = (stbi__flip_vertically_on_write ? height-1-row : row)*width*comp + col*comp;
                  float r, g, b;
                  if(row >= height) {
                     p -= width*comp*(row+1 - height);
                  }
                  if(col >= width) {
                     p -= comp*(col+1 - width);
                  }

                  r = imageData[p+0];
                  g = imageData[p+ofsG];
                  b = imageData[p+ofsB];
                  YDU[pos]=+0.29900f*r+0.58700f*g+0.11400f*b-128;
                  UDU[pos]=-0.16874f*r-0.33126f*g+0.50000f*b;
                  VDU[pos]=+0.50000f*r-0.41869f*g-0.08131f*b;
               }
            }

            DCY = stbiw__jpg_processDU(s, &bitBuf, &bitCnt, YDU, fdtbl_Y, DCY, YDC_HT, YAC_HT);
            DCU = stbiw__jpg_processDU(s, &bitBuf, &bitCnt, UDU, fdtbl_UV, DCU, UVDC_HT, UVAC_HT);
            DCV = stbiw__jpg_processDU(s, &bitBuf, &bitCnt, VDU, fdtbl_UV, DCV, UVDC_HT, UVAC_HT);
         }
      }

      // Do the bit alignment of the EOI marker
      stbiw__jpg_writeBits(s, &bitBuf, &bitCnt, fillBits);
   }

   // EOI
   stbiw__putc(s, 0xFF);
   stbiw__putc(s, 0xD9);

   return 1;
}

STBIWDEF int stbi_write_jpg_to_func(stbi_write_func *func, void *context, int x, int y, int comp, const void *data, int quality)
{
   stbi__write_context s;
   stbi__start_write_callbacks(&s, func, context);
   return stbi_write_jpg_core(&s, x, y, comp, (void *) data, quality);
}


#ifndef STBI_WRITE_NO_STDIO
STBIWDEF int stbi_write_jpg(char const *filename, int x, int y, int comp, const void *data, int quality)
{
   stbi__write_context s;
   if (stbi__start_write_file(&s,filename)) {
      int r = stbi_write_jpg_core(&s, x, y, comp, data, quality);
      stbi__end_write_file(&s);
      return r;
   } else
      return 0;
}
#endif

#endif // STB_IMAGE_WRITE_IMPLEMENTATION

/* Revision history
      1.09  (2018-02-11)
             fix typo in zlib quality API, improve STB_I_W_STATIC in C++
      1.08  (2018-01-29)
             add stbi__flip_vertically_on_write, external zlib, zlib quality, choose PNG filter
      1.07  (2017-07-24)
             doc fix
      1.06 (2017-07-23)
             writing JPEG (using Jon Olick's code)
      1.05   ???
      1.04 (2017-03-03)
             monochrome BMP expansion
      1.03   ???
      1.02 (2016-04-02)
             avoid allocating large structures on the stack
      1.01 (2016-01-16)
             STBIW_REALLOC_SIZED: support allocators with no realloc support
             avoid race-condition in crc initialization
             minor compile issues
      1.00 (2015-09-14)
             installable file IO function
      0.99 (2015-09-13)
             warning fixes; TGA rle support
      0.98 (2015-04-08)
             added STBIW_MALLOC, STBIW_ASSERT etc
      0.97 (2015-01-18)
             fixed HDR asserts, rewrote HDR rle logic
      0.96 (2015-01-17)
             add HDR output
             fix monochrome BMP
      0.95 (2014-08-17)
		       add monochrome TGA output
      0.94 (2014-05-31)
             rename private functions to avoid conflicts with stb_image.h
      0.93 (2014-05-27)
             warning fixes
      0.92 (2010-08-01)
             casts to unsigned char to fix warnings
      0.91 (2010-07-17)
             first public release
      0.90   first internal release
*/

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2017 Sean Barrett
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/
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

#ifndef system_paths_h
#define system_paths_h

#include <stdio.h>

#ifdef _WIN32
#define PATH_SEPARATOR "\\"
#define PATH_SEPARATOR_CHAR '\\'
#else
#define PATH_SEPARATOR "/"
#define PATH_SEPARATOR_CHAR '/'
#endif

void desktop_path(char *buffer, size_t size);
FILE* fopen_utf8(const char* filename, const char* mode);

#endif /* system_paths_h */
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

#ifndef utils_h
#define utils_h

#include <stdio.h>
#include <stdbool.h>

void displayName(const char *filename, char *destination, size_t size);
bool hasPostfix(const char *string, const char *postfix);

#endif /* utils_h */
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

#ifndef main_h
#define main_h

#include <stdbool.h>

enum MainState {
    MainStateUndefined,
    MainStateBootIntro,
    MainStateRunningProgram,
    MainStateRunningTool,
    MainStateDevMenu,
};

enum Zoom {
    ZoomPixelPerfect,
    ZoomLarge,
    ZoomOverscan,
    ZoomSqueeze,
};

void bootNX(void);
void rebootNX(void);
bool hasProgram(void);
const char *getMainProgramFilename(void);
void selectProgram(const char *filename);
void runMainProgram(void);
void runToolProgram(const char *filename);
void showDevMenu(void);
bool usesMainProgramAsDisk(void);
void getDiskFilename(char *outputString);
void getRamFilename(char *outputString);
void setMouseEnabled(bool enabled);

#endif /* main_h */
