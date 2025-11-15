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
  // units per sec
  double speed{};
};

struct Moving {
  Position targetPosition;
};

struct Team {
  enum class Color : uint8_t { Blue, Red };
  Color teamColor;
};

struct DamagingArea {
  Team owningTeam;
};

struct Abilities {
  struct Ability {
    AbilityTag tag{};
    float cooldownRemaining{};
    int rank{};
    int currentCharges = 1;
    uint8_t maxCharges = 1;
  };

  std::unordered_map<AbilitySlot, Ability> abilities;
};

}  // namespace lol_at_home_shared
