//-----------------------------------------------------------------------------
//
//  $Logfile:: /Code/DLLs/game/entity.h                                      $
// $Revision:: 124                                                            $
//   $Author:: Steven                                                         $
//     $Date:: 10/13/03 9:43a                                                 $
//
// Copyright (C) 2002 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
//
// DESCRIPTION:
// Base class for all entities that are controlled by the game.  If you have any
// object that should be called on a periodic basis and it is not an entity,
// then you have to have an dummy entity that calls it.
//
// An entity in the game is any object that is not part of the world.  Any non-world
// object that is visible in the game is an entity, although it is not required that
// all entities be visible to the player.  Some objects are basically just virtual
// constructs that act as an instigator of certain actions, for example, some
// triggers are invisible and cannot be touched, but when activated by other
// objects can cause things to happen.
//
// All entities are capable of receiving messages from Sin or from other entities.
// Messages received by an entity may be ignored, passed on to their superclass,
// or acted upon by the entity itself.  The programmer must decide on the proper
// action for the entity to take to any message.  There will be many messages
// that are completely irrelevant to an entity and should be ignored.  Some messages
// may require certain states to exist and if they are received by an entity when
// it these states don't exist may indicate a logic error on the part of the
// programmer or map designer and should be reported as warnings (if the problem is
// not severe enough for the game to be halted) or as errors (if the problem should
// not be ignored at any cost).
//

class BindInfo;
class MorphInfo;
class Entity;

#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <cinttypes>

#include "program.h"
#include "vector.h"
#include "scriptvariable.h"
#include "DamageModification.hpp"
#include "UseData.h"

// modification flags
enum EModificationFlags
{
  FLAG_IGNORE,
  FLAG_CLEAR,
  FLAG_ADD,
};

enum EDamageT
{
  DamageNo,
  DamageYes, // will take damage if hit
  DamageAim // auto targeting recognizes this
};

//deadflag
enum EDeadFlag
{
  DeadNo,
  DeadDying,
  DeadDead,
  DeadRespawnable
};

// flags
enum EFlags
{
  FlagFly = 1 << 0,
  FlagSwim = 1 << 1, // implied immunity to drowning
  FlagInwater = 1 << 2,
  FlagGodmode = 1 << 3,
  FlagNotarget = 1 << 4,
  FlagPartialground = 1 << 5, // not all corners are valid
  FlagTeamslave = 1 << 6, // not the first on the team
  FlagNoKnockback = 1 << 7,
  FlagThink = 1 << 8,
  FlagBlood = 1 << 9, // when hit, it should bleed.
  FlagDieGibs = 1 << 10, // when it dies, it should gib
  FlagDieExplode = 1 << 11, // when it dies, it will explode
  FlagRotatedbounds = 1 << 12, // model uses rotated mins and maxs
  FlagDontsave = 1 << 13, // don't add to the save game
  FlagImmobile = 1 << 14, // entity has been immobilized somehow
  FlagPartialImmobile = 1 << 15, // entity has been immobilized somehow
  FlagStunned = 1 << 16,
  FlagPostthink = 1 << 17, // call a think function after the physics have been run
  FlagTouchTriggers = 1 << 18, // should this entity touch triggers
  FlagAutoaim = 1 << 19 // Auto aim on this entity
};

// Create flags

enum ECreateFlags
{
  EntityCreateFlagAnimate = 1 << 0,
  EntityCreateFlagMover = 1 << 1
};


// damage flags
enum EDamageFlags
{
  DamageRadius = 0x00000001, // damage was indirect
  DamageNoArmor = 0x00000002, // armor does not protect from this damage
  DamageEnergy = 0x00000004, // damage is from an energy based weapon
  DamageNoKnockback = 0x00000008, // do not affect velocity, just view angles
  DamageBullet = 0x00000010, // damage is from a bullet (used for ricochets)
  DamageNoProtection = 0x00000020,// armor, shields, invulnerability, and god mode have no effect
  DamageNoSkill = 0x00000040, // damage is not affected by skill level
};

extern Event EV_ClientMove;
extern Event EV_ClientEndFrame;

