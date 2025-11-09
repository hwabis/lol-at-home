#pragma once

#include <cstdint>
#include <unordered_map>
#include "abilities/AbilitySlot.h"
#include "abilities/AbilityTag.h"

namespace lol_at_home_shared {

struct Position {
  double x{};
  double y{};
};

struct Health {
  double currentHealth{};
  double maxHealth{};
  double healthRegenPerSec{};
};

struct Mana {
  double mana{};
  double maxMana{};
  double manaRegenPerSec{};
};

struct Movable {
  double speed{};
};

struct Moving {
  Position targetPosition;
};

struct Team {
  enum class Color : uint8_t { Blue, Red };
  Color teamColorFB;
};

struct DamagingArea {
  Team owningTeam;
};

struct Abilities {
  static constexpr int numAbilities = 6;

  struct Ability {
    AbilityTag Tag{};
    float CooldownRemaining{};
    int Rank{};
    int CurrentCharges = 1;
    uint8_t MaxCharges = 1;
  };

  std::unordered_map<AbilitySlot, Ability> abilities;
};

}  // namespace lol_at_home_shared
