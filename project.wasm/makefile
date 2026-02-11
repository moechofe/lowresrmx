# Declaration of variables
CC = emcc
CC_FLAGS = -w -s USE_SDL=2 -I ../backend.core -I ../backend.core/machine -I ../backend.core/accessories -I ../backend.core/datamanager -I ../backend.core/interpreter -I ../backend.core/libraries -I ../backend.core/overlay
CC_FLAGS_MAIN = -s USE_SDL=2

# File names
# EXEC = ../web.community/sources/player.js
EXEC = ./player.js
SOURCES = $(wildcard ../backend.core/*.c) $(wildcard ../backend.core/*/*.c) $(wildcard ../frontend.sdl/*.c)
OBJECTS = $(SOURCES:.c=.o)

# Main target
$(EXEC): $(OBJECTS)
	@mkdir -p $(@D)
	$(CC) $(CC_FLAGS_MAIN) $(OBJECTS) -o $(EXEC)

# To obtain object files
%.o: %.c
	$(CC) -c $(CC_FLAGS) $< -o $@

# To remove generated files
clean:
	rm -f $(OBJECTS)
