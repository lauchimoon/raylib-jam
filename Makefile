SRC = $(wildcard *.c screens/*.c)
PLATFORM = DESKTOP
NAME = fistbumpd

ifeq ($(PLATFORM), WEB)
	CC = emcc
	LDLIBS = -L lib/web/ -lraylib
	OUT = build/web/index.html
	CFLAGS = -Wall -D_DEFAULT_SOURCE -Os -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864 -s FORCE_FILESYSTEM=1 -s DYNAMIC_EXECUTION=1 --preload-file assets --shell-file lib/minshell.html
	DEFINE = -DWEB
else
	CC = gcc
	LDLIBS = -L lib/desktop/ -lraylib -lGL -lm -lpthread -ldl
	OUT = build/$(NAME)
	CFLAGS = -Wall
	DEFINE = -DDESKTOP
endif

INCLUDE = -I ../raylib/src/ -I .
INVOKE = $(CC) $(SRC) $(LDLIBS) $(INCLUDE) $(DEFINE) $(CFLAGS) -o $(OUT)

default:
	$(INVOKE)

run:
ifeq ($(PLATFORM), WEB)
	$(INVOKE)
	emrun $(OUT)
else
	$(INVOKE)
	./$(OUT)
endif
	rm $(OUT)
