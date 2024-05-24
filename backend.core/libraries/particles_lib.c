#include "libraries/particles_lib.h"

#include "core.h"
#include "machine/machine.h"
#include "pcg_basic.h"

#define _USE_MATH_DEFINES
#ifndef __USE_MISC
  #define __USE_MISC
#endif
#include <math.h>

static pcg32_random_t pcg;

void prtclib_setupPool(struct ParticlesLib *lib,int firstSprite,int poolCount,int particleAddr)
{
    lib->first_sprite_id=firstSprite;
    lib->pool_count=poolCount;
    lib->pool_next_id=0;
    lib->particles_data_addr=particleAddr;

    for(int particle_id=0;particle_id<lib->pool_count;++particle_id)
    {
			int particle = lib->particles_data_addr + particle_id*PARTICLE_MEM_SIZE; // 6 bytes

      machine_poke(lib->core, particle+EMITTER_MEM_DELAY, -1);
    }

    pcg32_srandom_r(&pcg, 20321911116532, (intptr_t)&pcg);
}

void prtclib_setApperanceLabel(struct ParticlesLib *lib,int apperanceId,struct Token *label)
{
    lib->apperances_label[apperanceId]=label;
}

void prtclib_setupEmitter(struct ParticlesLib *lib,int emitterCount,int emitterAddr)
{
    lib->emitters_count=emitterCount;
    lib->emitters_data_addr=emitterAddr;
}

void prtclib_setSpawnerLabel(struct ParticlesLib *lib,int emitterId,struct Token *label)
{
    lib->emitters_label[emitterId]=label;
}

void prtclib_spawn(struct ParticlesLib *lib,int emitterId,float posX,float posY)
{
    if(emitterId<0 && emitterId>=lib->emitters_count) return;

    int emitter = lib->emitters_data_addr + emitterId*EMITTER_MEM_SIZE; // 6 bytes

    machine_poke_short(lib->core, emitter+EMITTER_MEM_X, ((int16_t)posX*16)&0x1FFF);
    machine_poke_short(lib->core, emitter+EMITTER_MEM_Y, ((int16_t)posY*16)&0x1FFF);

    machine_poke(lib->core, emitter+EMITTER_MEM_DELAY, 0); // start with no delay

    uint8_t repeat = 1 + dat_readU8(lib->emitters_label[emitterId],EMITTER_DATA_REPEAT,0);
    machine_poke(lib->core, emitter+EMITTER_MEM_REPEAT, repeat);
}

void prtclib_stop(struct ParticlesLib *lib,int emitterId)
{
    if(emitterId<0 && emitterId>=lib->emitters_count) return;

    int emitter = lib->emitters_data_addr + emitterId*EMITTER_MEM_SIZE; // 6 bytes

    machine_poke(lib->core, emitter+EMITTER_MEM_REPEAT, 0);
}

