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

#include "video_chip.h"
#include "core.h"
#include <string.h>

#define OVERLAY_FLAG (1<<6)

// // FAMICUBE
// uint32_t better_palette[]={ 0x000000, 0xe03c28, 0xffffff, 0xd7d7d7, 0xa8a8a8, 0x7b7b7b, 0x343434, 0x151515, 0x0d2030, 0x415d66, 0x71a6a1, 0xbdffca, 0x25e2cd, 0x0a98ac, 0x005280, 0x00604b, 0x20b562, 0x58d332, 0x139d08, 0x004e00, 0x172808, 0x376d03, 0x6ab417, 0x8cd612, 0xbeeb71, 0xeeffa9, 0xb6c121, 0x939717, 0xcc8f15, 0xffbb31, 0xffe737, 0xf68f37, 0xad4e1a, 0x231712, 0x5c3c0d, 0xae6c37, 0xc59782, 0xe2d7b5, 0x4f1507, 0x823c3d, 0xda655e, 0xe18289, 0xf5b784, 0xffe9c5, 0xff82ce, 0xcf3c71, 0x871646, 0xa328b3, 0xcc69e4, 0xd59cfc, 0xfec9ed, 0xe2c9ff, 0xa675fe, 0x6a31ca, 0x5a1991, 0x211640, 0x3d34a5, 0x6264dc, 0x9ba0ef, 0x98dcff, 0x5ba8ff, 0x0a89ff, 0x024aca, 0x00177d, };

// FAMICUBE
uint32_t better_palette[]={ 0xff000000, 0xffe03c28, 0xffffffff, 0xffd7d7d7, 0xffa8a8a8, 0xff7b7b7b, 0xff343434, 0xff151515, 0xff0d2030, 0xff415d66, 0xff71a6a1, 0xffbdffca, 0xff25e2cd, 0xff0a98ac, 0xff005280, 0xff00604b, 0xff20b562, 0xff58d332, 0xff139d08, 0xff004e00, 0xff172808, 0xff376d03, 0xff6ab417, 0xff8cd612, 0xffbeeb71, 0xffeeffa9, 0xffb6c121, 0xff939717, 0xffcc8f15, 0xffffbb31, 0xffffe737, 0xfff68f37, 0xffad4e1a, 0xff231712, 0xff5c3c0d, 0xffae6c37, 0xffc59782, 0xffe2d7b5, 0xff4f1507, 0xff823c3d, 0xffda655e, 0xffe18289, 0xfff5b784, 0xffffe9c5, 0xffff82ce, 0xffcf3c71, 0xff871646, 0xffa328b3, 0xffcc69e4, 0xffd59cfc, 0xfffec9ed, 0xffe2c9ff, 0xffa675fe, 0xff6a31ca, 0xff5a1991, 0xff211640, 0xff3d34a5, 0xff6264dc, 0xff9ba0ef, 0xff98dcff, 0xff5ba8ff, 0xff0a89ff, 0xff024aca, 0xff00177d, };

int video_getCharacterPixel(struct Character *character, int x, int y)
{
    int b0 = (character->data[y] >> (7 - x)) & 0x01;
    int b1 = (character->data[y | 8] >> (7 - x)) & 0x01;
    return b0 | (b1 << 1);
}

