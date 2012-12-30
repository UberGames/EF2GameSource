#
# Quake3 Unix Makefile
#
# Currently build for the following:
#	Linux i386 (full client)
#	Linux Alpha (dedicated server only)
#	SGI IRIX (full client)
#
# Nov '98 by Zoid <zoid@idsoftware.com>
#
# GNU Make required
#

PLATFORM=$(shell uname|tr A-Z a-z)
PLATFORM_RELEASE=$(shell uname -r)

###
### These paths are where you probably want to change things
###

# Where we are building to, libMesaVoodooGL.so.3.3 should be here, etc.
# the demo pk3 file should be here in demoq3/pak0.pk3 or baseq3/pak0.pk3
#BDIR=/home/zoid/Quake3
BDIR=./

# Where we are building from (where the source code should be!)
#MOUNT_DIR=/projects/Quake3/q3code
MOUNT_DIR=..

# Custom lcc location, used in vm builds
LCC=/home/zoid/Quake3/lcc

# Custom q3asm location, used in vm builds
Q3ASM=/home/zoid/Quake3/q3asm

# Build name

#BUILD_NAME=linuxquake3
BUILD_NAME=linuxef2ded

#############################################################################
##
## You shouldn't have to touch anything below here
##
#############################################################################

BASEQ3_DIR=$(BDIR)/baseq3

BD=debug$(ARCH)$(GLIBC)
BR=release$(ARCH)$(GLIBC)
CDIR=$(MOUNT_DIR)/Executable/client
SDIR=$(MOUNT_DIR)/Executable/server
RDIR=$(MOUNT_DIR)/Libs/renderer
CMDIR=$(MOUNT_DIR)/Shared/qcommon
UDIR=$(MOUNT_DIR)/Executable/unix
GDIR=$(MOUNT_DIR)/dlls/game
CGDIR=$(MOUNT_DIR)/DLLs/cgame
BAIDIR=$(MOUNT_DIR)/botai
BLIBDIR=$(MOUNT_DIR)/Executable/botlib
NDIR=$(MOUNT_DIR)/Executable/null
UIDIR=$(MOUNT_DIR)/ui
JPDIR=$(MOUNT_DIR)/jpeg-6
DLLINC = $(MOUNT_DIR)/DLLs
SHAREINC = $(MOUNT_DIR)/Shared
EXECINC  = $(MOUNT_DIR)/Executable
LIBINC   = $(MOUNT_DIR)/Libs


#used for linux i386 builds
MESADIR=/usr/local/src/Mesa

VERSION=1.16j
RPM_RELEASE=1

#############################################################################
# SETUP AND BUILD -- LINUX
#############################################################################

ifeq ($(PLATFORM),linux)

ifneq (,$(findstring libc6,$(shell if [ -e /lib/libc.so.6* ];then echo libc6;fi)))
GLIBC=-glibc
else
GLIBC=
endif #libc6 test


ifneq (,$(findstring alpha,$(shell uname -m)))
ARCH=axp
RPMARCH=alpha
VENDOR=dec
else #default to i386
ARCH=i386
RPMARCH=i386
VENDOR=unknown
endif #alpha test

BASE_CFLAGS=-Dstricmp=strcasecmp -Dstrcmpi=strcasecmp -Dstrnicmp=strncasecmp -pipe -m32 -I../dlls
#BASE_CFLAGS=-Dstricmp=strcasecmp -I$(MESADIR)/include -I/usr/X11R6/include -I/usr/include/glide -pipe

DEBUG_CFLAGS=$(BASE_CFLAGS) -g -Wno-deprecated
ifeq ($(ARCH),axp)
#CC=gcc
CC=g++
GC=gcc
RELEASE_CFLAGS=$(BASE_CFLAGS) -DNDEBUG -O6 -ffast-math -funroll-loops -fomit-frame-pointer -fexpensive-optimizations
else
CC=g++
GC=gcc
#NEWGCC=/usr/local/gcc-2.95/bin/gcc
#CC=$(shell if [ -f $(NEWGCC) ]; then echo $(NEWGCC); else echo g++; fi )
RELEASE_CFLAGS=$(BASE_CFLAGS) -DNDEBUG -O6 -march=pentium -fomit-frame-pointer -pipe -ffast-math -falign-loops=2 -falign-jumps=2 -falign-functions=2 -fno-strict-aliasing -fstrength-reduce -Wno-deprecated

endif

LIBEXT=a

SHLIBEXT=so
SHLIBCFLAGS= -fPIC
SHLIBLDFLAGS=-shared

ARFLAGS=ar rv
RANLIB=ranlib

THREAD_LDFLAGS=-lpthread
LDFLAGS=-ldl -lm
GLLDFLAGS=-L/usr/X11R6/lib -L$(MESADIR)/lib -lX11 -lXext -lXxf86dga -lXxf86vm

#ifeq ($(ARCH),axp)
TARGETS=\
	$(B)/ef2game$(ARCH).$(SHLIBEXT) \
	$(B)/$(PLATFORM)ef2ded 

#else
#TARGETS=\
#	$(B)/$(PLATFORM)quake3 \
#	$(B)/$(PLATFORM)quake3-smp \
#	$(B)/$(PLATFORM)q3ded \
#	$(B)/vm/qagame.qvm \
#	$(B)/vm/cgame.qvm \
#	$(B)/vm/ui.qvm

#	$(B)/cgame$(ARCH).$(SHLIBEXT) \
#	$(B)/qagame$(ARCH).$(SHLIBEXT) \
#	$(B)/ui$(ARCH).$(SHLIBEXT) \
#
#endif

else # ifeq Linux

#############################################################################
# SETUP AND BUILD -- IRIX
#############################################################################

ifeq ($(PLATFORM),irix)

ARCH=mips  #default to MIPS
VENDOR=sgi
GLIBC=  #libc is irrelevant