void prtclib_update(struct Core *core, struct ParticlesLib *lib)
{
    struct Interpreter *interpreter = core->interpreter;

    struct Token *dataToken;

    // update emitters
    for(int emitter_id=0; emitter_id<lib->emitters_count; ++emitter_id)
    {
        int emitter = lib->emitters_data_addr + emitter_id*EMITTER_MEM_SIZE; // 6 bytes

        float emitter_pos_x=(float)machine_peek_short(lib->core, emitter+EMITTER_MEM_X)/16;
        float emitter_pos_y=(float)machine_peek_short(lib->core, emitter+EMITTER_MEM_Y)/16;

        // wait for delay to end
        int delay=machine_peek(lib->core, emitter+EMITTER_MEM_DELAY);
        if(delay>0)
        {
          machine_poke(lib->core, emitter+EMITTER_MEM_DELAY, delay-1);
          continue;
        }

        // is there more to repeat?
        int repeat=machine_peek(lib->core, emitter+EMITTER_MEM_REPEAT);
        if(repeat>0)
        {
          // emitter data
          uint8_t apperance_id = dat_readU8(lib->emitters_label[emitter_id],EMITTER_DATA_APPEARANCE,255);
          if (apperance_id>APPEARANCE_MAX) continue;

					float shape = dat_readFloat(lib->emitters_label[emitter_id],EMITTER_DATA_SHAPE,1);
					float outer = dat_readFloat(lib->emitters_label[emitter_id],EMITTER_DATA_OUTER,0);
          float inner = dat_readFloat(lib->emitters_label[emitter_id],EMITTER_DATA_INNER,0);
					float arc = dat_readFloat(lib->emitters_label[emitter_id],EMITTER_DATA_ARC,0);
					float rotation = dat_readFloat(lib->emitters_label[emitter_id],EMITTER_DATA_ROTATION,0);
          float speed_x = dat_readFloat(lib->emitters_label[emitter_id],EMITTER_DATA_SPEED_X,0);
          float speed_y = dat_readFloat(lib->emitters_label[emitter_id],EMITTER_DATA_SPEED_Y,0);
					float gravity = dat_readFloat(lib->emitters_label[emitter_id],EMITTER_DATA_GRAVITY,0);
          uint8_t count = dat_readU8(lib->emitters_label[emitter_id],EMITTER_DATA_COUNT,0);
          uint8_t delay = dat_readU8(lib->emitters_label[emitter_id],EMITTER_DATA_DELAY,0);

          // reduce repeat
          machine_poke(lib->core, emitter+EMITTER_MEM_REPEAT, repeat-1);

          // reset delay
          machine_poke(lib->core, emitter+EMITTER_MEM_DELAY, delay);

          for(int i=0; i<count; ++i)
          {
            // spawn a particle
            int particle_id = lib->pool_next_id;
            lib->pool_next_id = (lib->pool_next_id+1) % lib->pool_count;

            int particle = lib->particles_data_addr + particle_id*PARTICLE_MEM_SIZE; // 6 bytes

            int sprite_id=lib->first_sprite_id+particle_id;
            struct Sprite *spr=&lib->core->machine->spriteRegisters.sprites[sprite_id];

						float sprite_distance_x;
						float sprite_distance_y;
						int sign;

            // inner, outer ring
            if(outer>0 && outer>inner && shape!=0)
            {
							if(shape>0)
							{
									float angle=(float)ldexp(pcg32_random_r(&pcg),-32);
									float r=(float)sqrt(ldexp(pcg32_random_r(&pcg),-32));
									float diff=outer-inner;
									float space_x=r*(diff)+inner;
									float space_y=r*(diff)+inner;
									sprite_distance_x=cosf(angle*M_PI*2)*space_x;
									sprite_distance_y=sinf(angle*M_PI*2)*space_y*shape;
							}
							else if(inner==0)
							{
									sign=pcg32_boundedrand_r(&pcg,2)*2-1;
									sprite_distance_x=(float)pcg32_boundedrand_r(&pcg,outer)*sign;
									sign=pcg32_boundedrand_r(&pcg,2)*2-1;
									sprite_distance_y=(float)pcg32_boundedrand_r(&pcg,outer)*sign*-shape;
							}
							else
							{
									float x,y;
									int sign;
									float spectral=outer+outer*-shape;
									double choose=ldexp(pcg32_random_r(&pcg),-32)*spectral;
									if(choose<outer)
									{
										// horizontal
										sign=pcg32_boundedrand_r(&pcg,2)*2-1;
										sprite_distance_x=(float)pcg32_boundedrand_r(&pcg,outer*2)-outer;
										sprite_distance_y=((float)pcg32_boundedrand_r(&pcg,outer-inner)+inner)*sign*-shape;
									}
									else
									{
										// vertical
										sign=pcg32_boundedrand_r(&pcg,2)*2-1;
										sprite_distance_x=((float)pcg32_boundedrand_r(&pcg,outer-inner)+inner)*sign;
										sprite_distance_y=((float)pcg32_boundedrand_r(&pcg,outer*2)-outer)*-shape;
									}
							}
            }
						else
						{
							sprite_distance_x=0;
							sprite_distance_y=0;
						}

						if(gravity>0)
						{
							int a=0;
						}

						float vector_len = sqrtf(sprite_distance_x*sprite_distance_x+sprite_distance_y*sprite_distance_y);
						float sprite_norm_x=sprite_distance_x/vector_len;
						float sprite_norm_y=sprite_distance_y/vector_len;

						// position
						spr->x=(int)((emitter_pos_x + sprite_distance_x + SPRITE_OFFSET_X)*16) & 0x1FFF;
						spr->y=(int)((emitter_pos_y + sprite_distance_y + SPRITE_OFFSET_Y)*16) & 0x1FFF;

            // speed x,y
            machine_poke_short(lib->core, particle+PARTICLE_MEM_X, speed_x + sprite_norm_x*gravity);
            machine_poke_short(lib->core, particle+PARTICLE_MEM_Y, speed_y + sprite_norm_y*gravity);

            // apperance
            machine_poke(lib->core, particle+PARTICLE_MEM_APPEARANCE, apperance_id);
            machine_poke(lib->core, particle+PARTICLE_MEM_FRAME, 0);
          }
        }
    }

    // update particles
    for(int particle_id=0; particle_id<lib->pool_count; ++particle_id)
    {
        int particle = lib->particles_data_addr + particle_id*PARTICLE_MEM_SIZE; // 6 bytes

        // apperance is also used to disable the particle
        int apperance_id=machine_peek(lib->core, particle+PARTICLE_MEM_APPEARANCE);
        if(apperance_id>APPEARANCE_MAX) continue;

        // sprite
        int sprite_id=lib->first_sprite_id+particle_id;
        struct Sprite *spr=&lib->core->machine->spriteRegisters.sprites[sprite_id];

        // position x
        float speed_x=(float)machine_peek_short(lib->core, particle+PARTICLE_MEM_X);
        spr->x=(int)(spr->x+speed_x)&0x1FFF;

        // position y
        float speed_y=(float)machine_peek_short(lib->core, particle+PARTICLE_MEM_Y);
        spr->y=(int)(spr->y+speed_y)&0x1FFF;

        // character

        int step_id=machine_peek(lib->core, particle+PARTICLE_MEM_FRAME);

        float character=dat_readFloat(lib->apperances_label[apperance_id],step_id,0);
        if(character>=0)
        {
          spr->character=(uint8_t)character;
          machine_poke(lib->core, particle+PARTICLE_MEM_FRAME, (step_id+1)&0x1FFF);
        }
        else
        {
          step_id=step_id+(int)character;
          character=dat_readFloat(lib->apperances_label[apperance_id],step_id,0);
          if(character>=0)
          {
            spr->character=(uint8_t)character;
            machine_poke(lib->core, particle+PARTICLE_MEM_FRAME, (step_id+1)&0x1FFF);
          }
        }
    }
}