// Generic entity events
extern Event EV_Classname;
extern Event EV_Activate;
extern Event EV_Use;
extern Event EV_FadeNoRemove;
extern Event EV_FadeOut;
extern Event EV_FadeIn;
extern Event EV_Fade;
extern Event EV_Killed;
extern Event EV_GotKill;
extern Event EV_Pain;
extern Event EV_Damage;
extern Event EV_Stun;
extern Event EV_Gib;
extern Event EV_Kill;
extern Event EV_DeathSinkStart;
extern Event EV_TouchTriggers;
extern Event EV_DoRadiusDamage;

// Physics events
extern Event EV_MoveDone;
extern Event EV_Touch;
extern Event EV_Contact;
extern Event EV_LostContact;
extern Event EV_Blocked;
extern Event EV_Attach;
extern Event EV_AttachModel;
extern Event EV_RemoveAttachedModel;
extern Event EV_RemoveAttachedModelByTargetname;
extern Event EV_Detach;
extern Event EV_UseBoundingBox;

// Animation events
extern Event EV_NewAnim;
extern Event EV_LastFrame;
extern Event EV_TakeDamage;
extern Event EV_NoDamage;
extern Event EV_SetCinematicAnim;

// script stuff
extern Event EV_Model;
extern Event EV_Hide;
extern Event EV_Show;
extern Event EV_BecomeSolid;
extern Event EV_BecomeNonSolid;
extern Event EV_Sound;
extern Event EV_StopSound;
extern Event EV_Bind;
extern Event EV_Unbind;
extern Event EV_JoinTeam;
extern Event EV_QuitTeam;
extern Event EV_SetHealth;
extern Event EV_SetMaxHealth;
extern Event EV_SetSize;
extern Event EV_SetAlpha;
extern Event EV_SetOrigin;
extern Event EV_Warp;
extern Event EV_SetTargetName;
extern Event EV_SetTarget;
extern Event EV_SetKillTarget;
extern Event EV_SetAngles;
extern Event EV_SetAngle;
extern Event EV_RegisterAlias;
extern Event EV_Anim;
extern Event EV_StartAnimating;
extern Event EV_SurfaceModelEvent;
extern Event EV_ProcessInitCommands;
extern Event EV_Stop;
extern Event EV_StopLoopSound;
extern Event EV_SetControllerAngles;
extern Event EV_DisplayEffect;
extern Event EV_ForceAlpha;

extern Event EV_SetFloatVar;
extern Event EV_SetVectorVar;
extern Event EV_SetStringVar;

extern Event EV_GetFloatVar;
extern Event EV_GetVectorVar;
extern Event EV_GetStringVar;

extern Event EV_RemoveVariable;
extern Event EV_DoesVarExist;


extern Event EV_UseDataAnim;
extern Event EV_UseDataType;
extern Event EV_UseDataThread;
extern Event EV_UseDataMaxDist;
extern Event EV_UseData;

extern Event EV_CreateEarthquake;
extern Event EV_SpawnEffect;

// Morph stuff

extern Event EV_Morph;
extern Event EV_Unmorph;

// dir is 1
// power is 2
// min size is 3
// max size is 4
// percentage is 5
// thickness 6
// entity is 7
// origin 8

// AI sound events
extern Event EV_BroadcastSound;
extern Event EV_HeardSound;
extern Event EV_Hurt;
extern Event EV_IfSkill;

extern Event EV_SetArchetype;
extern Event EV_SetGameplayHealth;
extern Event EV_SetGameplayDamage;
extern Event EV_ProcessGameplayData;

extern Event EV_StopStasis;

extern Event EV_AddHealthOverTime;

extern Event EV_SimplePlayDialog;
// Define ScriptMaster
class ScriptMaster;

//
// Spawn args
//
// "spawnflags"
// "alpha" default 1.0
// "model"
// "origin"
// "targetname"
// "target"
//
static const uint8_t MAX_MODEL_CHILDREN = 8;

class Entity;
class Animate;
class Mover;

class BindInfo
{
public:

  // Model Binding variables
  int32_t numchildren;
  int32_t children[MAX_MODEL_CHILDREN];

  // Team variables
  str moveteam;
  Entity* teamchain;
  Entity* teammaster;

  // Binding variables
  Entity* bindmaster;
  qboolean bind_use_my_angles;
  //Vector localorigin;
  //Vector localangles;

  qboolean detach_at_death;

  BindInfo();

  void Archive(Archiver& Arc);
};

inline BindInfo::BindInfo()
{
  // model binding variables
  numchildren = 0;

  for (auto i = 0; i < MAX_MODEL_CHILDREN; i++)
  {
    children[i] = ENTITYNUM_NONE;
  }

  detach_at_death = true;

  // team variables
  teamchain = nullptr;
  teammaster = nullptr;

  // bind variables
  bindmaster = nullptr;

  bind_use_my_angles = false;
}

inline void BindInfo::Archive(Archiver& Arc)
{
  Arc.ArchiveInteger(&numchildren);
  Arc.ArchiveRaw(children, sizeof(children));
  Arc.ArchiveString(&moveteam);
  Arc.ArchiveObjectPointer(reinterpret_cast<Class **>(&teamchain));
  Arc.ArchiveObjectPointer(reinterpret_cast<Class **>(&teammaster));
  Arc.ArchiveObjectPointer(reinterpret_cast<Class **>(&bindmaster));
  Arc.ArchiveBoolean(&bind_use_my_angles);
  Arc.ArchiveBoolean(&detach_at_death);
}

BindInfo* CreateBindInfo();

class MorphInfo
{
public:
  MorphInfo()
  {
    for (auto i = 0; i < NUM_MORPH_CONTROLLERS; i++)
    {
      controllers[i].index = -1;
      controllers[i].current_percent = 0.0;
    }

    controller_on = false;
  }

  struct TMorph
  {
    int32_t index;
    float current_percent;
    float speed;
    float final_percent;
    qboolean return_to_zero;
    int32_t channel;
  };

  TMorph controllers[NUM_MORPH_CONTROLLERS];
  qboolean controller_on;

  void Archive(Archiver& Arc)
  {
    for (auto i = 0; i < NUM_MORPH_CONTROLLERS; i++)
    {
      Arc.ArchiveInteger(&controllers[i].index);
      Arc.ArchiveFloat(&controllers[i].current_percent);
      Arc.ArchiveFloat(&controllers[i].speed);
      Arc.ArchiveFloat(&controllers[i].final_percent);
      Arc.ArchiveBoolean(&controllers[i].return_to_zero);
      Arc.ArchiveInteger(&controllers[i].channel);
    }

    Arc.ArchiveBoolean(&controller_on);
  }
};


MorphInfo* CreateMorphInfo();

typedef SafePtr<Entity> EntityPtr;

class Program;

class Entity : public Listener
{
  Vector _localOrigin;
  Container<EntityPtr> _lastTouchedList;
  bool _fulltrace;
  int32_t _groupID;
  str _archetype;
  bool _missionObjective;
  str _targetPos;

  bool _networkDetail;

protected:
  void BuildUseData();

public:
  CLASS_PROTOTYPE(Entity);

  // Construction / destruction
  Entity();
  explicit Entity(int32_t create_flag);
  virtual ~Entity();

  // Spawning variables
  int32_t entnum;
  gentity_t* edict;
  gclient_t* client;
  int32_t spawnflags;

  // Standard variables
  str model;

  // Physics variables
  Vector total_delta; // total unprocessed movement
  Vector mins;
  Vector maxs;
  Vector absmin;
  Vector absmax;
  Vector centroid;
  Vector velocity;
  Vector avelocity;
  Vector origin;
  Vector angles;
  Vector size;
  int32_t movetype;
  int32_t mass;
  float gravity; // per entity gravity multiplier (1.0 is normal)
  float orientation[3][3];
  Vector localangles;

  // Ground variables
  gentity_t* groundentity;
  cplane_t groundplane;
  int32_t groundcontents;

  // Surface variables
  int32_t numsurfaces;

