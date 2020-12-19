#pragma once

#include "main.h"

#include <sys/types.h>
#include <vector>

#define UnitDataOffset 0x8
#define UnitParamOffset 0x74
#define StatusParamOffset 0x8

enum class eParamType : int
{
  NONE = 0,
  HP = 1,
  ATK = 2,
  DEF = 3,
  MAGIC_ATK = 4,
  MAGIC_DEF = 5,
  PHYSICAL_CRITICAL = 6,
  MAGIC_CRITICAL = 7,
  DODGE = 8,
  LIFE_STEAL = 9,
  WAVE_HP_RECOVERY = 10,
  WAVE_ENERGY_RECOVERY = 11,
  PHYSICAL_PENETRATE = 12,
  MAGIC_PENETRATE = 13,
  ENERGY_RECOVERY_RATE = 14,
  HP_RECOVERY_RATE = 15,
  ENERGY_REDUCE_RATE = 16,
  ACCURACY = 17
};

struct StatusParam
{
};

struct UnitParam
{
  char pad[StatusParamOffset];
  StatusParam *baseParam;
};

struct UnitData
{
  char pad[UnitParamOffset];
  UnitParam *unitParam;
};

struct UnitParameter
{
  char pad[UnitDataOffset];
  UnitData *unitData;
};

int (*GetParam)(StatusParam *, eParamType);
int GetParamHook(StatusParam *self, eParamType paramType)
{
  return GetParam(self, paramType);
}

void (*SetAtk)(StatusParam *, int);
void SetAtkHook(StatusParam *self, int v)
{
  SetAtk(self, v);
}

void (*SetMagicAtk)(StatusParam *, int);
void SetMagicAtkHook(StatusParam *self, int v)
{
  SetMagicAtk(self, v);
}

void (*SetDef)(StatusParam *, int);
void SetDefHook(StatusParam *self, int v)
{
  SetDef(self, v);
}

void (*SetMagicDef)(StatusParam *, int);
void SetMagicDefHook(StatusParam *self, int v)
{
  SetMagicDef(self, v);
}

typedef struct
{
  DWORD unitParameter;
  int atk, mAtk, def, mDef;
} Unit;

std::vector<Unit> *Units = new std::vector<Unit>();

void (*Initialize)(void *, void *, bool, bool, bool);
void InitializeHook(void *self, UnitParameter *unitParameter, bool isEnemy, bool isFirstWave, bool isGaugeAlwaysVisible)
{
  StatusParam *baseParam = unitParameter->unitData->unitParam->baseParam;
  int atk = GetParam(baseParam, eParamType::ATK),
      mAtk = GetParam(baseParam, eParamType::MAGIC_ATK),
      def = GetParam(baseParam, eParamType::DEF),
      mDef = GetParam(baseParam, eParamType::MAGIC_DEF);

  bool isModified = false;
  for (int i = 0; i < Units->size(); ++i)
  {
    Unit unit = (*Units)[i];
    if (unit.unitParameter == (DWORD)unitParameter)
    {
      isModified = unit.atk == atk && unit.mAtk == mAtk && unit.def == def && unit.mDef == mDef;
      if (!isModified)
      {
        Units->erase(Units->begin() + i);
      }
      break;
    }
  }

  if (!isModified)
  {
    if (isEnemy)
    {
      if (conf.atkDivisor > 1 && atk > 10)
        atk = atk / conf.atkDivisor;
      if (conf.mAtkDivisor > 1 && mAtk > 10)
        mAtk = mAtk / conf.mAtkDivisor;
      if (conf.defDivisor > 1 && def > 10)
        def = def / conf.defDivisor;
      if (conf.mDefDivisor > 1 && mDef > 10)
        mDef = mDef / conf.mDefDivisor;
    }
    else
    {
      if (conf.atkMultiplier > 1 && atk > 0)
        atk = atk * conf.atkMultiplier;
      if (conf.mAtkMultiplier > 1 && mAtk > 0)
        mAtk = mAtk * conf.mAtkMultiplier;
      if (conf.defMultiplier > 1 && def > 0)
        def = def * conf.defMultiplier;
      if (conf.mDefMultiplier > 1 && mDef > 0)
        mDef = mDef * conf.mDefMultiplier;
    }

    Units->push_back({(DWORD)unitParameter, atk, mAtk, def, mDef});

    SetAtk(baseParam, atk);
    SetMagicAtk(baseParam, mAtk);
    SetDef(baseParam, def);
    SetMagicDef(baseParam, mDef);
  }

  Initialize(self, unitParameter, isEnemy, isFirstWave, isGaugeAlwaysVisible);
}