void video_renderPlane(struct Character *characters, struct Plane *plane, int sizeMode, int y, int scrollX, int scrollY, int pixelFlag, uint8_t *scanlineBuffer, bool opaque)
{
    int divShift = sizeMode ? 4 : 3;
    int planeY = y + scrollY;
    int row = (planeY >> divShift) & ROWS_MASK;
    int cellY = planeY & 7;

    int x = 0;
    int b = 0;
    uint8_t d0 = 0;
    uint8_t d1 = 0;
    uint8_t pal = 0;
    uint8_t pri = 0;

    int pre = scrollX & 7;

    while (x < SCREEN_WIDTH)
    {
        if (!b)
        {
            int planeX = x + scrollX;
            int column = (planeX >> divShift) & COLS_MASK;
            struct Cell *cell = &plane->cells[row][column];

            int index = cell->character;
            if (sizeMode)
            {
                index += (cell->attr.flipX ? (planeX >> 3) + 1 : planeX >> 3) & 1;
                index += ((cell->attr.flipY ? (planeY >> 3) + 1 : planeY >> 3) & 1) << 4;
            }

            struct Character *character = &characters[index];
            pal = cell->attr.palette << 2;
            pri = cell->attr.priority << 7;

            int fcy = cell->attr.flipY ? (7 - cellY) : cellY;
            d0 = character->data[fcy];
            d1 = character->data[fcy | 8];
            if (cell->attr.flipX)
            {
                // reverse bits hack from http://graphics.stanford.edu/~seander/bithacks.html#ReverseByteWith32Bits
                d0 = ((d0 * 0x0802LU & 0x22110LU) | (d0 * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;
                d1 = ((d1 * 0x0802LU & 0x22110LU) | (d1 * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;
            }
        }

        if (pre)
        {
            --pre;
        }
        else
        {
            if (pri >= (*scanlineBuffer >> 7))
            {
                int pixel = ((d0 >> 7) & 1) | ((d1 >> 6) & 2);
								// if color is not zero, replace it by the new color, I guess
								if (pixel || (pixel==0 && opaque))
                {
                    *scanlineBuffer = pixel | pal | pri | pixelFlag;
                }
            }
            ++scanlineBuffer;
            ++x;
        }

        d0 <<= 1;
        d1 <<= 1;
        b = (b + 1) & 7;
    }
}

void video_renderSprites(struct SpriteRegisters *reg, struct VideoRam *ram, int y, uint8_t *scanlineBuffer, uint8_t *scanlineSpriteBuffer)
{
    for (int i = NUM_SPRITES - 1; i >= 0; i--)
    {
        struct Sprite *sprite = &reg->sprites[i];
        if ((sprite->x/16) != 0 || (sprite->y/16) != 0)
        {
            int spriteY = y - (sprite->y/16) + SPRITE_OFFSET_Y;
            int size = (sprite->attr.size + 1) << 3;
            if (spriteY >= 0 && spriteY < size)
            {
                if (sprite->attr.flipY)
                {
                    spriteY = size - spriteY - 1;
                }
                int charIndex = sprite->character + ((spriteY >> 3) << 4);
                if (sprite->attr.flipX)
                {
                    charIndex += sprite->attr.size;
                }
                int minX = (sprite->x/16) - SPRITE_OFFSET_X;
                int maxX = minX + size;
                if (minX < 0)
                {
                    int skip = -minX >> 3;
                    if (sprite->attr.flipX)
                    {
                        charIndex -= skip;
                    }
                    else
                    {
                        charIndex += skip;
                    }
                }
                if (minX < 0) minX = 0;
                if (maxX > SCREEN_WIDTH) maxX = SCREEN_WIDTH;
                uint8_t *buffer = &scanlineSpriteBuffer[minX];
                int spriteX = minX - (int)(sprite->x/16) + SPRITE_OFFSET_X;
                if (sprite->attr.flipX)
                {
                    spriteX = size - spriteX - 1;
                }
                struct Character *character = &ram->characters[charIndex];
                for (int x = minX; x < maxX; x++)
                {
                    int pixel = video_getCharacterPixel(character, spriteX & 0x07, spriteY & 0x07);
                    if (pixel)
                    {
                        *buffer = pixel | (sprite->attr.palette << 2) | (sprite->attr.priority << 7);
                    }
                    buffer++;
                    if (sprite->attr.flipX)
                    {
                        if (!(spriteX & 0x07))
                        {
                            character--;
                        }
                        spriteX--;
                    }
                    else
                    {
                        spriteX++;
                        if (!(spriteX & 0x07))
                        {
                            character++;
                        }
                    }
                }
            }
        }
    }
    for (int x = 0; x < SCREEN_WIDTH; x++)
    {
        int pixel = *scanlineSpriteBuffer;
        if (pixel && (pixel >> 7) >= (*scanlineBuffer >> 7))
        {
            *scanlineBuffer = pixel;
        }
        scanlineSpriteBuffer++;
        scanlineBuffer++;
    }
}

void video_renderScreen(struct Core *core, uint32_t *outputRGB)
{
    uint8_t scanlineBuffer[SCREEN_WIDTH];
    uint8_t scanlineSpriteBuffer[SCREEN_WIDTH];
    uint32_t *outputPixel = outputRGB;

    struct VideoRam *ram = &core->machine->videoRam;
    struct VideoRegisters *reg = &core->machine->videoRegisters;
    struct SpriteRegisters *sreg = &core->machine->spriteRegisters;
    struct ColorRegisters *creg = &core->machine->colorRegisters;
    struct IORegisters *io = &core->machine->ioRegisters;
		struct MachineInternals *mi = core->machineInternals;

    int width=SCREEN_WIDTH;
    int height=SCREEN_HEIGHT;
    int skip_before=0;
    int skip_after=0;
    if (core->interpreter->compat)
    {
        int sw=io->shown.width!=0?io->shown.width:SCREEN_WIDTH;
        int sh=io->shown.height!=0?io->shown.height:SCREEN_HEIGHT;
        width=160;
        height=128;
        skip_before=(sw-width)/2;
        skip_after=SCREEN_WIDTH-width-skip_before;
        outputPixel+=SCREEN_WIDTH*(sh-height)/2;
    }
    for (int y = 0; y<height; y++)
    {
        reg->rasterLine = y;
        if (core->interpreter->compat && y>=0 && y<120)
        {
            itp_runInterrupt(core, InterruptTypeRaster);
        }
        else
        {
            itp_runInterrupt(core, InterruptTypeRaster);
        }
        memset(scanlineBuffer, 0, sizeof(scanlineBuffer));

        bool skip = (core->interpreter->interruptOverCycles > 0);
        if (!skip)
        {
            if (reg->attr.planeDEnabled)
            {
                int scrollX = reg->scrollDX; // | (reg->scrollMSB.dX << 8);
                int scrollY = reg->scrollDY; // | (reg->scrollMSB.dY << 8);
                video_renderPlane(ram->characters, &ram->planeD,
								0, //reg->attr.planeDCellSize,
								y, scrollX, scrollY, 0, scanlineBuffer,
								mi->planeColor0IsOpaque[3]);
            }
            if (reg->attr.planeCEnabled)
            {
                int scrollX = reg->scrollCX; // | (reg->scrollMSB.cX << 8);
                int scrollY = reg->scrollCY; // | (reg->scrollMSB.cY << 8);
                video_renderPlane(ram->characters, &ram->planeC,
								0, //reg->attr.planeCCellSize,
								y, scrollX, scrollY, 0, scanlineBuffer,
								mi->planeColor0IsOpaque[2]);
            }
            if (reg->attr.planeBEnabled)
            {
                int scrollX = reg->scrollBX; // | (reg->scrollMSB.bX << 8);
                int scrollY = reg->scrollBY; // | (reg->scrollMSB.bY << 8);
                video_renderPlane(ram->characters, &ram->planeB,
								0, //reg->attr.planeBCellSize,
								y, scrollX, scrollY, 0, scanlineBuffer,
								mi->planeColor0IsOpaque[1]);
            }
            if (reg->attr.planeAEnabled)
            {
                int scrollX = reg->scrollAX; // | (reg->scrollMSB.aX << 8);
                int scrollY = reg->scrollAY; // | (reg->scrollMSB.aY << 8);
                video_renderPlane(ram->characters, &ram->planeA,
								0, //reg->attr.planeACellSize,
								y, scrollX, scrollY, 0, scanlineBuffer,
								mi->planeColor0IsOpaque[0]);
            }
            if (reg->attr.spritesEnabled)
            {
                memset(scanlineSpriteBuffer, 0, sizeof(scanlineSpriteBuffer));
                video_renderSprites(sreg, ram, y, scanlineBuffer, scanlineSpriteBuffer);
            }
        }

        // overlay
        video_renderPlane((struct Character *)overlayCharacters, &core->overlay->plane, 0, y, 0, 0, OVERLAY_FLAG, scanlineBuffer, 0);

        outputPixel+=skip_before;

        for (int x = 0; x < width; x++)
        {
            int colorIndex = scanlineBuffer[x] & 0x1F;
            int color = (scanlineBuffer[x] & OVERLAY_FLAG) ? overlayColors[colorIndex] : skip ? 0 : creg->colors[colorIndex];

            uint32_t c = better_palette[color & 63];

#if BGR
            uint32_t a=(c>>24)&0xff;
            uint32_t r=(c>>16)&0xff;
            uint32_t g=(c>>8)&0xff;
            uint32_t b=c&0xff;
            c=(a<<24)|(b<<16)|(g<<8)|r;
#endif
            *outputPixel = c;
            ++outputPixel;
        }

        outputPixel+=skip_after;
    }
}
