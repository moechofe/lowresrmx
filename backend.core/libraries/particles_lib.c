#include "particles_lib.h"

#include "core.h"
#include "machine.h"
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

    for(int i=0;i<poolCount;++i)
    {
      machine_poke(lib->core, particleAddr+(i*6)+4, -1);
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

    int emitter = lib->emitters_data_addr + emitterId*6; // 6 bytes

    machine_poke_short(lib->core, emitter, ((int16_t)posX*16)&0x1FFF);
    machine_poke_short(lib->core, emitter+2, ((int16_t)posY*16)&0x1FFF);

    machine_poke(lib->core, emitter+4, 0); // start with no delay

    uint8_t repeat = 1 + dat_readU8(lib->emitters_label[emitterId],9,0);
    machine_poke(lib->core, emitter+5, repeat);
}

void prtclib_stop(struct ParticlesLib *lib,int emitterId)
{
    if(emitterId<0 && emitterId>=lib->emitters_count) return;

    int emitter = lib->emitters_data_addr + emitterId*6; // 6 bytes

    machine_poke(lib->core, emitter+5, 0);
}

void prtclib_update(struct Core *core, struct ParticlesLib *lib)
{
    struct Interpreter *interpreter = core->interpreter;

    struct Token *dataToken;

    // update emitters
    for(int emitter_id=0; emitter_id<lib->emitters_count; ++emitter_id)
    {
        int emitter = lib->emitters_data_addr + emitter_id*6; // 6 bytes

        float pos_x=(float)machine_peek_short(lib->core, emitter)/16;
        float pos_y=(float)machine_peek_short(lib->core, emitter+2)/16;

        // wait for delay to end
        int delay=machine_peek(lib->core, emitter+4);
        if(delay>0)
        {
          machine_poke(lib->core, emitter+4, delay-1);
          continue;
        }

        // is there more to repeat?
        int repeat=machine_peek(lib->core, emitter+5);
        if(repeat>0)
        {
          // emitter data
          uint8_t apperance_id = dat_readU8(lib->emitters_label[emitter_id],0,255);
          if (apperance_id>APPEARANCE_MAX) continue;

					float shape = dat_readFloat(lib->emitters_label[emitter_id],1,1);
          float outer = dat_readFloat(lib->emitters_label[emitter_id],2,0);
          float inner = dat_readFloat(lib->emitters_label[emitter_id],3,0);
					float rotation;
					float arc = modff(dat_readFloat(lib->emitters_label[emitter_id],4,0), &rotation);
          float speed_x = dat_readFloat(lib->emitters_label[emitter_id],5,0);
          float speed_y = dat_readFloat(lib->emitters_label[emitter_id],6,0);
          uint8_t count = dat_readU8(lib->emitters_label[emitter_id],7,0);
          uint8_t delay = dat_readU8(lib->emitters_label[emitter_id],8,0);

          // reduce repeat
          machine_poke(lib->core, emitter+5, repeat-1);

          // reset delay
          machine_poke(lib->core, emitter+4, delay);

          for(int i=0; i<count; ++i)
          {
            // spawn a particle
            int particle_id = lib->pool_next_id;
            lib->pool_next_id = (lib->pool_next_id+1) % lib->pool_count;

            int particle = lib->particles_data_addr + particle_id*6; // 6 bytes

            int sprite_id=lib->first_sprite_id+particle_id;
            struct Sprite *spr=&lib->core->machine->spriteRegisters.sprites[sprite_id];

            // inner, outer ring
            if(outer>0 && outer>=inner && outer-inner>0 && shape!=0)
            {
							if(shape>0)
							{
									// TODO: https://stackoverflow.com/questions/5837572/generate-a-random-point-within-a-circle-uniformly

									// try sqrt(r)

									double angle=ldexp(pcg32_random_r(&pcg),-32);
									float width=(float)pcg32_boundedrand_r(&pcg,outer-inner)+inner;
									float height=width*shape;
									spr->x=(int)((pos_x+SPRITE_OFFSET_X+cosf(angle*M_PI*2)*width)*16) & 0x1FFF;
									spr->y=(int)((pos_y+SPRITE_OFFSET_Y+sinf(angle*M_PI*2)*height)*16) & 0x1FFF;
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
										x=(float)pcg32_boundedrand_r(&pcg,outer*2)-outer;
										y=((float)pcg32_boundedrand_r(&pcg,outer-inner)+inner)*-shape;
										y*=sign;
									}
									else
									{
										// vertical
										x=(float)pcg32_boundedrand_r(&pcg,outer-inner)+inner;
										x*=sign;
										y=((float)pcg32_boundedrand_r(&pcg,outer*2)-outer)*-shape;
									}
									spr->x=(int)((pos_x+SPRITE_OFFSET_X+x)*16) & 0x1FFF;
									spr->y=(int)((pos_y+SPRITE_OFFSET_Y+y)*16) & 0x1FFF;
							}
            }
						else
						{
							spr->x=(int)((pos_x + SPRITE_OFFSET_X)*16) & 0x1FFF;
							spr->y=(int)((pos_y + SPRITE_OFFSET_Y)*16) & 0x1FFF;
						}

            // speed x,y
            machine_poke_short(lib->core, particle, speed_x);
            machine_poke_short(lib->core, particle+2, speed_y);

            // apperance
            machine_poke(lib->core, particle+4, apperance_id);
            machine_poke(lib->core, particle+5, 0);
          }
        }
    }

    // update particles
    for(int particle_id=0; particle_id<lib->pool_count; ++particle_id)
    {
        int particle = lib->particles_data_addr + particle_id*6; // 6 bytes

        // apperance is also used to disable the particle
        int apperance_id=machine_peek(lib->core, particle+4);
        if(apperance_id>APPEARANCE_MAX) continue;

        // sprite
        int sprite_id=lib->first_sprite_id+particle_id;
        struct Sprite *spr=&lib->core->machine->spriteRegisters.sprites[sprite_id];

        // position x
        float speed_x=(float)machine_peek_short(lib->core, particle);
        spr->x=(int)(spr->x+speed_x)&0x1FFF;

        // position y
        float speed_y=(float)machine_peek_short(lib->core, particle+2);
        spr->y=(int)(spr->y+speed_y)&0x1FFF;

        // character

        int step_id=machine_peek(lib->core, particle+5);

        float character=dat_readFloat(lib->apperances_label[apperance_id],step_id,0);
        if(character>=0)
        {
          spr->character=(uint8_t)character;
          machine_poke(lib->core, particle+5, (step_id+1)&0x1FFF);
        }
        else
        {
          step_id=step_id+(int)character;
          character=dat_readFloat(lib->apperances_label[apperance_id],step_id,0);
          if(character>=0)
          {
            spr->character=(uint8_t)character;
            machine_poke(lib->core, particle+5, (step_id+1)&0x1FFF);
          }
        }
    }
}

