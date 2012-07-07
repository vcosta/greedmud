!IF "$(CFG)" == ""
CFG=debug
!MESSAGE No configuration specified. Defaulting to 'debug'.
!ENDIF 

!IF "$(CFG)" != "release" && "$(CFG)" != "debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "greed.mak" CFG="debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "release" (based on "Win32 (x86) Console Application")
!MESSAGE "debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

CPP=cl.exe
RSC=rc.exe

OBJS=\
	"act_clan.obj"\
	"act_comm.obj"\
	"act_game.obj"\
	"act_info.obj"\
	"act_lang.obj"\
	"act_move.obj"\
	"act_obj.obj"\
	"act_olc.obj"\
	"act_wiz.obj"\
	"bit.obj"\
	"board.obj"\
	"comm.obj"\
	"const.obj"\
	"db.obj"\
	"db2.obj"\
	"fight.obj"\
	"handler.obj"\
	"hunt.obj"\
	"interp.obj"\
	"magic.obj"\
	"mem.obj"\
	"mob_comm.obj"\
	"mob_prog.obj"\
	"save.obj"\
	"spec_mob.obj"\
	"spec_obj.obj"\
	"spec_rom.obj"\
	"ssm.obj"\
	"string.obj"\
	"tables.obj"\
	"update.obj"\
	"wiznet.obj"

!IF  "$(CFG)" == "release"

# Begin Custom Macros
# End Custom Macros

ALL : "envy.exe"


CLEAN :
	-@erase $(OBJS)
	-@erase "vc60.idb"
	-@erase "envy.exe"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"envy.pch" /YX /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"envy.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console /incremental:no /pdb:"envy.pdb" /machine:I386 /out:"envy.exe" 

"envy.exe" : $(DEF_FILE) $(OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(OBJS)
<<

!ELSEIF  "$(CFG)" == "debug"

# Begin Custom Macros
# End Custom Macros

ALL : "envy.exe"


CLEAN :
	-@erase $(OBJS)
	-@erase "vc60.idb"
	-@erase "vc60.pdb"
	-@erase "envy.exe"
	-@erase "envy.ilk"
	-@erase "envy.pdb"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"envy.pch" /YX /FD /GZ  /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"envy.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  wsock32.lib /nologo /subsystem:console /incremental:yes /pdb:"envy.pdb" /debug /machine:I386 /out:"envy.exe" /pdbtype:sept 

"envy.exe" : $(DEF_FILE) $(OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(OBJS)
<<

!ENDIF 

.c.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(CFG)" == "release" || "$(CFG)" == "debug"

SOURCE=.\act_clan.c

"act_clan.obj" : $(SOURCE)


SOURCE=.\act_comm.c

"act_comm.obj" : $(SOURCE)


SOURCE=.\act_game.c

"act_game.obj" : $(SOURCE)


SOURCE=.\act_info.c

"act_info.obj" : $(SOURCE)


SOURCE=.\act_lang.c

"act_lang.obj" : $(SOURCE)


SOURCE=.\act_move.c

"act_move.obj" : $(SOURCE)


SOURCE=.\act_obj.c

"act_obj.obj" : $(SOURCE)


SOURCE=.\act_olc.c

"act_olc.obj" : $(SOURCE)


SOURCE=.\act_wiz.c

"act_wiz.obj" : $(SOURCE)


SOURCE=.\bit.c

"bit.obj" : $(SOURCE)


SOURCE=.\board.c

"board.obj" : $(SOURCE)


SOURCE=.\comm.c

"comm.obj" : $(SOURCE)


SOURCE=.\const.c

"const.obj" : $(SOURCE)


SOURCE=.\db.c

"db.obj" : $(SOURCE)


SOURCE=.\db2.c

"db2.obj" : $(SOURCE)


SOURCE=.\fight.c

"fight.obj" : $(SOURCE)


SOURCE=.\handler.c

"handler.obj" : $(SOURCE)


SOURCE=.\hunt.c

"hunt.obj" : $(SOURCE)


SOURCE=.\interp.c

"interp.obj" : $(SOURCE)


SOURCE=.\magic.c

"magic.obj" : $(SOURCE)


SOURCE=.\mem.c

"mem.obj" : $(SOURCE)


SOURCE=.\mob_comm.c

"mob_comm.obj" : $(SOURCE)


SOURCE=.\mob_prog.c

"mob_prog.obj" : $(SOURCE)


SOURCE=.\save.c

"save.obj" : $(SOURCE)


SOURCE=.\spec_mob.c

"spec_mob.obj" : $(SOURCE)


SOURCE=.\spec_obj.c

"spec_obj.obj" : $(SOURCE)


SOURCE=.\spec_rom.c

"spec_rom.obj" : $(SOURCE)


SOURCE=.\ssm.c

"ssm.obj" : $(SOURCE)


SOURCE=.\string.c

"string.obj" : $(SOURCE)


SOURCE=.\tables.c

"tables.obj" : $(SOURCE)


SOURCE=.\update.c

"update.obj" : $(SOURCE)


SOURCE=.\wiznet.c

"wiznet.obj" : $(SOURCE)

!ENDIF 
