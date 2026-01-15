#pragma once

#include <cstdint>
#include <unordered_map>
#include "abilities/AbilitySlot.h"
#include "abilities/AbilityTag.h"

namespace lah::shared {

struct Position {
  float x{};
  float y{};
  float championRadius = 50.0F;
};

struct Health {
  float current{};
  float max{};
  float regenPerSec{};
};

struct Mana {
  float mana{};
  float maxMana{};
  float manaRegenPerSec{};
};

enum class MovementState : uint8_t {
  Idle = 0,
  Moving = 1,
};

struct Movable {
  float speed{};  // units per sec
  MovementState state = MovementState::Idle;
  float targetX{};  // ignored if state == idle
  float targetY{};  // ignored if state == idle
};

struct Team {
  // todo add a neutral team
  enum class Color : uint8_t { Blue, Red };
  Color color;
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

}  // namespace lah::shared
