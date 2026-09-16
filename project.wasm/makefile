# Declaration of variables
CC = emcc
CC_FLAGS = -w -s USE_SDL=3 -I ../backend.core -I ../backend.core/machine -I ../backend.core/accessories -I ../backend.core/datamanager -I ../backend.core/interpreter -I ../backend.core/libraries -I ../backend.core/overlay -I ../frontend.sdl
CC_FLAGS_MAIN = -s USE_SDL=3

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

# Headless harness runner
SOURCES_TEST = $(wildcard ../backend.core/*.c) $(wildcard ../backend.core/*/*.c) $(wildcard ../test.harness/*.c)
CC_FLAGS_TEST = -w -I ../backend.core -I ../backend.core/accessories -I ../backend.core/datamanager \
                -I ../backend.core/interpreter -I ../backend.core/libraries -I ../backend.core/machine \
                -I ../backend.core/overlay -I ../frontend.sdl -I ../test.harness \
                -sNODERAWFS=1 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1

build/test_player.js: $(SOURCES_TEST)
	@mkdir -p build
	$(CC) $(SOURCES_TEST) $(CC_FLAGS_TEST) -o $@

# To remove generated files
clean:
	rm -f $(OBJECTS)
