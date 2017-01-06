CFILES:=$(wildcard src/source/*.c)
OBJ_FILES:=$(addprefix obj/,$(notdir $(CFILES:.c=.o)))
INCLUDES:=-I src/header

.PHONY: clean
CXX_FLAGS=-lm -lpthread

helo_offline: obj/ $(OBJ_FILES)
	gcc $(OBJ_FILES) -o helo_offline $(CXX_FLAGS)

obj/%.o: src/source/%.c
	gcc $(INCLUDES) -std=c99 -c $< -o $@

obj/:
	mkdir -p obj

clean:
	rm -f helo_offline
	rm -Rf obj
