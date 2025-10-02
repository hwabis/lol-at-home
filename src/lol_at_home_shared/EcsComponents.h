#pragma once

#include <cstdint>
#include <unordered_map>
#include "abilities/AbilityTag.h"
#include "abilities/AbilitySlot.h"

namespace lol_at_home_shared {

struct Position {
  double X{};
  double Y{};
};

struct Health {
  double CurrentHealth{};
  double MaxHealth{};
  double HealthRegenPerSec{};
};

struct Mana {
  double Mana{};
  double MaxMana{};
  double ManaRegenPerSec{};
};

struct Movable {
  double Speed{};
};

struct Moving {
  Position TargetPosition;
};

struct Team {
  enum class Color : uint8_t { Blue, Red };
  Color TeamColor;
};

struct DamagingArea {
  Team OwningTeam;
};

struct Abilities {
  static constexpr int NumAbilities = 6;

  struct Ability {
    AbilityTag Tag{};
    float CooldownRemaining{};
    int Rank{};
    int CurrentCharges = 1;
    uint8_t MaxCharges = 1;
  };

  std::unordered_map<AbilitySlot, Ability> Abilities;
};

}  // namespace lol_at_home_shared
