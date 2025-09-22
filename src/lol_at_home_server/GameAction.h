#pragma once

#include <variant>
#include "EntityStats.h"

namespace lol_at_home_server {

// Externally received actions triggered by a player, e.g. champion movement.
// This does not encompass actions for non-player characters (e.g. tower shots,
// minion auto-attacks).
struct GameAction {
  EntityId Id{};
};

struct MoveAction : GameAction {
  Vector2 EndPosition;
};

struct AbilityGameAction : GameAction {
  enum class Ability : uint8_t { Q, W, E, R };
  Ability Ability{};
  Vector2 StartPosition;
  Vector2 EndPosition;
};

struct AutoAttackGameAction : GameAction {
  EntityId TargetId{};
};

struct StopGameAction : GameAction {};

using GameActionVariant = std::variant<MoveAction,
                                       AbilityGameAction,
                                       AutoAttackGameAction,
                                       StopGameAction>;

}  // namespace lol_at_home_server