  // Light variables
  float lightRadius;

  // Targeting variables
  str target;
  str targetname;
  str killtarget;

  // Character state
  float health;
  float max_health;
  int32_t deadflag;
  int32_t flags;

  // underwater variables
  int32_t watertype;
  int32_t waterlevel;

  // Pain and damage variables
  EDamageT takedamage;
  int32_t damage_type;

  qboolean look_at_me;
  bool projectilesCanStickToMe;

  str explosionModel;

  ScriptVariableList entityVars;

  uint32_t _affectingViewModes;
  Vector watch_offset;

  // Pluggable modules

  Animate* animate;
  Mover* mover;
  BindInfo* bind_info;
  MorphInfo* morph_info;
  Program* ObjectProgram;
  DamageModificationSystem* damageModSystem;
  UseData* useData;

  void Setup();

  static Entity* FindEntityByName(const str& EntityName);
  void SetEntNum(int32_t Num);
  void ClassnameEvent(Event* ev);
  void SpawnFlagsEvent(Event* ev);

  Vector InterceptTarget(const Vector& targetPosition, const Vector& targetVelocity, float maxSpeed) const
  {
    auto myPosition(origin);
    auto predictedPosition = targetPosition + targetVelocity * (Vector::Distance(targetPosition, myPosition) / maxSpeed);
    auto desiredDirection = predictedPosition - myPosition;

    return desiredDirection.toAngles();
  }

  Vector InterceptTargetXY(const Vector& targetPosition, const Vector& targetVelocity, float maxSpeed) const
  {
    auto myPosition(origin);
    auto distanceToTargetPosition = Vector::Distance(targetPosition, myPosition);
    if (!fSmallEnough(distanceToTargetPosition, 0.1f))
    {
      auto predictedPosition = targetPosition + targetVelocity * (distanceToTargetPosition / maxSpeed);
      auto desiredDirection = predictedPosition - myPosition;
      desiredDirection.z = 0.0f;
      return desiredDirection.toAngles();
    }
    return angles;
  }

  float DistanceTo(const Vector& pos)
  {
    Vector delta;

    delta = origin - pos;
    return delta.length();
  }

  float DistanceTo(const Entity* ent)
  {
    Vector delta;

    assert(ent);

    if (!ent)
      return 999999.0f; // "Infinite" distance

    delta = origin - ent->origin;
    return delta.length();
  }

  qboolean WithinDistance(const Vector& pos, float dist)
  {
    auto delta(origin - pos);

    // check squared distance
    return delta * delta < dist * dist;
  }

  qboolean WithinDistance(const Entity* ent, float dist)
  {
    if (!ent)
    {
      return false;
    }

    auto delta(origin - ent->origin);

    // check squared distance
    return delta * delta < dist * dist;
  }

  bool WithinDistanceXY(const Vector& pos, float dist)
  {
    auto distance = Vector::DistanceXY(origin, pos);
    return distance <= dist;
  }

  bool WithinDistanceXY(const Entity* ent, float dist)
  {
    if (!ent)
      return false;

    auto distance = Vector::DistanceXY(origin, ent->origin);
    return distance <= dist;
  }

  const char* Target(void)
  {
    return target.c_str();
  }

  void SetTarget(const char* target);
  qboolean Targeted(void)
  {
    return targetname.length() == 0 ? false : true;
  }

  const char* TargetName(void)
  {
    return targetname.c_str();
  }

  void SetTargetName(const char* target);
  void SetKillTarget(const char* killtarget);
  const char* KillTarget(void)
  {
    return killtarget.c_str();
  }

  const Vector& GetLocalOrigin(void) const
  {
    return _localOrigin;
  }

  void SetLocalOrigin(const Vector& localOrigin)
  {
    _localOrigin = localOrigin;
  }

  virtual void setModel(const char* model);
  void setModel(const str& mdl)
  {
    setModel(mdl.c_str());
  }

  virtual void setViewModel(const char* model);
  void setViewModel(const str& mdl)
  {
    setViewModel(mdl.c_str());
  }

