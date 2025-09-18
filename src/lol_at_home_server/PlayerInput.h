#pragma once

#include <cstdint>
#include "Entity.h"
#include "Vector2.h"

namespace lol_at_home_server {

struct PlayerInput {
  enum class Type : uint8_t {
    MOVE,
    ABILITY,
    AUTO_ATTACK,
    STOP,
  };

  enum class Ability : uint8_t {
    Q,
    W,
    E,
    R,
  };

  EntityId PlayerChampionId;
  Type Type;

  Ability Ability;
  Vector2 StartPos;
  Vector2 EndPos;
  EntityId TargetUnit;
};

}  // namespace lol_at_home_server
