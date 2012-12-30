# Microsoft Developer Studio Project File - Name="game" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=game - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "game.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "game.mak" CFG="game - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "game - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "game - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "game - Win32 Demo Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "game - Win32 VTune" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "game - Win32 Intel Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "game - Win32 Release CDROM" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/EF2/game", FYHAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "game - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "./Release"
# PROP Intermediate_Dir "./Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FGAME_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "..\..\Shared" /I "..\..\DLLs" /I "..\..\Executable" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FGAME_EXPORTS" /D "GAME_DLL" /D "MISSIONPACK" /D "ENABLE_ALTROUTING" /D "MSVC_BUILD" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib winmm.lib /nologo /dll /map /machine:I386 /out:"../../Executable/Release/gamex86.dll"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug"
# PROP Intermediate_Dir "./Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FGAME_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W4 /Gm /GX /ZI /Od /I "..\..\Shared" /I "..\..\DLLs" /I "..\..\Executable" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FGAME_EXPORTS" /D "GAME_DLL" /D "MISSIONPACK" /D "ENABLE_ALTROUTING" /D "MSVC_BUILD" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"../../Executable/Debug/game.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib winmm.lib /nologo /dll /pdb:"Debug/gamex86.pdb" /debug /machine:I386 /out:"../../Executable/Debug/gamex86.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "game___Win32_Demo_Release"
# PROP BASE Intermediate_Dir "game___Win32_Demo_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Demo_Release"
# PROP Intermediate_Dir "Demo_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FGAME_EXPORTS" /D "GAME_DLL" /FR /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FGAME_EXPORTS" /D "GAME_DLL" /D "NDEBUG" /D "WIN32" /D "PRE_RELEASE_DEMO" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib winmm.lib /nologo /dll /map /machine:I386 /out:"../Release/gamex86.dll"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 kernel32.lib user32.lib winmm.lib /nologo /dll /map /machine:I386 /out:"../Demo_Release/gamex86.dll"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "game___Win32_VTune"
# PROP BASE Intermediate_Dir "game___Win32_VTune"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "VTune"
# PROP Intermediate_Dir "VTune"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W4 /Gm /GX /ZI /Od /I "..\..\Shared" /I "..\..\DLLs" /I "..\..\Executable" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FGAME_EXPORTS" /D "GAME_DLL" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W4 /Gm /GX /Zi /O2 /I "..\..\Shared" /I "..\..\DLLs" /I "..\..\Executable" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FGAME_EXPORTS" /D "GAME_DLL" /D "MISSIONPACK" /D "ENABLE_ALTROUTING" /D "MSVC_BUILD" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib winmm.lib /nologo /dll /debug /machine:I386 /out:"../../Executable/Debug/gamex86.dll" /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib winmm.lib /nologo /dll /debug /machine:I386 /out:"../../Executable/VTune/gamex86.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "game___Win32_Intel_Release"
# PROP BASE Intermediate_Dir "game___Win32_Intel_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "./IntelRelease"
# PROP Intermediate_Dir "./IntelRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /O2 /I "..\..\Shared" /I "..\..\DLLs" /I "..\..\Executable" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FGAME_EXPORTS" /D "GAME_DLL" /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "..\..\Shared" /I "..\..\DLLs" /I "..\..\Executable" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FGAME_EXPORTS" /D "GAME_DLL" /D "_USE_INTEL_COMPILER" /D "MISSIONPACK" /D "ENABLE_ALTROUTING" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib winmm.lib /nologo /dll /map /machine:I386 /out:"../../Executable/Release/gamex86.dll"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 kernel32.lib user32.lib winmm.lib /nologo /dll /map /machine:I386 /out:"../../Executable/IntelRelease/gamex86.dll"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "game___Win32_Release_CDROM0"
# PROP BASE Intermediate_Dir "game___Win32_Release_CDROM0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /O2 /I "..\..\Shared" /I "..\..\DLLs" /I "..\..\Executable" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FGAME_EXPORTS" /D "GAME_DLL" /D "MISSIONPACK" /D "ENABLE_ALTROUTING" /D "MSVC_BUILD" /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "..\..\Shared" /I "..\..\DLLs" /I "..\..\Executable" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FGAME_EXPORTS" /D "GAME_DLL" /D "MISSIONPACK" /D "ENABLE_ALTROUTING" /D "MSVC_BUILD" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib winmm.lib /nologo /dll /map /machine:I386 /out:"../../Executable/Release/gamex86.dll"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 kernel32.lib user32.lib winmm.lib /nologo /dll /map /machine:I386 /out:"../../Executable/Release/gamex86.dll"
# SUBTRACT LINK32 /debug

