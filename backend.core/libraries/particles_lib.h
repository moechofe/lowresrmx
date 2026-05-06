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

#ifndef particles_lib_h
#define particles_lib_h

#include <stdbool.h>

#include "tokenizer.h"
#include "video_chip.h"

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

void prtclib_setupPool(struct ParticlesLib *lib, int firstSprite, int poolCount, int particleAddr);
// void prtclib_setApperanceLabel(struct ParticlesLib *lib,int apperanceId,struct Token *label);

void prtclib_setupEmitter(struct ParticlesLib *lib, int poolCount, int particleAddr);
void prtclib_setSpawnerLabel(struct ParticlesLib *lib, int emitterId, struct Token *label);

void prtclib_spawn(struct ParticlesLib *lib, int emitterId, float posX, float posY);
void prtclib_stop(struct ParticlesLib *lib, int emitterId);

void prtclib_update(struct Core *core, struct ParticlesLib *lib);
void prtclib_interrupt(struct Core *core, struct ParticlesLib *lib);
void prtclib_clear(struct Core *core, struct ParticlesLib *lib);

#endif
