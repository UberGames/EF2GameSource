


This is the game source for Star Trek Elite Force II.  The package contains all of the 
server side game code for singleplayer and multi-player and it includes all of the 
changes up to and including the 1.10 patch.

How To Use
----------
When compiled it will create the file gamex86.dll which will be placed in the
.\executable\Debug directory or the .\executable\release directory depending on 
if you are creating a debug or a release build of the dll.

You can either replace the old gamex86.dll from the full game (although you
should back it up) or you can change which game dll the executable uses
by specifing which one to use at the command line.

ex.
EF2 +set gamedll c:\projects\EF2\executable\debug\gamex86.dll

NOTE: the code has only been tested under Visual Studio 6.0.


Quick layout of the code
------------------------

multiplayer code:

mp_manager      - manages all of the multiplayer code

mp_modeBase     - the base class for all of the modes (dm, team, ctf, etc.)
mp_modeTeamBase - code that is shared between for all team related modes

mp_modeDM       - all deathmatch related code
mp_modeTeamDM   - all team deathmatch specific code
mp_modeCtf      - all CTF related code

mp_modifiers    - all of the code for the various modifiers (action hero, disintegration, etc.)


main class hierarchy:

class - base class, allows safe pointers
  listener - adds event handling
    entity - basic entity class
      sentient - anything that thinks on its own
        actor - AI creatures
        player - the player(s) of course
      trigger - a host of different kind of entities that are turned on in some manner (touch, damaged, etc) to do a specific task
      item - your basic item
        weapon - weapons that can be used by sentients (players or actors)
        powerups - special items that can be picked up, are automatically used, and usually last a specific amount of time

This hierarchy only shows a small portion of the code base but it is a fairly good start at understanding
it.
  

Restrictions
------------
This code and the dll that it will build will only work with a full copy of Star Trek Elite Force II.

All code and contents included in this package are
Copyright (C) 1998 by Ritual Entertainment, Inc.
All rights reserved.

This entire package is supplied as is.  Ritual and Activision will not provide any support.

See the file licenseAgreement.txt in this package for the full license agreement.
