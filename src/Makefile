# Generic Makefile.  Read 'port.txt' in the documentation
# directory for the Makefile that is right for you  - Kahn

CC      = gcc
PROF    =
DEBUG   = -g
DEFINES = #-Wall -Wextra -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wmissing-declarations -Wdeclaration-after-statement #-DUSE_PTHREADS
O_FLAGS = #-O2
C_FLAGS = $(O_FLAGS) -Wall	$(DEBUG) $(PROF) $(DEFINES)
L_FLAGS = $(O_FLAGS)		$(DEBUG) $(PROF) -lcrypt# -lpthread


O_FILES = act_clan.o act_comm.o act_game.o act_info.o act_lang.o act_move.o   \
          act_obj.o act_wiz.o bit.o board.o comm.o const.o db.o     \
          db2.o fight.o handler.o hunt.o interp.o magic.o mem.o mob_comm.o    \
          mob_prog.o save.o spec_mob.o spec_obj.o spec_rom.o ssm.o   \
          tables.o update.o wiznet.o

envy: $(O_FILES)
	rm -f envy
	$(CC) -o envy $(O_FILES) $(L_FLAGS) 

.c.o: merc.h
	$(CC) -c $(C_FLAGS) $<

clean:
	rm -f *.o envy envy2
