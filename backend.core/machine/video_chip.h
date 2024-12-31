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
#define BGR 1
#endif
#endif
#if __ANDROID__
#define BGR 0
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