  virtual void SetModelEvent(Event* ev)
  {
    char modelname[256];
    strcpy(modelname, ev->GetString(1));
    auto tmpPtr = strstr(modelname, "*");
    if (tmpPtr)
    {
      ev->SetString(1, tmpPtr);
    }

    setModel(ev->GetString(1));
  }

  void SetTeamEvent(Event* ev);
  virtual void TriggerEvent(Event* ev);
  void hideModel(void)
  {
    edict->s.renderfx |= RF_DONTDRAW;
    if (getSolidType() <= SOLID_TRIGGER)
      edict->svflags |= SVF_NOCLIENT;
  }

  void EventHideModel(Event* ev);
  virtual void showModel(void)
  {
    edict->s.renderfx &= ~RF_DONTDRAW;
    edict->svflags &= ~SVF_NOCLIENT;
  }

  void EventShowModel(Event* ev);
  qboolean hidden(void)
  {
    return edict->s.renderfx & RF_DONTDRAW ? true : false;
  }

  void ProcessInitCommandsEvent(Event* ev);
  void ProcessInitCommands(int32_t index, qboolean cache = false);

  void setAlpha(float alpha);
  float alpha(void)
  {
    return edict->s.alpha;
  }

  void setMoveType(int32_t type)
  {
    movetype = type;
  }

  void setMoveType(Event* ev);

  int32_t getMoveType(void)
  {
    return movetype;
  }

  void setSolidType(solid_t type);
  int32_t getSolidType(void)
  {
    return edict->solid;
  }

  virtual Vector getParentVector(const Vector& vec);
  Vector getLocalVector(const Vector& vec);

  virtual void setSize(Vector min, Vector max);
  virtual void setOrigin(const Vector& org);
  virtual void setOrigin(void);
  virtual void addOrigin(const Vector& org);
  virtual void setOriginEveryFrame(Event* ev);

  void GetRawTag(int32_t tagnum, orientation_t* orient, bodypart_t part = legs);
  qboolean GetRawTag(const char* tagname, orientation_t* orient, bodypart_t part = legs);

  void GetTag(int32_t tagnum, orientation_t* orient);
  qboolean GetTag(const char* name, orientation_t* orient);
  void GetTag(int32_t tagnum, Vector* pos, Vector* forward = nullptr, Vector* left = nullptr, Vector* up = nullptr);
  qboolean GetTag(const char* name, Vector* pos, Vector* forward = nullptr, Vector* left = nullptr, Vector* up = nullptr);

  virtual int32_t CurrentFrame(bodypart_t part = legs);
  virtual int32_t CurrentAnim(bodypart_t part = legs);

  virtual void setAngles(const Vector& ang);
  virtual void setAngles(void);
  virtual void SetOrigin(Event* ev);
  void GetOrigin(Event* ev);

  Vector GetControllerAngles(int32_t num);
  void SetControllerAngles(int32_t num, vec3_t angles);
  void SetControllerAngles(Event* ev);
  void SetControllerTag(int32_t num, int32_t tag_num);

  void link(void);
  void unlink(void)
  {
    gi.unlinkentity(edict);
  }


  void setContents(int32_t type)
  {
    edict->contents = type;
  }

  int32_t getContents(void)
  {
    return edict->contents;
  }

  void setScale(float scale);

  qboolean droptofloor(float maxfall);
  qboolean isClient(void)
  {
    return client != nullptr ? true : false;
  }

  virtual void SetDeltaAngles(void)
  {
    if (!client)
      return;

    for (auto i = 0; i < 3; i++)
      client->ps.delta_angles[i] = ANGLE2SHORT(client->ps.viewangles[i]);
  }

  virtual void DamageEvent(Event* event);

  void Damage(Entity* inflictor, Entity* attacker, float damage, const Vector& position, const Vector& direction,
              const Vector& normal, int32_t knockback, int32_t flags, int32_t meansofdeath, int32_t surface_number = -1,
              int32_t bone_number = -1, Entity* weapon = nullptr);

  void Stun(float time);

  void DamageType(Event* ev);
  virtual qboolean CanDamage(const Entity* target, const Entity* skip_ent = nullptr);

