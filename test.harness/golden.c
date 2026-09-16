// Copyright 2016-2024 Timo Kloss
// Copyright 2021-2026 Martin Mauchauffée

// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.

// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:

// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "golden.h"
#include "audio_chip.h"
#include "core.h"
#include "video_chip.h"
#include <stdlib.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define FNV_OFFSET_BASIS 0xcbf29ce484222325ULL
#define FNV_PRIME 0x100000001b3ULL

static uint64_t hashByte(uint64_t hash, uint8_t byte)
{
	return (hash ^ byte) * FNV_PRIME;
}

uint64_t golden_hashFrame(struct Core *core, uint32_t *scratchPixels)
{
	video_renderScreen(core, scratchPixels, SCREEN_WIDTH * (int)sizeof(uint32_t));

	uint64_t hash = FNV_OFFSET_BASIS;
	for(int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
	{
		uint32_t pixel = scratchPixels[i];
		// same byte order switch as writeImage below: the hash must not depend on ABGR
#if ABGR
		uint8_t r = (pixel) & 0xFF;
		uint8_t g = (pixel >> 8) & 0xFF;
		uint8_t b = (pixel >> 16) & 0xFF;
#else
		uint8_t r = (pixel >> 16) & 0xFF;
		uint8_t g = (pixel >> 8) & 0xFF;
		uint8_t b = (pixel) & 0xFF;
#endif
		hash = hashByte(hash, r);
		hash = hashByte(hash, g);
		hash = hashByte(hash, b);
	}
	return hash;
}

uint64_t golden_hashAudio(struct Core *core, int16_t *scratchSamples)
{
	audio_renderAudio(core, scratchSamples, GOLDEN_AUDIO_SAMPLES, 44100, 0);

	uint64_t hash = FNV_OFFSET_BASIS;
	for(int i = 0; i < GOLDEN_AUDIO_SAMPLES; i++)
	{
		uint16_t sample = (uint16_t)scratchSamples[i];
		hash = hashByte(hash, sample & 0xFF);
		hash = hashByte(hash, (sample >> 8) & 0xFF);
	}
	return hash;
}

bool golden_writePng(const char *filename, uint32_t *pixels)
{
	return writeImage(filename, SCREEN_WIDTH, SCREEN_HEIGHT, pixels, SCREEN_WIDTH * (int)sizeof(uint32_t), 1);
}

bool writeImage(const char *filename, int width, int height, uint32_t *pixels, int pitch, int scale)
{
	// pitch is the source row stride in bytes, which is not always
	// width * 4: SDL_LockTexture pads rows (Direct3D on Windows notably),
	// so the source buffer must be walked with its own stride
	const int srcWidth = pitch / (int)sizeof(uint32_t);

	uint8_t *data = malloc(width * height * 3 * scale * scale);
	if(data)
	{
		int i = 0;
		for(int y = 0; y < height; y++)
		{
			for(int ys = 0; ys < scale; ys++)
			{
				for(int x = 0; x < width; x++)
				{
					uint32_t pixel = pixels[y * srcWidth + x];
					// stbi_write_png with comp=3 wants R,G,B. The engine's word layout
					// depends on ABGR (see machine/video_chip.h): 0xAARRGGBB when 0,
					// 0xAABBGGRR when 1.
#if ABGR
					uint8_t r = (pixel) & 0xFF;
					uint8_t g = (pixel >> 8) & 0xFF;
					uint8_t b = (pixel >> 16) & 0xFF;
#else
					uint8_t r = (pixel >> 16) & 0xFF;
					uint8_t g = (pixel >> 8) & 0xFF;
					uint8_t b = (pixel) & 0xFF;
#endif
					for(int xs = 0; xs < scale; xs++)
					{
						data[i++] = r;
						data[i++] = g;
						data[i++] = b;
					}
				}
			}
		}

		int result = stbi_write_png(filename, width * scale, height * scale, 3, data, width * 3 * scale);
		free(data);

		return (result != 0);
	}
	return false;
}
