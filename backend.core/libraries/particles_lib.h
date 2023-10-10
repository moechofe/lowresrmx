#ifndef particles_lib_h
#define particles_lib_h

#include <stdbool.h>

#include "tokenizer.h"
#include "video_chip.h"

#define EMITTER_MAX 16
#define APPEARANCE_MAX 24
#define SPAWNER_MAX 16

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