!ENDIF 

# Begin Target

# Name "game - Win32 Release"
# Name "game - Win32 Debug"
# Name "game - Win32 Demo Release"
# Name "game - Win32 VTune"
# Name "game - Win32 Intel Release"
# Name "game - Win32 Release CDROM"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Behaviors"

# PROP Default_Filter ".cpp"
# Begin Source File

SOURCE=.\changePosture.cpp
# End Source File
# Begin Source File

SOURCE=.\closeInOnEnemy.cpp
# End Source File
# Begin Source File

SOURCE=.\closeInOnEnemyWhileFiringWeapon.cpp
# End Source File
# Begin Source File

SOURCE=.\closeInOnPlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\corridorCombatWithRangedWeapon.cpp
# End Source File
# Begin Source File

SOURCE=.\coverCombatWithRangedWeapon.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\doAttack.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\generalCombatWithMeleeWeapon.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\generalCombatWithRangedWeapon.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gotoCurrentHelperNode.cpp
# End Source File
# Begin Source File

SOURCE=.\gotoHelperNode.cpp
# End Source File
# Begin Source File

SOURCE=.\gotoHelperNodeEX.cpp
# End Source File
# Begin Source File

SOURCE=.\gotoHelperNodeNearestEnemy.cpp
# End Source File
# Begin Source File

SOURCE=.\healGroupMember.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\holdPosition.cpp
# End Source File
# Begin Source File

SOURCE=.\MoveRandomDirection.cpp
# End Source File
# Begin Source File

SOURCE=.\patrol.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayAnim.cpp
# End Source File
# Begin Source File

SOURCE=.\rangedCombatWithWeapon.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\rotateToEntity.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\selectBestWeapon.cpp
# End Source File
# Begin Source File

SOURCE=.\snipeEnemy.cpp
# End Source File
# Begin Source File

SOURCE=.\stationaryFireCombat.cpp
# End Source File
# Begin Source File

SOURCE=.\stationaryFireCombatEX.cpp
# End Source File
# Begin Source File

SOURCE=.\suppressionFireCombat.cpp
# End Source File
# Begin Source File

SOURCE=.\talk.cpp
# End Source File
# Begin Source File

SOURCE=.\teleportToEntity.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\teleportToPosition.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\torsoAimAndFireWeapon.cpp
# End Source File
# Begin Source File

SOURCE=.\useAlarm.cpp
# End Source File
# Begin Source File

SOURCE=.\watchEntity.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\watchEntityEX.cpp
# End Source File
# Begin Source File

SOURCE=.\work.cpp
# End Source File
# End Group
# Begin Group "Multiplayer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\mp_awardsystem.cpp
# End Source File
# Begin Source File

SOURCE=.\mp_manager.cpp
# End Source File
# Begin Source File

SOURCE=.\mp_modeBase.cpp
# End Source File
# Begin Source File

SOURCE=.\mp_modeCtf.cpp
# End Source File
# Begin Source File

SOURCE=.\mp_modeDm.cpp
# End Source File
# Begin Source File

SOURCE=.\mp_modeTeamBase.cpp
# End Source File
# Begin Source File

SOURCE=.\mp_modeTeamDm.cpp
# End Source File
# Begin Source File

SOURCE=.\mp_modifiers.cpp
# End Source File
# Begin Source File

SOURCE=.\mp_team.cpp
# End Source File
# End Group
# Begin Group "botlib source"

# PROP Default_Filter ".cpp"
# Begin Source File

SOURCE=.\ai_chat.cpp
# End Source File
# Begin Source File

SOURCE=.\ai_cmd.cpp
# End Source File
# Begin Source File

SOURCE=.\ai_dmnet.cpp
# End Source File
# Begin Source File

SOURCE=.\ai_dmq3.cpp
# End Source File
# Begin Source File

SOURCE=.\ai_main.cpp
# End Source File
# Begin Source File

SOURCE=.\ai_team.cpp
# End Source File
# Begin Source File

SOURCE=.\ai_vcmd.cpp
# End Source File
# Begin Source File

SOURCE=.\be_ea.h
# End Source File
# Begin Source File

