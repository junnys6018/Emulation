# SET THIS TO THE LOCATION OF FAMITRACKER
FAMITRACKER = /d/dev/NES-Emulator/Resources/FamiTracker-v0.4.6/FamiTracker.exe

# SET THIS TO THE LOCATION OF ld65 AND ca65
ld65 = /d/dev/NES-Emulator/Resources/cc65-snapshot-win32/bin/ld65.exe
ca65 = /d/dev/NES-Emulator/Resources/cc65-snapshot-win32/bin/ca65.exe

FAMITONE_DIR = vendor/famitone2/

SRC_DIR = src/
INT_DIR = int/
RES_DIR = res/

INCLUDES  = $(SRC_DIR)nes.inc
INCLUDES += $(SRC_DIR)global.inc
INCLUDES += $(SRC_DIR)famitone2.inc

BINARIES  = $(RES_DIR)background.chr
BINARIES += $(RES_DIR)sprites.chr
BINARIES += $(RES_DIR)opening_screen.nam
BINARIES += $(RES_DIR)end_screen.nam
BINARIES += $(RES_DIR)level_select.nam
BINARIES += $(RES_DIR)palette.pal
BINARIES += $(INT_DIR)levels.bin

OBJ_FILES  = $(INT_DIR)blockoban.o
OBJ_FILES += $(INT_DIR)nmi.o
OBJ_FILES += $(INT_DIR)irq.o
OBJ_FILES += $(INT_DIR)gamepad.o
OBJ_FILES += $(INT_DIR)reset.o
OBJ_FILES += $(INT_DIR)util.o
OBJ_FILES += $(INT_DIR)drawing.o 
OBJ_FILES += $(INT_DIR)vars.o
OBJ_FILES += $(INT_DIR)level.o
OBJ_FILES += $(INT_DIR)end_screen.o
OBJ_FILES += $(INT_DIR)sound_data.o
OBJ_FILES += $(INT_DIR)famitone2.o

SOUND_FILES = $(INT_DIR)effects.s $(INT_DIR)song.s


all: blockoban.nes

blockoban.nes: $(OBJ_FILES) $(SRC_DIR)blockoban.cfg | $(INT_DIR)
	$(ld65) -C $(SRC_DIR)blockoban.cfg -o blockoban.nes $(OBJ_FILES)
	
$(INT_DIR)%.o: $(SRC_DIR)%.asm $(INCLUDES) | $(INT_DIR)
	$(ca65) $< -o $@
	
$(INT_DIR)levels.bin: levels.txt compile_levels.py | $(INT_DIR)
	python compile_levels.py
	
$(INT_DIR)blockoban.o: $(SRC_DIR)blockoban.asm $(BINARIES) $(INCLUDES) | $(INT_DIR)
	$(ca65) $< -o $@
	
$(INT_DIR)famitone2.o: $(FAMITONE_DIR)famitone2.s $(SRC_DIR)famitone2.inc | $(INT_DIR)
	$(ca65) $< -o $@
	
$(INT_DIR)sound_data.o: $(SRC_DIR)sound_data.asm $(INCLUDES) $(SOUND_FILES) | $(INT_DIR)
	$(ca65) $< -o $@
	
$(INT_DIR)effects.s: $(INT_DIR)effects.nsf | $(INT_DIR)
	$(FAMITONE_DIR)tools/nsf2data.exe $< -ca65 -ntsc
	
$(INT_DIR)effects.nsf: $(RES_DIR)effects.ftm | $(INT_DIR)
	$(FAMITRACKER) $< -export $@
	
$(INT_DIR)song.s: $(INT_DIR)song.txt | $(INT_DIR)
	$(FAMITONE_DIR)tools/text2data.exe $< -ca65 -ntsc
	
$(INT_DIR)song.txt: $(RES_DIR)song.ftm | $(INT_DIR)
	$(FAMITRACKER) $< -export $@
	
$(INT_DIR):
	mkdir -p $(INT_DIR)
	
clean:
	rm -rf $(INT_DIR) blockoban.nes