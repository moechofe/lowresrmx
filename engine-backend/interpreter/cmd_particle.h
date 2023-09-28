#ifndef cmd_particle_h
#define cmd_particle_h

#include "error.h"

struct Core;

enum ErrorCode cmd_PARTICLE(struct Core *core);
enum ErrorCode cmd_EMITTER(struct Core *core);

#endif