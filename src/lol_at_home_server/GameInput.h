#pragma once

#include <cstdint>

namespace lol_at_home_server {

using EntityId = uint32_t;
using PlayerId = uint32_t;

struct Vector2 {
  float X;
  float Y;
};

struct GameInput {
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

  PlayerId PlayerId;
  Type Type;

  Ability Ability;
  Vector2 StartPos;
  Vector2 EndPos;
  EntityId TargetUnit;
};

}  // namespace lol_at_home_server