  qboolean IsTouching(const Entity* e1);

  void FadeNoRemove(Event* ev);
  void FadeOut(Event* ev);
  void FadeIn(Event* ev);
  void Fade(Event* ev);

  virtual void CheckGround(void);
  virtual qboolean HitSky(const trace_t* trace);
  virtual qboolean HitSky(void);

  void BecomeSolid(Event* ev);
  void BecomeNonSolid(Event* ev);
  virtual void SetHealth(Event* ev);
  void GetHealth(Event* ev);
  virtual void SetMaxHealth(Event* ev);
  void SetSize(Event* ev);
  virtual void SetMins(Event* ev);
  virtual void SetMaxs(Event* ev);
  void GetMins(Event* ev);
  void GetMaxs(Event* ev);
  void SetScale(Event* ev);
  void setRandomScale(Event* ev);
  void SetAlpha(Event* ev);
  void SetTargetName(Event* ev);
  void GetTargetName(Event* ev);
  void GetRawTargetName(Event* ev);
  virtual void SetTarget(Event* ev);
  void getTarget(Event* ev);
  void GetTargetEntity(Event* ev);
  void SetKillTarget(Event* ev);
  void GetModelName(Event* ev);
  virtual void SetAngles(Event* ev);
  void GetAngles(Event* ev);
  virtual void SetAngleEvent(Event* ev);
  void TouchTriggersEvent(Event* ev);
  void IncreaseShotCount(Event* ev);
  void GetVelocity(Event* ev);
  void SetVelocity(Event* ev);

  // Support for checking/setting fulltrace flag
  void SetFullTraceEvent(Event* ev);

  void setFullTrace(bool fulltrace)
  {
    _fulltrace = fulltrace;
  }

  bool usesFullTrace()
  {
    return _fulltrace;
  }

  Vector GetClosestCorner(const Vector& position);

  str GetRandomAlias(const str& name)
  {
    str realname;
    const char* s;

    s = gi.Alias_FindRandom(edict->s.modelindex, name.c_str());
    if (s != nullptr)
    {
      realname = s;
    }
    else
    {
      s = gi.GlobalAlias_FindRandom(name.c_str());
      if (s != nullptr)
      {
        realname = s;
      }
    }

    return realname;
  }

  void SetWaterType(void);

  // model binding functions
  qboolean attach(int32_t parent_entity_num, int32_t tag_num, qboolean use_angles = true, Vector attach_offset = Vector(0, 0, 0), Vector attach_angles_offset = Vector(0, 0, 0));
  void detach(void);

  void RegisterAlias(Event* ev);
  void RegisterAliasAndCache(Event* ev);
  void Cache(Event* ev);

  qboolean GlobalAliasExists(const char* name)
  {
    assert(name);
    return gi.GlobalAlias_FindRandom(name) != nullptr;
  }

  qboolean AliasExists(const char* name)
  {
    assert(name);
    return gi.Alias_FindRandom(edict->s.modelindex, name) != nullptr;
  }

  // Sound Stuff
  void Sound(Event* ev);
  virtual void Sound(const str& sound_name, int32_t channel = CHAN_BODY, float volume = -1.0f, float min_dist = -1.0f, Vector* origin = nullptr, float pitch_modifier = 1.0f, qboolean onlySendToThisEntity = false);
  void StopSound(int32_t channel);
  void StopSound(Event* ev);
  void LoopSound(Event* ev);
  void LoopSound(const str& sound_name, float volume = -1.0f, float min_dist = -1.0f);
  void StopLoopSound(Event* ev);
  void StopLoopSound(void);

  // Light Stuff
  void SetLight(Event* ev);
  void LightOn(Event* ev);
  void LightOff(Event* ev);
  void LightRed(Event* ev);
  void LightGreen(Event* ev);
  void LightBlue(Event* ev);
  void LightRadius(Event* ev);
  void LightStyle(Event* ev);
  void Flags(Event* ev);
  void Effects(Event* ev);
  void RenderEffects(Event* ev);
  void SVFlags(Event* ev);