SOURCE=.\g_bot.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\_pch_cpp.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yc"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yc"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yc"_pch_cpp.h"
# ADD CPP /Yc"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yc"_pch_cpp.h"
# ADD CPP /Yc"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\actor.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\actor_combatsubsystem.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\actor_enemymanager.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\actor_headwatcher.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\actor_locomotion.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\actor_posturecontroller.cpp
# End Source File
# Begin Source File

SOURCE=.\actor_sensoryperception.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\actorgamecomponents.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\actorstrategies.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\actorutil.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ammo.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\animate.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\archive.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\armor.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\beam.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\behavior.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\behaviors_general.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\behaviors_specific.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\bg_misc.c

!IF  "$(CFG)" == "game - Win32 Release"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# SUBTRACT CPP /YX /Yc

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\bg_pmove.c

!IF  "$(CFG)" == "game - Win32 Release"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# SUBTRACT CPP /YX /Yc

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\body.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\bspline.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\camera.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CameraPath.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\characterstate.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CinematicArmature.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\class.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\compiler.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DamageModification.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\debuglines.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\decals.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dispenser.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\doors.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\earthquake.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\entity.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\equipment.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\explosion.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\FollowPath.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\FollowPathToEntity.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\FollowPathToPoint.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\g_main.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\g_phys.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\g_spawn.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\g_utils.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game.def
# End Source File
# Begin Source File

SOURCE=.\gamecmds.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gamecvars.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Shared\qcommon\gameplaydatabase.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Shared\qcommon\gameplayformulamanager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Shared\qcommon\gameplaymanager.cpp
# End Source File
# Begin Source File

SOURCE=.\gamescript.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gibs.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\globalcmd.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GoDirectlyToPoint.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\goo.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gravpath.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\groupcoordinator.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\health.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\helper_node.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interpreter.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\inventoryitem.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ipfilter.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\item.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\level.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\lexer.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\light.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\listener.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\misc.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\mover.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\nature.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\navigate.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\object.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Shared\qcommon\output.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\path.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\player.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\player_combat.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\player_util.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\playerheuristics.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PlayerStart.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\portal.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\powerups.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\program.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\puzzleobject.cpp
# End Source File
# Begin Source File

SOURCE=.\q_math.c

!IF  "$(CFG)" == "game - Win32 Release"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\q_mathsys.c

!IF  "$(CFG)" == "game - Win32 Release"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\q_shared.c

!IF  "$(CFG)" == "game - Win32 Release"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageAI.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\script.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scriptmaster.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scriptslave.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scriptvariable.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sentient.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shrapnelbomb.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\soundman.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\spawners.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\specialfx.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\stationaryvehicle.cpp
# End Source File
# Begin Source File

SOURCE=.\steering.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\str.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\teammateroster.cpp
# End Source File
# Begin Source File

SOURCE=.\trigger.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\UseData.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\vehicle.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\viewthing.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\waypoints.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\weapon.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\WeaponDualWield.cpp
# End Source File
# Begin Source File

SOURCE=.\weaputils.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Executable\win32\win_bounds.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# SUBTRACT BASE CPP /YX
# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\worldspawn.cpp

!IF  "$(CFG)" == "game - Win32 Release"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Demo Release"

!ELSEIF  "$(CFG)" == "game - Win32 VTune"

!ELSEIF  "$(CFG)" == "game - Win32 Intel Release"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ELSEIF  "$(CFG)" == "game - Win32 Release CDROM"

# ADD BASE CPP /Yu"_pch_cpp.h"
# ADD CPP /Yu"_pch_cpp.h"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "BehaviorHeaders"

# PROP Default_Filter ".hpp"
# Begin Source File

SOURCE=.\changePosture.hpp
# End Source File
# Begin Source File

SOURCE=.\closeInOnEnemy.hpp
# End Source File
# Begin Source File

SOURCE=.\closeInOnEnemyWhileFiringWeapon.hpp
# End Source File
# Begin Source File

SOURCE=.\closeInOnPlayer.hpp
# End Source File
# Begin Source File

SOURCE=.\corridorCombatWithRangedWeapon.hpp
# End Source File
# Begin Source File

SOURCE=.\coverCombatWithRangedWeapon.hpp
# End Source File
# Begin Source File

SOURCE=.\doAttack.hpp
# End Source File
# Begin Source File

SOURCE=.\generalCombatWithMeleeWeapon.hpp
# End Source File
# Begin Source File

SOURCE=.\generalCombatWithRangedWeapon.hpp
# End Source File
# Begin Source File