CC=cc
BASE_CFLAGS=-Dstricmp=strcasecmp -Dstrnicmp=strncasecmp -Xcpluscomm -woff 1185 -mips3 \
	-nostdinc -I. -I$(ROOT)/usr/include
RELEASE_CFLAGS=$(BASE_CFLAGS) -O3
DEBUG_CFLAGS=$(BASE_CFLAGS) -g

LIBEXT=a

SHLIBEXT=so
SHLIBCFLAGS=
SHLIBLDFLAGS=-shared

ARFLAGS=ar rv
RANLIB=ranlib

LDFLAGS=-ldl -lm
GLLDFLAGS=-L/usr/X11/lib -lGL -lX11 -lXext -lm

TARGETS=$(B)/sgiquake3 \
	$(B)/q3ded

else # ifeq IRIX

#############################################################################
# SETUP AND BUILD -- GENERIC
#############################################################################

CC=cc
#CC=gcc
BASE_CFLAGS=-Dstricmp=strcasecmp -Dstrnicmp=strncasecmp
DEBUG_CFLAGS=$(BASE_CFLAGS) -g
RELEASE_CFLAGS=$(BASE_CFLAGS) -DNDEBUG -O

LIBEXT=a

SHLIBEXT=so
SHLIBCFLAGS=-fPIC
SHLIBLDFLAGS=-shared

ARFLAGS=ar rv
RANLIB=ranlib

LDFLAGS=-ldl -lm

TARGETS=\
	$(B)/$(PLATFORM)ef2ded

endif #Linux
endif #IRIX