  void BroadcastSound(float pos = SOUND_RADIUS, int32_t soundType = SOUNDTYPE_GENERAL);
  void BroadcastSound(Event* ev);
  float ModifyFootstepSoundRadius(float radius, int32_t soundTypeIdx);
  virtual void Kill(Event* ev);
  void SurfaceModelEvent(Event* ev);
  void SurfaceCommand(const char* surf_name, const char* token);

  virtual void Postthink(void);
  virtual void Think(void);
  void DamageSkin(trace_t* trace, float damage);

  void AttachEvent(Event* ev);
  void AttachModelEvent(Event* ev);
  void RemoveAttachedModelEvent(Event* ev);
  void removeAttachedModelByTargetname(Event* ev);
  void removeAttachedModelByTargetname(const str& targetNameToRemove);
  void DetachEvent(Event* ev);
  void TakeDamageEvent(Event* ev);
  void NoDamageEvent(Event* ev);
  void Gravity(Event* ev);
  //void              GiveOxygen( float time );
  void UseBoundingBoxEvent(Event* ev);
  void HurtEvent(Event* ev);
  void IfSkillEvent(Event* ev);
  void SetMassEvent(Event* ev);
  void Censor(Event* ev);
  void Ghost(Event* ev);

  void StationaryEvent(Event* ev);
  void Explosion(Event* ev);
  void SelfDetonate(Event* ev);
  void DoRadiusDamage(Event* ev);

  void Shader(Event* ev);

  void KillAttach(Event* ev);
  //void					SetBloodModel( Event *ev );

  void DropToFloorEvent(Event* ev);
  void SetAnimOnAttachedModel(Event* ev);
  void SetAnimOnAttachedModel(const str& AnimName, const str& TagName);
  void SetEntityExplosionModel(Event* ev);

  virtual void SetCinematicAnim(const str& AnimName);
  virtual void SetCinematicAnim(Event* ev);
  virtual void CinematicAnimDone(void);
  virtual void CinematicAnimDone(Event* ev);

  // Binding methods
  void joinTeam(Entity* teammember);
  void quitTeam(void);
  qboolean isBoundTo(const Entity* master);
  virtual void bind(Entity* master, qboolean use_my_angles = false);
  virtual void unbind(void);

  void JoinTeam(Event* ev);
  void EventQuitTeam(Event* ev);
  virtual void BindEvent(Event* ev);
  virtual void EventUnbind(Event* ev);
  void AddToSoundManager(Event* ev);
  void NoLerpThisFrame(void);

  virtual void addAngles(const Vector& add);

  void DeathSinkStart(Event* ev);
  void DeathSink(Event* ev);

  void LookAtMe(Event* ev);
  void ProjectilesCanStickToMe(Event* ev);
  void DetachAllChildren(Event* ev);

  void MorphEvent(Event* ev);
  void UnmorphEvent(Event* ev);
  void MorphControl(Event* ev);
  int32_t GetMorphChannel(const char* morph_name);
  void StartMorphController(void);
  qboolean MorphChannelMatches(int32_t morph_channel1, int32_t morph_channel2);

  void ProjectileAtk(Event* ev);
  void ProjectileAttackPoint(Event* ev);
  void ProjectileAttackEntity(Event* ev);
  void ProjectileAttackFromTag(Event* ev);
  void ProjectileAttackFromPoint(Event* ev);
  void TraceAtk(Event* ev);

  virtual void VelocityModified(void);
  virtual void Archive(Archiver& arc) override;

  virtual void PassToAnimate(Event* ev);
  void SetObjectProgram(Event* ev);
  virtual void SetWatchOffset(Event* ev);
  void ExecuteProgram(Event* ev);

  void Contents(Event* ev);
  void setMask(Event* ev);

  void hideFeaturesForFade(void);
  void showFeaturesForFade(void);

  void DisplayEffect(Event* ev);
  void clearDisplayEffects(void);

  void getCustomShaderInfo(const str& customShader, str& shaderName, str& soundName);

