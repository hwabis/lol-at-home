#pragma once

#include <cstdint>
#include <entt/entt.hpp>
#include <unordered_map>
#include "ChampionId.h"
#include "abilities/AbilitySlot.h"
#include "abilities/AbilityTag.h"

namespace lah::shared {

struct Position {
  float x{};
  float y{};
};

struct Radius {
  float radius{};
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

struct MovementStats {
  float speed{};  // units per sec
};

struct CharacterState {
  enum class State : uint8_t {
    Idle = 0,
    Moving = 1,
    AutoAttackMoving = 2,
    AutoAttackWindup = 3,
  };
  State state = State::Idle;
};

// transient component - only attached when moving
struct MoveTarget {
  float targetX{};
  float targetY{};
};

struct Team {
  // todo add a neutral team
  enum class Color : uint8_t { Blue, Red };
  Color color;
};

struct AutoAttackStats {
  float range;
  float damage;
  float windupDuration;
  float attackSpeed;
};

struct ChampionType {
  ChampionId id;
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