void prtclib_interrupt(struct Core *core,struct ParticlesLib *lib)
{
    for(int particle_id=0; particle_id<lib->pool_count; ++particle_id)
    {
        int particle = lib->particles_data_addr + particle_id*6; // 6 bytes

        // apperance is also used to disable the particle
        int apperance_id=machine_peek(lib->core, particle+4);
        if(apperance_id>APPEARANCE_MAX) continue;

        lib->interrupt_sprite_id = lib->first_sprite_id+particle_id;
        lib->interrupt_particle_addr = lib->particles_data_addr + particle_id*6;

        itp_runInterrupt(core, InterruptTypeParticle);
    }

		for(int emitter_id=0; emitter_id<lib->emitters_count; ++emitter_id)
    {
				int emitter = lib->emitters_data_addr + emitter_id*6; // 6 bytes

				int repeat=machine_peek(lib->core, emitter+5);
				if(repeat>0)
				{
						uint8_t count = dat_readU8(lib->emitters_label[emitter_id],5,0);
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
        int particle = lib->particles_data_addr + particle_id*6; // 6 bytes
        machine_poke(lib->core, particle+4, 255);

        int sprite_id=lib->first_sprite_id+particle_id;
        struct Sprite *spr=&lib->core->machine->spriteRegisters.sprites[sprite_id];

        spr->x = 0;
        spr->y = 0;
    }
}