  void setCustomShader(const char* customShader);
  void setCustomShader(Event* ev);
  void clearCustomShader(const char* customShader = nullptr);
  void clearCustomShader(Event* ev);
  bool hasCustomShader(const char* customShader = nullptr);

  void setCustomEmitter(const char* customEmitter);
  void setCustomEmitter(Event* ev);
  void clearCustomEmitter(const char* customEmitter = nullptr);
  void clearCustomEmitter(Event* ev);

  Entity* SpawnEffect(const str& name, const Vector& origin, const Vector& angles, float removeTime);
  Entity* SpawnSound(const str& sound, const Vector& pos, float volume, float removeTime);

  void SpawnEffect(Event* ev);

  void attachEffect(const str& modelName, const str& tagName, float removeTime);
  void attachEffect(Event* ev);

  void ForceAlpha(Event* ev);
  void CreateEarthquake(Event* ev);

  void SetFloatVar(Event* ev);
  void SetVectorVar(Event* ev);
  void SetStringVar(Event* ev);
  void GetFloatVar(Event* ev);
  void doesVarExist(Event* ev);
  void RemoveVariable(Event* ev);
  void GetVectorVar(Event* ev);
  void GetStringVar(Event* ev);
  void SetUserVar1(Event* ev);
  void SetUserVar2(Event* ev);
  void SetUserVar3(Event* ev);
  void SetUserVar4(Event* ev);
  void isWithinDistanceOf(Event* ev);

  void affectingViewMode(Event* ev);
  void addAffectingViewModes(uint32_t mask);
  void removeAffectingViewModes(uint32_t mask);

  void TikiNote(Event* ev);
  void TikiTodo(Event* ev);

  void AddDamageModifier(Event* ev);
  void ResolveDamage(::Damage& damage);

  // Health interface
  float getHealth(void)
  {
    return health;
  };

  float getMaxHealth(void)
  {
    return max_health;
  };

  void setHealth(float newHealth)
  {
    health = newHealth;
  };

  void setMaxHealth(float newMaxHealth)
  {
    max_health = newMaxHealth;
  };

  void addHealth(float healthToAdd, float maxHealth = 0.0f);
  void addHealthOverTime(Event* ev);

  // Group Number Interface
  void SetGroupID(Event* ev);
  void AddToGroup(int32_t ID);

  void SetGroupID(int32_t ID)
  {
    _groupID = ID;
  }

  int32_t GetGroupID()
  {
    return _groupID;
  };

  void MultiplayerEvent(Event* ev);

  Container<EntityPtr>& GetLastTouchedList()
  {
    return _lastTouchedList;
  }

  // Archetype name
  virtual void setArchetype(Event* ev);
  str getArchetype() const;

  virtual str getName() const
  {
    return "";
  }

  void setMissionObjective(Event* ev);

  // Usable Entity functions
  void useDataAnim(Event* ev);
  void useDataType(Event* ev);
  void useDataThread(Event* ev);
  void useDataMaxDist(Event* ev);
  void useDataCount(Event* ev);
  void useDataEvent(Event* ev);
  bool hasUseData()
  {
    return useData != nullptr ? true : false;
  }


  // GameplayManager interfaces to health and damage
  void setGameplayHealth(Event* ev);
  void setGameplayDamage(Event* ev);

  virtual void processGameplayData(Event*)
  {
  }

  // Think interface

  void turnThinkOn(void)
  {
    flags |= FlagThink;
  }

  void turnThinkOff(void)
  {
    flags &= ~FlagThink;
  }

  bool isThinkOn(void)
  {
    return flags & FlagThink ? true : false;
  }

  void startStasis(void);
  void stopStasis(void);
  void startStasis(Event* ev);
  void stopStasis(Event* ev);
  void setTargetPos(Event* ev);
  void setTargetPos(const str& targetPos);
  str getTargetPos();

  void simplePlayDialog(Event* ev);

  virtual void warp(Event* ev);

  void traceHitsEntity(Event* ev);

  void setNetworkDetail(Event* ev);
  bool isNetworkDetail(void);
};


#include "worldspawn.h"

#endif
