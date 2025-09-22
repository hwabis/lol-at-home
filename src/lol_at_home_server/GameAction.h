#pragma once

#include <cstdint>
#include "EntityStats.h"

namespace lol_at_home_server {

// Actions for any unit (e.g. champions using abilties, minions attacking).
// Not every action type will u se all the fields, e.g. Ability is only used if
// action Type is Ability
struct GameAction {
  enum class Type : uint8_t { Move, Ability, AutoAttack, Stop };
  enum class Ability : uint8_t { Q, W, E, R };

  EntityId Id;
  Type Type;

  Ability Ability;
  Vector2 StartPosition;
  Vector2 EndPosition;
  EntityId TargetId;
};

}  // namespace lol_at_home_server
