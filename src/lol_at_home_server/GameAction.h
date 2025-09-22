#pragma once

#include <variant>
#include "EntityStats.h"

namespace lol_at_home_server {

// Actions triggered by any entity, e.g. a player moving their champion, a tower
// shot, minion auto-attack.
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
