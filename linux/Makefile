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
PLATFORM_ARCH=$(shell uname -m | sed -e s/i.86/i386/)

###
### These paths are where you probably want to change things
###

# Where we are building from (where the source code should be!)
MOUNT_DIR=..

#############################################################################
##
## You shouldn't have to touch anything below here
##
#############################################################################
BD=debug$(ARCH)$(GLIBC)
BR=release$(ARCH)$(GLIBC)
CMDIR=$(MOUNT_DIR)/Shared/qcommon
GDIR=$(MOUNT_DIR)/dlls/game
DLLINC = $(MOUNT_DIR)/DLLs
SHAREINC = $(MOUNT_DIR)/Shared
EXECINC  = $(MOUNT_DIR)/Executable
LIBINC   = $(MOUNT_DIR)/Libs

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

BASE_CFLAGS=-Dstricmp=strcasecmp -Dstrcmpi=strcasecmp -Dstrnicmp=strncasecmp -pipe -I../dlls
ifeq ($(PLATFORM_ARCH),x86_64)
BASE_CFLAGS+=-m32
endif

DEBUG_CFLAGS=$(BASE_CFLAGS) -g -Wno-deprecated
ifeq ($(ARCH),axp)
CC=g++
GC=gcc
RELEASE_CFLAGS=$(BASE_CFLAGS) -DNDEBUG -O6 -ffast-math -funroll-loops -fomit-frame-pointer -fexpensive-optimizations
else
CC=g++
GC=gcc
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

TARGETS=\
	$(B)/ef2game$(ARCH).$(SHLIBEXT)

else # ifeq Linux

#############################################################################
# SETUP AND BUILD -- GENERIC
#############################################################################

CC=cc
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

DO_GAME_SHLIB_CC=$(CC) -DGAME_DLL -DDEDICATED -DMISSIONPACK -DLINUX $(CFLAGS) $(SHLIBCFLAGS) -I$(DLLINC) -I$(SHAREINC) -I$(EXECINC) -I$(CMDIR) -I$(LIBINC) -o $@ -c $<
DO_GAME_SHLIB_GC=$(GC) -DGAME_DLL -DDEDICATED -DMISSIONPACK -DLINUX $(CFLAGS) $(SHLIBCFLAGS) -I$(DLLINC) -I$(SHAREINC) -I$(EXECINC) -I$(CMDIR) -I$(LIBINC) -o $@ -c $<

#### DEFAULT TARGET
# default:build_release
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
	if [ ! -d $(B)/game ]; then mkdir $(B)/game;fi

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
# MISC
#############################################################################
clean:
	rm -rf $(BR)/game/*
	rm -rf $(BD)/game/*