SOURCE=.\gotoCurrentHelperNode.hpp
# End Source File
# Begin Source File

SOURCE=.\gotoHelperNode.hpp
# End Source File
# Begin Source File

SOURCE=.\gotoHelperNodeEX.hpp
# End Source File
# Begin Source File

SOURCE=.\gotoHelperNodeNearestEnemy.hpp
# End Source File
# Begin Source File

SOURCE=.\healGroupMember.hpp
# End Source File
# Begin Source File

SOURCE=.\holdPosition.hpp
# End Source File
# Begin Source File

SOURCE=.\MoveRandomDirection.hpp
# End Source File
# Begin Source File

SOURCE=.\patrol.hpp
# End Source File
# Begin Source File

SOURCE=.\PlayAnim.hpp
# End Source File
# Begin Source File

SOURCE=.\rangedCombatWithWeapon.hpp
# End Source File
# Begin Source File

SOURCE=.\rotateToEntity.hpp
# End Source File
# Begin Source File

SOURCE=.\selectBestWeapon.hpp
# End Source File
# Begin Source File

SOURCE=.\snipeEnemy.hpp
# End Source File
# Begin Source File

SOURCE=.\stationaryFireCombat.hpp
# End Source File
# Begin Source File

SOURCE=.\stationaryFireCombatEX.hpp
# End Source File
# Begin Source File

SOURCE=.\suppressionFireCombat.hpp
# End Source File
# Begin Source File

SOURCE=.\talk.hpp
# End Source File
# Begin Source File

SOURCE=.\teleportToEntity.hpp
# End Source File
# Begin Source File

SOURCE=.\teleportToPosition.hpp
# End Source File
# Begin Source File

SOURCE=.\torsoAimAndFireWeapon.hpp
# End Source File
# Begin Source File

SOURCE=.\useAlarm.hpp
# End Source File
# Begin Source File

SOURCE=.\watchEntity.hpp
# End Source File
# Begin Source File

SOURCE=.\watchEntityEX.hpp
# End Source File
# Begin Source File

SOURCE=.\work.hpp
# End Source File
# End Group
# Begin Group "Multiplayer Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\mp_awardsystem.hpp
# End Source File
# Begin Source File

SOURCE=.\mp_manager.hpp
# End Source File
# Begin Source File

SOURCE=.\mp_modeBase.hpp
# End Source File
# Begin Source File

SOURCE=.\mp_modeCtf.hpp
# End Source File
# Begin Source File

SOURCE=.\mp_modeDm.hpp
# End Source File
# Begin Source File

SOURCE=.\mp_modeTeamBase.hpp
# End Source File
# Begin Source File

SOURCE=.\mp_modeTeamDm.hpp
# End Source File
# Begin Source File

SOURCE=.\mp_modifiers.hpp
# End Source File
# Begin Source File

SOURCE=.\mp_shared.hpp
# End Source File
# Begin Source File

SOURCE=.\mp_team.hpp
# End Source File
# End Group
# Begin Group "botlib header files"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\ai_chat.h
# End Source File
# Begin Source File

SOURCE=.\ai_cmd.h
# End Source File
# Begin Source File

SOURCE=.\ai_dmnet.h
# End Source File
# Begin Source File

SOURCE=.\ai_dmq3.h
# End Source File
# Begin Source File

SOURCE=.\ai_main.h
# End Source File
# Begin Source File

SOURCE=.\ai_team.h
# End Source File
# Begin Source File

SOURCE=.\ai_vcmd.h
# End Source File
# Begin Source File

SOURCE=.\be_aas.h
# End Source File
# Begin Source File

SOURCE=.\be_ai_char.h
# End Source File
# Begin Source File

SOURCE=.\be_ai_chat.h
# End Source File
# Begin Source File

SOURCE=.\be_ai_gen.h
# End Source File
# Begin Source File

SOURCE=.\be_ai_goal.h
# End Source File
# Begin Source File

SOURCE=.\be_ai_move.h
# End Source File
# Begin Source File

SOURCE=.\be_ai_weap.h
# End Source File
# Begin Source File

SOURCE=.\botlib.h
# End Source File
# Begin Source File

SOURCE=.\botmenudef.h
# End Source File
# Begin Source File

SOURCE=.\chars.h
# End Source File
# Begin Source File

SOURCE=.\inv.h
# End Source File
# Begin Source File

SOURCE=.\match.h
# End Source File
# Begin Source File