void prtclib_interrupt(struct Core *core,struct ParticlesLib *lib)
{
    for(int particle_id=0; particle_id<lib->pool_count; ++particle_id)
    {
        int particle = lib->particles_data_addr + particle_id*PARTICLE_MEM_SIZE; // 6 bytes

        // apperance is also used to disable the particle
        int apperance_id=machine_peek(lib->core, particle+PARTICLE_MEM_APPEARANCE);
        if(apperance_id>APPEARANCE_MAX) continue;

        lib->interrupt_sprite_id = lib->first_sprite_id+particle_id;
        lib->interrupt_particle_addr = lib->particles_data_addr + particle_id*PARTICLE_MEM_SIZE;

        itp_runInterrupt(core, InterruptTypeParticle);
    }

		for(int emitter_id=0; emitter_id<lib->emitters_count; ++emitter_id)
    {
				int emitter = lib->emitters_data_addr + emitter_id*EMITTER_MEM_SIZE; // 6 bytes

				int repeat=machine_peek(lib->core, emitter+EMITTER_MEM_REPEAT);
				if(repeat>0)
				{
						uint8_t count = dat_readU8(lib->emitters_label[emitter_id],EMITTER_DATA_COUNT,0);
						if(count>0)
						{
							lib->interrupt_emitter_id = emitter_id;
							lib->interrupt_emitter_addr = emitter;
							itp_runInterrupt(core, InterruptTypeEmitter);
						}
				}
		}
}


void prtclib_clear(struct Core *core,struct ParticlesLib *lib)
{
    for(int particle_id=0; particle_id<lib->pool_count; ++particle_id)
    {
        int particle = lib->particles_data_addr + particle_id*PARTICLE_MEM_SIZE; // 6 bytes
        machine_poke(lib->core, particle+PARTICLE_MEM_APPEARANCE, 255);

        int sprite_id=lib->first_sprite_id+particle_id;
        struct Sprite *spr=&lib->core->machine->spriteRegisters.sprites[sprite_id];

        spr->x = 0;
        spr->y = 0;
    }
}
