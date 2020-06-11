ifneq (,$(findstring clean, $(MAKECMDGOALS))) # Prevent searching for compilers if the make target clean
else
CHECK := $(shell gcc --version >/dev/null 2>&1 || (echo "Failed to search for gcc with error: $$"))
ifeq (,${CHECK})
    COMPILER := gcc

else # Check for clang if gcc is unavailable
    CHECK2 := $(shell clang --version >/dev/null 2>&1 || (echo "Failed to search for clang with error: $$?"))
	ifeq (,${CHECK2})
		COMPILER := clang
		LIBS := -lstdc
	else
$(error No C compilers found.)
	endif
endif

$(info C compiler found: ${COMPILER})
$(info )
endif

FLAGS := -O3 -MMD
LIBS  := -lm
BUILD := build
SRC := src
OBJ := ${BUILD}/obj
BIN := ${BUILD}/bin

EXECUTABLE := ${BIN}/CMP

SRCS := $(wildcard $(SRC)/*.c) $(wildcard $(SRC)/*/*.c)
OBJS := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))

DEPS := $(OBJS:.o=.d)
-include $(DEPS)

$(shell mkdir -p build build/bin build/obj)

# .PHONY means that these rules get executed even if files of those names exist.
.PHONY: clean

CMP : ${OBJS} 
	${COMPILER} ${FLAGS} -o ${EXECUTABLE} $^ ${LIBS}
	@echo
	@echo "Compiled Successfully!! Run the program using ./${EXECUTABLE}"

${OBJ}/%.o: ${SRC}/%.c
	${COMPILER} ${FLAGS} -c $< -o $@

clean:
	rm -rf ${BUILD}