SOURCE=.\syn.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\_pch_cpp.h
# End Source File
# Begin Source File

SOURCE=.\actor.h
# End Source File
# Begin Source File

SOURCE=.\actor_combatsubsystem.h
# End Source File
# Begin Source File

SOURCE=.\actor_enemymanager.h
# End Source File
# Begin Source File

SOURCE=.\actor_headwatcher.h
# End Source File
# Begin Source File

SOURCE=.\actor_locomotion.h
# End Source File
# Begin Source File

SOURCE=.\actor_posturecontroller.hpp
# End Source File
# Begin Source File

SOURCE=.\actor_sensoryperception.h
# End Source File
# Begin Source File

SOURCE=.\actorgamecomponents.h
# End Source File
# Begin Source File

SOURCE=.\actorincludes.h
# End Source File
# Begin Source File

SOURCE=.\actorstrategies.h
# End Source File
# Begin Source File

SOURCE=.\actorutil.h
# End Source File
# Begin Source File

SOURCE=.\ammo.h
# End Source File
# Begin Source File

SOURCE=.\animate.h
# End Source File
# Begin Source File

SOURCE=.\archive.h
# End Source File
# Begin Source File

SOURCE=.\armor.h
# End Source File
# Begin Source File

SOURCE=.\beam.h
# End Source File
# Begin Source File

SOURCE=.\behavior.h
# End Source File
# Begin Source File

SOURCE=.\behaviors.h
# End Source File
# Begin Source File

SOURCE=.\behaviors_general.h
# End Source File
# Begin Source File

SOURCE=.\behaviors_specific.h
# End Source File
# Begin Source File

SOURCE=.\bg_local.h
# End Source File
# Begin Source File

SOURCE=.\bg_public.h
# End Source File
# Begin Source File

SOURCE=.\bit_vector.h
# End Source File
# Begin Source File

SOURCE=.\body.h
# End Source File
# Begin Source File

SOURCE=.\bspline.h
# End Source File
# Begin Source File

SOURCE=.\camera.h
# End Source File
# Begin Source File

SOURCE=.\CameraPath.h
# End Source File
# Begin Source File

SOURCE=.\characterstate.h
# End Source File
# Begin Source File

SOURCE=.\CinematicArmature.h
# End Source File
# Begin Source File

SOURCE=.\class.h
# End Source File
# Begin Source File

SOURCE=.\compiler.h
# End Source File
# Begin Source File

SOURCE=.\container.h
# End Source File
# Begin Source File

SOURCE=.\DamageModification.hpp
# End Source File
# Begin Source File

SOURCE=.\debuglines.h
# End Source File
# Begin Source File

SOURCE=.\decals.h
# End Source File
# Begin Source File

SOURCE=.\dispenser.hpp
# End Source File
# Begin Source File

SOURCE=.\doors.h
# End Source File
# Begin Source File

SOURCE=.\earthquake.h
# End Source File
# Begin Source File

SOURCE=.\entity.h
# End Source File
# Begin Source File

SOURCE=.\equipment.h
# End Source File
# Begin Source File

SOURCE=.\explosion.h
# End Source File
# Begin Source File

SOURCE=.\FollowPath.h
# End Source File
# Begin Source File

SOURCE=.\FollowPathToEntity.h
# End Source File
# Begin Source File

SOURCE=.\FollowPathToPoint.h
# End Source File
# Begin Source File

SOURCE=.\g_local.h
# End Source File
# Begin Source File

SOURCE=.\g_main.h
# End Source File
# Begin Source File

SOURCE=.\g_phys.h
# End Source File
# Begin Source File

SOURCE=.\g_public.h
# End Source File
# Begin Source File

SOURCE=.\g_spawn.h
# End Source File
# Begin Source File

SOURCE=.\g_utils.h
# End Source File
# Begin Source File

SOURCE=.\game.h
# End Source File
# Begin Source File

SOURCE=.\gamecmds.h
# End Source File
# Begin Source File

SOURCE=.\gamecvars.h
# End Source File
# Begin Source File

SOURCE=..\..\Shared\qcommon\gameplaydatabase.h
# End Source File
# Begin Source File

SOURCE=..\..\Shared\qcommon\gameplayformulamanager.h
# End Source File
# Begin Source File

SOURCE=..\..\Shared\qcommon\gameplaymanager.h
# End Source File
# Begin Source File

SOURCE=.\gamescript.h
# End Source File
# Begin Source File

