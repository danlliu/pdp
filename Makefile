
ASSEMBLER_DIR = assembler_src
ASSEMBLER_OBJ = AssemblerCommon.o LineParser.o LabelResolver.o InstructionAssembler.o DirectiveResolver.o DirectiveAssembler.o TapeWriter.o

SIMULATOR_DIR = simulator_src
SIMULATOR_OBJ = PDPSettings.o PDPState.o TapeReader.o

CLANG = g++ -std=c++17 -O3 -Wall -Werror
CLANG_OBJ = $(CLANG) -c
DEBUG_FLAGS = -DDEBUG -g3

.SUFFIXES:

.PHONY: all
all: assembler simulator

.PHONY: debug
debug: assembler_debug simulator_debug

%.dbg.o: %.cpp %.hpp
	$(CLANG_OBJ) $(DEBUG_FLAGS) $< -o $@

%.o: %.cpp %.hpp
	$(CLANG_OBJ) $< -o $@

assembler: $(ASSEMBLER_OBJ:%.o=$(ASSEMBLER_DIR)/%.o) $(ASSEMBLER_DIR)/assembler.cpp
	$(CLANG) $^ -o $@

assembler_debug: $(ASSEMBLER_OBJ:%.o=$(ASSEMBLER_DIR)/%.dbg.o) $(ASSEMBLER_DIR)/assembler.cpp
	$(CLANG) $(DEBUG_FLAGS) $^ -o $@

simulator: $(SIMULATOR_OBJ:%.o=$(SIMULATOR_DIR)/%.o) $(SIMULATOR_DIR)/simulator.cpp
	$(CLANG) $^ -o $@

simulator_debug: $(SIMULATOR_OBJ:%.o=$(SIMULATOR_DIR)/%.dbg.o) $(SIMULATOR_DIR)/simulator.cpp
	$(CLANG) $(DEBUG_FLAGS) $^ -o $@

.PHONY: clean
clean:
	for f in $(ASSEMBLER_OBJ:%.o=$(ASSEMBLER_DIR)/%.o); do \
		rm -f $$f; \
	done
	for f in $(ASSEMBLER_OBJ:%.o=$(ASSEMBLER_DIR)/%.dbg.o); do \
		rm -f $$f; \
	done
	for f in $(SIMULATOR_OBJ:%.o=$(SIMULATOR_DIR)/%.o); do \
		rm -f $$f; \
	done
	for f in $(SIMULATOR_OBJ:%.o=$(SIMULATOR_DIR)/%.dbg.o); do \
		rm -f $$f; \
	done
	rm -rf *.dSYM
	rm -f assembler assembler_debug
	rm -f simulator simulator_debug