DO_CC=$(CC) $(CFLAGS) -o $@ -c $<
DO_SMP_CC=$(CC) $(CFLAGS) -DSMP -o $@ -c $<
DO_BOT_CC=$(GC) $(CFLAGS) -DBOTLIB -DAASINTERN -DMISSIONPACK -DENABLE_ALTROUTING $(SHLIBCFLAGS) -I$(DLLINC) -I$(SHAREINC) -I$(EXECINC) -I$(CMDIR) -I$(LIBINC) -o $@ -c $<
DO_DEBUG_CC=$(CC) $(DEBUG_CFLAGS) -o $@ -c $<
DO_SHLIB_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) -I$(DLLINC) -I$(SHAREINC) -I$(EXECINC) -I$(CMDIR) -I$(LIBINC) -o $@ -c $<
DO_GAME_SHLIB_CC=$(CC) -DGAME_DLL -DDEDICATED -DMISSIONPACK -DLINUX $(CFLAGS) $(SHLIBCFLAGS) -I$(DLLINC) -I$(SHAREINC) -I$(EXECINC) -I$(CMDIR) -I$(LIBINC) -o $@ -c $<
DO_GAME_SHLIB_GC=$(GC) -DGAME_DLL -DDEDICATED -DMISSIONPACK -DLINUX $(CFLAGS) $(SHLIBCFLAGS) -I$(DLLINC) -I$(SHAREINC) -I$(EXECINC) -I$(CMDIR) -I$(LIBINC) -o $@ -c $<
DO_SHLIB_DEBUG_CC=$(CC) $(DEBUG_CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<

DO_AS=$(CC) $(CFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<

DO_DED_CC=$(CC) -DDEDICATED -DEF2 -DAASINTERN -DAAS_SAMPLE_DEBUG -DDEBUG_AI_MOVE -DDEBUG_AI_WEAP -DDEBUG_AI_GOAL -DALTROUTE_DEBUG -DROUTING_DEBUG -DREACH_DEBUG -DAAS_MOVE_DEBUG -DMISSIONPACK -DENABLE_ALTROUTING -DLINUX -DBOTLIB -DC_ONLY $(CFLAGS) -I$(DLLINC) -I$(SHAREINC) -I$(EXECINC) -I$(CMDIR) -I$(LIBINC) -c -o $@  $<
DO_DED_GC=$(GC) -DDEDICATED -DEF2 -DAASINTERN -DAAS_SAMPLE_DEBUG -DDEBUG_AI_MOVE -DDEBUG_AI_WEAP -DDEBUG_AI_GOAL -DALTROUTE_DEBUG -DROUTING_DEBUG -DREACH_DEBUG -DAAS_MOVE_DEBUG -DMISSIONPACK -DENABLE_ALTROUTING -DLINUX -DBOTLIB -DC_ONLY $(CFLAGS) -I$(DLLINC) -I$(SHAREINC) -I$(EXECINC) -I$(CMDIR) -I$(LIBINC) -c -o $@  $<
#DO_DED_CC=$(CC) -DDEDICATED -DC_ONLY $(CFLAGS) -o $@ -c $<

DO_LCC=$(LCC) -S -Wf-target=bytecode -Wf-g -I$(CGDIR) -I$(GDIR) -I$(BAIDIR) -I$(BLIBDIR) -I$(UIDIR) -o $@ -c $<

#### DEFAULT TARGET
# default:build_release

#build_debug:
#	$(MAKE) targets B=$(BD) CFLAGS="$(DEBUG_CFLAGS)"

build_release:
	$(MAKE) targets B=$(BR) CFLAGS="$(RELEASE_CFLAGS)"
build_debug:
	$(MAKE) targets B=$(BD) CFLAGS="$(DEBUG_CFLAGS)"

    
#Build both debug and release builds
#all:build_debug build_release
all:build_debug

targets:makedirs $(TARGETS)

makedirs: 
	@if [ ! -d $(B) ]; then mkdir $(B);fi; \
	if [ ! -d $(B)/ded ];then mkdir $(B)/ded;fi; \
	if [ ! -d $(B)/game ]; then mkdir $(B)/game;fi
	 


#############################################################################
# DEDICATED SERVER                        
#############################################################################

Q3DOBJ = \
    $(B)/ded/sv_bot.o \
    $(B)/ded/sv_client.o \
    $(B)/ded/sv_ccmds.o \
    $(B)/ded/sv_game.o \
    $(B)/ded/sv_init.o \
    $(B)/ded/sv_main.o \
    $(B)/ded/sv_netchan.o \
    $(B)/ded/sv_snapshot.o \
    $(B)/ded/sv_world.o \
    \
    $(B)/ded/alias.o \
    $(B)/ded/cm_load.o \
    $(B)/ded/cm_patch.o \
    $(B)/ded/cm_polylib.o \
    $(B)/ded/cm_terrain.o \
    $(B)/ded/cm_test.o \
    $(B)/ded/cm_trace.o \
    $(B)/ded/cmd.o \
    $(B)/ded/common.o \
    $(B)/ded/crc.o \
    $(B)/ded/cvar.o \
    $(B)/ded/files.o \
    $(B)/ded/gameplaydatabase.o \
    $(B)/ded/gameplayformulamanager.o \
    $(B)/ded/gameplaymanager.o \
    $(B)/ded/huffman.o \
    $(B)/ded/levelManager.o \
    $(B)/ded/lip.o \
    $(B)/ded/msg.o \
    $(B)/ded/net_chan.o \
    $(B)/ded/output.o \
    \
    $(B)/ded/q_math.o \
    $(B)/ded/q_mathsys.o \
    $(B)/ded/q_shared.o \
    $(B)/ded/memory.o \
    \
    $(B)/ded/str.o \
    $(B)/ded/stringresource.o \
    $(B)/ded/listener.o \
    $(B)/ded/sv_snd.o \
    $(B)/ded/viewmode.o \
    $(B)/ded/class.o \
    \
    $(B)/ded/tiki_main.o \
    $(B)/ded/tiki_script.o \
    $(B)/ded/unzip.o \
    \
    $(B)/ded/be_aas_bspq3.o \
    $(B)/ded/be_aas_cluster.o \
    $(B)/ded/be_aas_debug.o \
    $(B)/ded/be_aas_entity.o \
    $(B)/ded/be_aas_file.o \
    $(B)/ded/be_aas_main.o \
    $(B)/ded/be_aas_move.o \
    $(B)/ded/be_aas_optimize.o \
    $(B)/ded/be_aas_reach.o \
    $(B)/ded/be_aas_route.o \
    $(B)/ded/be_aas_routealt.o \
    $(B)/ded/be_aas_sample.o \
    $(B)/ded/be_ai_char.o \
    $(B)/ded/be_ai_chat.o \
    $(B)/ded/be_ai_gen.o \
    $(B)/ded/be_ai_goal.o \
    $(B)/ded/be_ai_move.o \
    $(B)/ded/be_ai_weap.o \
    $(B)/ded/be_ai_weight.o \
    $(B)/ded/be_ea.o \
    $(B)/ded/be_interface.o \
    $(B)/ded/l_crc.o \
    $(B)/ded/l_libvar.o \
    $(B)/ded/l_log.o \
    $(B)/ded/l_memory.o \
    $(B)/ded/l_precomp.o \
    $(B)/ded/l_script.o \
    $(B)/ded/l_struct.o \
    \
    $(B)/ded/unix_main.o \
    $(B)/ded/unix_net.o \
    $(B)/ded/unix_shared.o \
    \
    $(B)/ded/md4.o \
    $(B)/ded/null_client.o \
    $(B)/ded/null_cl.o \
    $(B)/ded/null_input.o \
    $(B)/ded/null_snddma.o

$(B)/$(PLATFORM)ef2ded : $(Q3DOBJ)
	$(CC) $(CFLAGS) -o $@ $(Q3DOBJ) $(LDFLAGS)

$(B)/ded/sv_bot.o : $(BLIBDIR)/sv_bot.c; $(DO_DED_GC) 
$(B)/ded/sv_client.o : $(SDIR)/sv_client.c; $(DO_DED_GC) 
$(B)/ded/sv_ccmds.o : $(SDIR)/sv_ccmds.c; $(DO_DED_GC) 
$(B)/ded/sv_game.o : $(SDIR)/sv_game.c; $(DO_DED_GC) 
$(B)/ded/sv_init.o : $(SDIR)/sv_init.c; $(DO_DED_GC) 
$(B)/ded/sv_main.o : $(SDIR)/sv_main.c; $(DO_DED_GC) 
$(B)/ded/sv_netchan.o : $(SDIR)/sv_net_chan.c; $(DO_DED_GC) 
$(B)/ded/sv_snapshot.o : $(SDIR)/sv_snapshot.c; $(DO_DED_GC) 
$(B)/ded/sv_snd.o : $(SDIR)/sv_snd.c; $(DO_DED_GC) 
$(B)/ded/sv_world.o : $(SDIR)/sv_world.c; $(DO_DED_GC) 
$(B)/ded/alias.o : $(CMDIR)/alias.c; $(DO_DED_GC) 
$(B)/ded/class.o : $(GDIR)/class.cpp; $(DO_DED_CC)
$(B)/ded/cm_load.o : $(CMDIR)/cm_load.c; $(DO_DED_GC) 
$(B)/ded/cm_polylib.o : $(CMDIR)/cm_polylib.c; $(DO_DED_GC) 
$(B)/ded/cm_terrain.o : $(CMDIR)/cm_terrain.c; $(DO_DED_GC)
$(B)/ded/cm_test.o : $(CMDIR)/cm_test.c; $(DO_DED_GC) 
$(B)/ded/cm_trace.o : $(CMDIR)/cm_trace.c; $(DO_DED_GC)
$(B)/ded/cm_patch.o : $(CMDIR)/cm_patch.c; $(DO_DED_GC) 
$(B)/ded/cmd.o : $(CMDIR)/cmd.c; $(DO_DED_GC) 
$(B)/ded/common.o : $(CMDIR)/common.c; $(DO_DED_GC) 
$(B)/ded/crc.o : $(CMDIR)/crc.c; $(DO_DED_GC) 
$(B)/ded/cvar.o : $(CMDIR)/cvar.c; $(DO_DED_GC) 
$(B)/ded/files.o : $(CMDIR)/files.c; $(DO_DED_GC) 
$(B)/ded/md4.o : $(CMDIR)/md4.c; $(DO_DED_GC) 
$(B)/ded/gameplaydatabase.o : $(CMDIR)/gameplaydatabase.cpp; $(DO_DED_CC)
$(B)/ded/gameplayformulamanager.o : $(CMDIR)/gameplayformulamanager.cpp; $(DO_DED_CC)
$(B)/ded/gameplaymanager.o : $(CMDIR)/gameplaymanager.cpp; $(DO_DED_CC)
$(B)/ded/huffman.o : $(CMDIR)/huffman.c; $(DO_DED_GC)
$(B)/ded/levelManager.o : $(CMDIR)/levelManager.cpp; $(DO_DED_CC)
$(B)/ded/listener.o : $(GDIR)/listener.cpp; $(DO_DED_CC)
$(B)/ded/lip.o : $(CMDIR)/lip.c; $(DO_DED_GC)

$(B)/ded/msg.o : $(CMDIR)/msg.c; $(DO_DED_GC) 
$(B)/ded/net_chan.o : $(CMDIR)/net_chan.c; $(DO_DED_GC) 
$(B)/ded/output.o : $(CMDIR)/output.cpp; $(DO_DED_CC) 
$(B)/ded/q_shared.o : $(GDIR)/q_shared.c; $(DO_DED_GC) 
$(B)/ded/memory.o : $(CMDIR)/memory.c; $(DO_DED_GC)
$(B)/ded/q_math.o : $(GDIR)/q_math.c; $(DO_DED_GC) 
$(B)/ded/q_mathsys.o : $(GDIR)/q_mathsys.c; $(DO_DED_GC)

$(B)/ded/str.o : $(GDIR)/str.cpp; $(DO_DED_CC)
$(B)/ded/stringresource.o : $(CMDIR)/stringresource.cpp; $(DO_DED_CC)

$(B)/ded/tiki_main.o : $(CMDIR)/tiki_main.cpp; $(DO_DED_CC)
$(B)/ded/tiki_script.o : $(CMDIR)/tiki_script.cpp; $(DO_DED_CC)
$(B)/ded/viewmode.o : $(CMDIR)/viewmode.cpp; $(DO_DED_CC)

$(B)/ded/be_aas_bspq3.o : $(BLIBDIR)/be_aas_bspq3.c; $(DO_BOT_CC) 
$(B)/ded/be_aas_cluster.o : $(BLIBDIR)/be_aas_cluster.c; $(DO_BOT_CC) 
$(B)/ded/be_aas_debug.o : $(BLIBDIR)/be_aas_debug.c; $(DO_BOT_CC) 
$(B)/ded/be_aas_entity.o : $(BLIBDIR)/be_aas_entity.c; $(DO_BOT_CC) 
$(B)/ded/be_aas_file.o : $(BLIBDIR)/be_aas_file.c; $(DO_BOT_CC) 
$(B)/ded/be_aas_main.o : $(BLIBDIR)/be_aas_main.c; $(DO_BOT_CC) 
$(B)/ded/be_aas_move.o : $(BLIBDIR)/be_aas_move.c; $(DO_BOT_CC) 
$(B)/ded/be_aas_optimize.o : $(BLIBDIR)/be_aas_optimize.c; $(DO_BOT_CC) 
$(B)/ded/be_aas_reach.o : $(BLIBDIR)/be_aas_reach.c; $(DO_BOT_CC) 
$(B)/ded/be_aas_route.o : $(BLIBDIR)/be_aas_route.c; $(DO_BOT_CC) 
$(B)/ded/be_aas_routealt.o : $(BLIBDIR)/be_aas_routealt.c; $(DO_BOT_CC) 
$(B)/ded/be_aas_sample.o : $(BLIBDIR)/be_aas_sample.c; $(DO_BOT_CC) 
$(B)/ded/be_ai_char.o : $(BLIBDIR)/be_ai_char.c; $(DO_BOT_CC) 
$(B)/ded/be_ai_chat.o : $(BLIBDIR)/be_ai_chat.c; $(DO_BOT_CC) 
$(B)/ded/be_ai_gen.o : $(BLIBDIR)/be_ai_gen.c; $(DO_BOT_CC) 
$(B)/ded/be_ai_goal.o : $(BLIBDIR)/be_ai_goal.c; $(DO_BOT_CC) 
$(B)/ded/be_ai_move.o : $(BLIBDIR)/be_ai_move.c; $(DO_BOT_CC) 
$(B)/ded/be_ai_weap.o : $(BLIBDIR)/be_ai_weap.c; $(DO_BOT_CC) 
$(B)/ded/be_ai_weight.o : $(BLIBDIR)/be_ai_weight.c; $(DO_BOT_CC) 
$(B)/ded/be_ea.o : $(BLIBDIR)/be_ea.c; $(DO_BOT_CC) 
$(B)/ded/be_interface.o : $(BLIBDIR)/be_interface.c; $(DO_BOT_CC) 
$(B)/ded/l_crc.o : $(BLIBDIR)/l_crc.c; $(DO_BOT_CC) 
$(B)/ded/l_libvar.o : $(BLIBDIR)/l_libvar.c; $(DO_BOT_CC) 
$(B)/ded/l_log.o : $(BLIBDIR)/l_log.c; $(DO_BOT_CC) 
$(B)/ded/l_memory.o : $(BLIBDIR)/l_memory.c; $(DO_BOT_CC) 
$(B)/ded/l_precomp.o : $(BLIBDIR)/l_precomp.c; $(DO_BOT_CC) 
$(B)/ded/l_script.o : $(BLIBDIR)/l_script.c; $(DO_BOT_CC) 
$(B)/ded/l_struct.o : $(BLIBDIR)/l_struct.c; $(DO_BOT_CC) 

$(B)/ded/unix_main.o : $(UDIR)/unix_main.c; $(DO_DED_GC) 
$(B)/ded/unix_net.o : $(UDIR)/unix_net.c; $(DO_DED_GC) 
$(B)/ded/unix_shared.o : $(UDIR)/unix_shared.c; $(DO_DED_GC) 
$(B)/ded/null_client.o : $(NDIR)/null_client.c; $(DO_DED_GC) 
$(B)/ded/null_cl.o : $(NDIR)/null_cl.cpp; $(DO_DED_CC) 
$(B)/ded/null_input.o : $(NDIR)/null_input.c; $(DO_DED_GC) 
$(B)/ded/null_snddma.o : $(NDIR)/null_snddma.c; $(DO_DED_GC) 
$(B)/ded/unzip.o : $(CMDIR)/unzip.c; $(DO_DED_GC) 
#$(B)/ded/vm.o : $(CMDIR)/vm.c; $(DO_DED_CC) 
#$(B)/ded/vm_interpreted.o : $(CMDIR)/vm_interpreted.c; $(DO_DED_CC) 
#$(B)/ded/vm_x86.o : $(CMDIR)/vm_x86.c $(DO_DED_CC) 

#############################################################################
# GAME
#############################################################################

GOBJ =\
    $(B)/game/changePosture.o \
    $(B)/game/closeInOnEnemy.o \
    $(B)/game/closeInOnEnemyWhileFiringWeapon.o \
    $(B)/game/closeInOnPlayer.o \
    $(B)/game/corridorCombatWithRangedWeapon.o \
    $(B)/game/coverCombatWithRangedWeapon.o \
    $(B)/game/doAttack.o \
    $(B)/game/generalCombatWithMeleeWeapon.o \
    $(B)/game/generalCombatWithRangedWeapon.o \
    $(B)/game/gotoCurrentHelperNode.o \
    $(B)/game/gotoHelperNode.o \
    $(B)/game/gotoHelperNodeEX.o \
    $(B)/game/gotoHelperNodeNearestEnemy.o \
    $(B)/game/healGroupMember.o \
    $(B)/game/holdPosition.o \
    $(B)/game/MoveRandomDirection.o \
    $(B)/game/patrol.o \
    $(B)/game/PlayAnim.o \
    $(B)/game/rangedCombatWithWeapon.o \
    $(B)/game/rotateToEntity.o \
    $(B)/game/selectBestWeapon.o \
    $(B)/game/snipeEnemy.o \
    $(B)/game/stationaryFireCombat.o \
    $(B)/game/stationaryFireCombatEX.o \
    $(B)/game/suppressionFireCombat.o \
    $(B)/game/talk.o \
    $(B)/game/teleportToEntity.o \
    $(B)/game/teleportToPosition.o \
    $(B)/game/torsoAimAndFireWeapon.o \
    $(B)/game/useAlarm.o \
    $(B)/game/watchEntity.o \
    $(B)/game/watchEntityEX.o \
    $(B)/game/work.o \
    \
    $(B)/game/mp_awardsystem.o \
    $(B)/game/mp_manager.o \
    $(B)/game/mp_modeBase.o \
    $(B)/game/mp_modeCtf.o \
    $(B)/game/mp_modeDm.o \
    $(B)/game/mp_modeTeamBase.o \
    $(B)/game/mp_modeTeamDm.o \
    $(B)/game/mp_modifiers.o \
    $(B)/game/mp_team.o \
    \
    $(B)/game/ai_chat.o \
    $(B)/game/ai_cmd.o \
    $(B)/game/ai_dmnet.o \
    $(B)/game/ai_dmq3.o \
    $(B)/game/ai_main.o \
    $(B)/game/ai_team.o \
    $(B)/game/ai_vcmd.o \
    $(B)/game/g_bot.o \
    \
    $(B)/game/actor.o \
    $(B)/game/actor_combatsubsystem.o \
    $(B)/game/actor_enemymanager.o \
    $(B)/game/actor_headwatcher.o \
    $(B)/game/actor_locomotion.o \
    $(B)/game/actor_posturecontroller.o \
    $(B)/game/actor_sensoryperception.o \
    $(B)/game/actorgamecomponents.o \
    $(B)/game/actorstrategies.o \
    $(B)/game/actorutil.o \
    $(B)/game/ammo.o \
    $(B)/game/animate.o \
    $(B)/game/archive.o \
    $(B)/game/armor.o \
    $(B)/game/beam.o \
    $(B)/game/behavior.o \
    $(B)/game/behaviors_general.o \
    $(B)/game/behaviors_specific.o \
    $(B)/game/bg_misc.o \
    $(B)/game/bg_pmove.o \
    $(B)/game/body.o \
    $(B)/game/bspline.o \
    $(B)/game/camera.o \
    $(B)/game/CameraPath.o \
    $(B)/game/characterstate.o \
    $(B)/game/CinematicArmature.o \
    $(B)/game/class.o \
    $(B)/game/compiler.o \
    $(B)/game/DamageModification.o \
    $(B)/game/debuglines.o \
    $(B)/game/decals.o \
    $(B)/game/dispenser.o \
    $(B)/game/doors.o \
    $(B)/game/earthquake.o \
    $(B)/game/entity.o \
    $(B)/game/equipment.o \
    $(B)/game/explosion.o \
    $(B)/game/FollowPath.o \
    $(B)/game/FollowPathToEntity.o \
    $(B)/game/FollowPathToPoint.o \
    $(B)/game/g_main.o \
    $(B)/game/g_phys.o \
    $(B)/game/g_spawn.o \
    $(B)/game/g_utils.o \
    $(B)/game/game.o \
    $(B)/game/gamecmds.o \
    $(B)/game/gamecvars.o \
    $(B)/game/gameplaydatabase.o \
    $(B)/game/gameplayformulamanager.o \
    $(B)/game/gameplaymanager.o \
    $(B)/game/gamescript.o \
    $(B)/game/gibs.o \
    $(B)/game/globalcmd.o \
    $(B)/game/GoDirectlyToPoint.o \
    $(B)/game/goo.o \
    $(B)/game/gravpath.o \
    $(B)/game/groupcoordinator.o \
    $(B)/game/health.o \
    $(B)/game/helper_node.o \
    $(B)/game/interpreter.o \
    $(B)/game/inventoryitem.o \
    $(B)/game/ipfilter.o \
    $(B)/game/item.o \
    $(B)/game/level.o \
    $(B)/game/lexer.o \
    $(B)/game/light.o \
    $(B)/game/listener.o \
    $(B)/game/misc.o \
    $(B)/game/mover.o \
    $(B)/game/nature.o \
    $(B)/game/navigate.o \
    $(B)/game/object.o \
    $(B)/game/output.o \
    $(B)/game/path.o \
    $(B)/game/player.o \
    $(B)/game/player_combat.o \
    $(B)/game/player_util.o \
    $(B)/game/playerheuristics.o \
    $(B)/game/PlayerStart.o \
    $(B)/game/portal.o \
    $(B)/game/powerups.o \
    $(B)/game/program.o \
    $(B)/game/puzzleobject.o \
    $(B)/game/q_math.o \
    $(B)/game/q_mathsys.o \
    $(B)/game/q_shared.o \
    $(B)/game/RageAI.o \
    $(B)/game/script.o \
    $(B)/game/scriptmaster.o \
    $(B)/game/scriptslave.o \
    $(B)/game/scriptvariable.o \
    $(B)/game/sentient.o \
    $(B)/game/shrapnelbomb.o \
    $(B)/game/soundman.o \
    $(B)/game/spawners.o \
    $(B)/game/specialfx.o \
    $(B)/game/stationaryvehicle.o \
    $(B)/game/steering.o \
    $(B)/game/str.o \
    $(B)/game/teammateroster.o \
    $(B)/game/trigger.o \
    $(B)/game/UseData.o \
    $(B)/game/vehicle.o \
    $(B)/game/viewthing.o \
    $(B)/game/waypoints.o \
    $(B)/game/weapon.o \
    $(B)/game/WeaponDualWield.o \
    $(B)/game/weaputils.o \
    $(B)/game/worldspawn.o 



$(B)/ef2game$(ARCH).$(SHLIBEXT) : $(GOBJ)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(GOBJ)

$(B)/game/changePosture.o : $(GDIR)/changePosture.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/closeInOnEnemy.o : $(GDIR)/closeInOnEnemy.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/closeInOnEnemyWhileFiringWeapon.o : $(GDIR)/closeInOnEnemyWhileFiringWeapon.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/closeInOnPlayer.o : $(GDIR)/closeInOnPlayer.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/corridorCombatWithRangedWeapon.o : $(GDIR)/corridorCombatWithRangedWeapon.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/coverCombatWithRangedWeapon.o : $(GDIR)/coverCombatWithRangedWeapon.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/doAttack.o : $(GDIR)/doAttack.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/generalCombatWithMeleeWeapon.o : $(GDIR)/generalCombatWithMeleeWeapon.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/generalCombatWithRangedWeapon.o : $(GDIR)/generalCombatWithRangedWeapon.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/gotoCurrentHelperNode.o : $(GDIR)/gotoCurrentHelperNode.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/gotoHelperNode.o : $(GDIR)/gotoHelperNode.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/gotoHelperNodeEX.o : $(GDIR)/gotoHelperNodeEX.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/gotoHelperNodeNearestEnemy.o : $(GDIR)/gotoHelperNodeNearestEnemy.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/healGroupMember.o : $(GDIR)/healGroupMember.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/holdPosition.o : $(GDIR)/holdPosition.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/MoveRandomDirection.o : $(GDIR)/MoveRandomDirection.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/patrol.o : $(GDIR)/patrol.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/PlayAnim.o : $(GDIR)/PlayAnim.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/rangedCombatWithWeapon.o : $(GDIR)/rangedCombatWithWeapon.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/rotateToEntity.o : $(GDIR)/rotateToEntity.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/selectBestWeapon.o : $(GDIR)/selectBestWeapon.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/snipeEnemy.o : $(GDIR)/snipeEnemy.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/stationaryFireCombat.o : $(GDIR)/stationaryFireCombat.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/stationaryFireCombatEX.o : $(GDIR)/stationaryFireCombatEX.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/suppressionFireCombat.o : $(GDIR)/suppressionFireCombat.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/talk.o : $(GDIR)/talk.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/teleportToEntity.o : $(GDIR)/teleportToEntity.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/teleportToPosition.o : $(GDIR)/teleportToPosition.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/torsoAimAndFireWeapon.o : $(GDIR)/torsoAimAndFireWeapon.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/useAlarm.o : $(GDIR)/useAlarm.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/watchEntity.o : $(GDIR)/watchEntity.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/watchEntityEX.o : $(GDIR)/watchEntityEX.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/work.o : $(GDIR)/work.cpp; $(DO_GAME_SHLIB_CC)

$(B)/game/mp_awardsystem.o : $(GDIR)/mp_awardsystem.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/mp_manager.o : $(GDIR)/mp_manager.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/mp_modeBase.o : $(GDIR)/mp_modeBase.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/mp_modeCtf.o : $(GDIR)/mp_modeCtf.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/mp_modeDm.o : $(GDIR)/mp_modeDm.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/mp_modeTeamBase.o : $(GDIR)/mp_modeTeamBase.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/mp_modeTeamDm.o : $(GDIR)/mp_modeTeamDm.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/mp_modifiers.o : $(GDIR)/mp_modifiers.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/mp_team.o : $(GDIR)/mp_team.cpp; $(DO_GAME_SHLIB_CC)

$(B)/game/ai_chat.o : $(GDIR)/ai_chat.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/ai_cmd.o : $(GDIR)/ai_cmd.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/ai_dmnet.o : $(GDIR)/ai_dmnet.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/ai_dmq3.o : $(GDIR)/ai_dmq3.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/ai_main.o : $(GDIR)/ai_main.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/ai_team.o : $(GDIR)/ai_team.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/ai_vcmd.o : $(GDIR)/ai_vcmd.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/g_bot.o : $(GDIR)/g_bot.cpp; $(DO_GAME_SHLIB_CC)

$(B)/game/actor.o : $(GDIR)/actor.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/actor_combatsubsystem.o : $(GDIR)/actor_combatsubsystem.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/actor_enemymanager.o : $(GDIR)/actor_enemymanager.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/actor_headwatcher.o : $(GDIR)/actor_headwatcher.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/actor_locomotion.o  : $(GDIR)/actor_locomotion.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/actor_posturecontroller.o : $(GDIR)/actor_posturecontroller.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/actor_sensoryperception.o : $(GDIR)/actor_sensoryperception.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/actorgamecomponents.o : $(GDIR)/actorgamecomponents.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/actorstrategies.o : $(GDIR)/actorstrategies.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/actorutil.o : $(GDIR)/actorutil.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/ammo.o : $(GDIR)/ammo.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/animate.o : $(GDIR)/animate.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/archive.o : $(GDIR)/archive.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/armor.o : $(GDIR)/armor.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/beam.o : $(GDIR)/beam.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/behavior.o : $(GDIR)/behavior.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/behaviors_general.o : $(GDIR)/behaviors_general.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/behaviors_specific.o : $(GDIR)/behaviors_specific.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/bg_misc.o : $(GDIR)/bg_misc.c; $(DO_GAME_SHLIB_GC)
$(B)/game/bg_pmove.o : $(GDIR)/bg_pmove.c; $(DO_GAME_SHLIB_GC)
$(B)/game/body.o : $(GDIR)/body.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/bspline.o : $(GDIR)/bspline.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/camera.o : $(GDIR)/camera.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/CameraPath.o : $(GDIR)/CameraPath.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/characterstate.o : $(GDIR)/characterstate.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/CinematicArmature.o : $(GDIR)/CinematicArmature.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/class.o : $(GDIR)/class.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/compiler.o : $(GDIR)/compiler.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/DamageModification.o : $(GDIR)/DamageModification.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/debuglines.o : $(GDIR)/debuglines.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/decals.o : $(GDIR)/decals.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/dispenser.o : $(GDIR)/dispenser.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/doors.o : $(GDIR)/doors.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/earthquake.o : $(GDIR)/earthquake.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/entity.o : $(GDIR)/entity.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/equipment.o : $(GDIR)/equipment.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/explosion.o : $(GDIR)/explosion.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/FollowPath.o : $(GDIR)/FollowPath.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/FollowPathToEntity.o : $(GDIR)/FollowPathToEntity.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/FollowPathToPoint.o : $(GDIR)/FollowPathToPoint.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/g_main.o : $(GDIR)/g_main.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/g_phys.o : $(GDIR)/g_phys.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/g_spawn.o : $(GDIR)/g_spawn.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/g_utils.o : $(GDIR)/g_utils.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/game.o : $(GDIR)/game.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/gamecmds.o : $(GDIR)/gamecmds.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/gamecvars.o : $(GDIR)/gamecvars.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/gameplaydatabase.o : $(CMDIR)/gameplaydatabase.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/gameplayformulamanager.o : $(CMDIR)/gameplayformulamanager.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/gameplaymanager.o : $(CMDIR)/gameplaymanager.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/gamescript.o : $(GDIR)/gamescript.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/gibs.o : $(GDIR)/gibs.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/globalcmd.o : $(GDIR)/globalcmd.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/GoDirectlyToPoint.o : $(GDIR)/GoDirectlyToPoint.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/goo.o : $(GDIR)/goo.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/gravpath.o : $(GDIR)/gravpath.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/groupcoordinator.o : $(GDIR)/groupcoordinator.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/health.o : $(GDIR)/health.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/helper_node.o : $(GDIR)/helper_node.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/interpreter.o : $(GDIR)/interpreter.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/inventoryitem.o : $(GDIR)/inventoryitem.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/ipfilter.o : $(GDIR)/ipfilter.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/item.o : $(GDIR)/item.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/level.o : $(GDIR)/level.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/lexer.o : $(GDIR)/lexer.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/light.o : $(GDIR)/light.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/listener.o : $(GDIR)/listener.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/misc.o : $(GDIR)/misc.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/mover.o : $(GDIR)/mover.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/nature.o : $(GDIR)/nature.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/navigate.o : $(GDIR)/navigate.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/object.o : $(GDIR)/object.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/output.o : $(CMDIR)/output.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/path.o : $(GDIR)/path.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/player.o : $(GDIR)/player.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/player_combat.o : $(GDIR)/player_combat.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/player_util.o : $(GDIR)/player_util.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/playerheuristics.o : $(GDIR)/playerheuristics.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/PlayerStart.o : $(GDIR)/PlayerStart.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/portal.o : $(GDIR)/portal.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/powerups.o : $(GDIR)/powerups.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/program.o : $(GDIR)/program.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/puzzleobject.o : $(GDIR)/puzzleobject.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/q_math.o : $(GDIR)/q_math.c; $(DO_GAME_SHLIB_GC)
$(B)/game/q_mathsys.o : $(GDIR)/q_mathsys.c; $(DO_GAME_SHLIB_GC)
$(B)/game/q_shared.o : $(GDIR)/q_shared.c; $(DO_GAME_SHLIB_GC)
$(B)/game/RageAI.o : $(GDIR)/RageAI.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/script.o : $(GDIR)/script.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/scriptmaster.o : $(GDIR)/scriptmaster.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/scriptslave.o : $(GDIR)/scriptslave.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/scriptvariable.o : $(GDIR)/scriptvariable.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/sentient.o : $(GDIR)/sentient.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/shrapnelbomb.o : $(GDIR)/shrapnelbomb.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/soundman.o : $(GDIR)/soundman.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/spawners.o : $(GDIR)/spawners.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/specialfx.o : $(GDIR)/specialfx.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/stationaryvehicle.o : $(GDIR)/stationaryvehicle.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/steering.o : $(GDIR)/steering.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/str.o : $(GDIR)/str.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/teammateroster.o : $(GDIR)/teammateroster.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/trigger.o : $(GDIR)/trigger.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/UseData.o : $(GDIR)/UseData.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/vehicle.o : $(GDIR)/vehicle.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/viewthing.o : $(GDIR)/viewthing.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/waypoints.o : $(GDIR)/waypoints.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/weapon.o : $(GDIR)/weapon.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/WeaponDualWield.o : $(GDIR)/WeaponDualWield.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/weaputils.o : $(GDIR)/weaputils.cpp; $(DO_GAME_SHLIB_CC)
$(B)/game/worldspawn.o : $(GDIR)/worldspawn.cpp; $(DO_GAME_SHLIB_CC)

#############################################################################
# RPM
#############################################################################

TMPDIR=/var/tmp
TARDIR=$(TMPDIR)/$(BUILD_NAME)
TARFILE = $(BUILD_NAME)-$(VERSION)-$(RPM_RELEASE).$(ARCH).tar

tar:
	if [ ! -d archives ];then mkdir archives;chmod 755 archives;fi
	$(MAKE) copyfiles COPYDIR=$(TARDIR)
	cd $(TARDIR); tar cvf $(TARFILE) * && gzip -9 $(TARFILE)
	mv $(TARDIR)/$(TARFILE).gz archives/.
	rm -rf $(TARDIR)
 
# Make RPMs.  You need to be root to make this work
RPMROOT=/usr/src/redhat
RPM = rpm
RPMFLAGS = -bb
INSTALLDIR = /usr/local/games/$(BUILD_NAME)
RPMDIR = $(TMPDIR)/$(BUILD_NAME)-$(VERSION)
DESTDIR= $(RPMDIR)/$(INSTALLDIR)

rpm: $(BUILD_NAME).spec
	touch $(RPMROOT)/SOURCES/$(BUILD_NAME)-$(VERSION).tar.gz
	if [ ! -d archives ];then mkdir archives;fi
	$(MAKE) copyfiles COPYDIR=$(DESTDIR)
	cp $(UDIR)/quake3.gif $(RPMROOT)/SOURCES/.
	cp $(BUILD_NAME).spec $(RPMROOT)/SPECS/.
	cd $(RPMROOT)/SPECS; $(RPM) $(RPMFLAGS) $(BUILD_NAME).spec
	rm -rf $(RPMDIR)
	mv $(RPMROOT)/RPMS/$(RPMARCH)/$(BUILD_NAME)-$(VERSION)-$(RPM_RELEASE).$(RPMARCH).rpm archives/$(BUILD_NAME)-$(VERSION)-$(RPM_RELEASE).$(RPMARCH).rpm 

copyfiles:
	-mkdirhier $(COPYDIR)
	cp $(BR)/linuxquake3 $(COPYDIR)/quake3.x86
	strip $(COPYDIR)/quake3.x86
	chmod 755 $(COPYDIR)/quake3.x86
	cp $(BR)/linuxq3ded $(COPYDIR)/q3ded
	strip $(COPYDIR)/q3ded
	chmod 755 $(COPYDIR)/q3ded
	cp $(BDIR)/libMesaVoodooGL.so.3.2 $(COPYDIR)/.
	chmod 755 $(COPYDIR)/libMesaVoodooGL.so.3.2
	( cd $(COPYDIR); ln -s libMesaVoodooGL.so.3.2 libMesaVoodooGL.so )
	cp $(BDIR)/Quake_III_Arena_FAQ.html $(COPYDIR)/.
	chmod 644 $(COPYDIR)/Quake_III_Arena_FAQ.html
	mkdir $(COPYDIR)/baseq3
	cp $(BASEQ3_DIR)/pak2.pk3 $(COPYDIR)/baseq3/.
	chmod 644 $(COPYDIR)/baseq3/pak2.pk3

$(BUILD_NAME).spec : $(UDIR)/$(BUILD_NAME).spec.sh Makefile
	sh $< $(VERSION) $(RPM_RELEASE) $(ARCH) $(INSTALLDIR) > $@

#############################################################################
# MISC
#############################################################################
clean:clean-dedicated

clean-dedicated:
	rm -rf $(BR)/ded/*
	rm -rf $(BR)/game/*
	rm -rf $(BD)/ded/*
	rm -rf $(BD)/game/*

#clean:clean-debug clean-release

#clean-debug:
#	$(MAKE) clean2 B=$(BD) CFLAGS="$(DEBUG_CFLAGS)"

#clean-release:
#	$(MAKE) clean2 B=$(BR) CFLAGS="$(DEBUG_CFLAGS)"