SOURCE=.\gibs.h
# End Source File
# Begin Source File

SOURCE=.\globalcmd.h
# End Source File
# Begin Source File

SOURCE=.\GoDirectlyToPoint.h
# End Source File
# Begin Source File

SOURCE=.\goo.h
# End Source File
# Begin Source File

SOURCE=.\gravpath.h
# End Source File
# Begin Source File

SOURCE=.\groupcoordinator.hpp
# End Source File
# Begin Source File

SOURCE=.\health.h
# End Source File
# Begin Source File

SOURCE=.\helper_node.h
# End Source File
# Begin Source File

SOURCE=.\interpreter.h
# End Source File
# Begin Source File

SOURCE=.\inventoryitem.h
# End Source File
# Begin Source File

SOURCE=.\ipfilter.h
# End Source File
# Begin Source File

SOURCE=.\item.h
# End Source File
# Begin Source File

SOURCE=.\level.h
# End Source File
# Begin Source File

SOURCE=.\lexer.h
# End Source File
# Begin Source File

SOURCE=.\light.h
# End Source File
# Begin Source File

SOURCE=.\Linklist.h
# End Source File
# Begin Source File

SOURCE=.\listener.h
# End Source File
# Begin Source File

SOURCE=.\misc.h
# End Source File
# Begin Source File

SOURCE=.\mover.h
# End Source File
# Begin Source File

SOURCE=.\nature.h
# End Source File
# Begin Source File

SOURCE=.\navigate.h
# End Source File
# Begin Source File

SOURCE=.\object.h
# End Source File
# Begin Source File

SOURCE=..\..\Shared\qcommon\output.h
# End Source File
# Begin Source File

SOURCE=.\path.h
# End Source File
# Begin Source File

SOURCE=.\player.h
# End Source File
# Begin Source File

SOURCE=.\playerheuristics.h
# End Source File
# Begin Source File

SOURCE=.\PlayerStart.h
# End Source File
# Begin Source File

SOURCE=.\portal.h
# End Source File
# Begin Source File

SOURCE=.\powerups.h
# End Source File
# Begin Source File

SOURCE=.\program.h
# End Source File
# Begin Source File

SOURCE=.\puzzleobject.hpp
# End Source File
# Begin Source File

SOURCE=.\q_shared.h
# End Source File
# Begin Source File

SOURCE=..\..\Shared\qcommon\quaternion.h
# End Source File
# Begin Source File

SOURCE=.\queue.h
# End Source File
# Begin Source File

SOURCE=.\RageAI.h
# End Source File
# Begin Source File

SOURCE=.\script.h
# End Source File
# Begin Source File

SOURCE=.\scriptmaster.h
# End Source File
# Begin Source File

SOURCE=.\scriptslave.h
# End Source File
# Begin Source File

SOURCE=.\scriptvariable.h
# End Source File
# Begin Source File

SOURCE=.\sentient.h
# End Source File
# Begin Source File

SOURCE=.\shrapnelbomb.h
# End Source File
# Begin Source File

SOURCE=.\soundman.h
# End Source File
# Begin Source File

SOURCE=.\spawners.h
# End Source File
# Begin Source File

SOURCE=.\specialfx.h
# End Source File
# Begin Source File

SOURCE=.\stack.h
# End Source File
# Begin Source File

SOURCE=.\stationaryvehicle.hpp
# End Source File
# Begin Source File

SOURCE=.\steering.h
# End Source File
# Begin Source File

SOURCE=.\str.h
# End Source File
# Begin Source File

SOURCE=.\surfaceflags.h
# End Source File
# Begin Source File

SOURCE=.\teammateroster.hpp
# End Source File
# Begin Source File

SOURCE=.\trigger.h
# End Source File
# Begin Source File

SOURCE=.\umap.h
# End Source File
# Begin Source File

SOURCE=.\UseData.h
# End Source File
# Begin Source File

SOURCE=.\vector.h
# End Source File
# Begin Source File

SOURCE=.\vehicle.h
# End Source File
# Begin Source File

SOURCE=.\viewthing.h
# End Source File
# Begin Source File

SOURCE=.\waypoints.h
# End Source File
# Begin Source File

SOURCE=.\weapon.h
# End Source File
# Begin Source File

SOURCE=.\WeaponDualWield.h
# End Source File
# Begin Source File

SOURCE=.\weaputils.h
# End Source File
# Begin Source File

SOURCE=.\worldspawn.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
