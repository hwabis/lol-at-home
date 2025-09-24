#pragma once

#include <entt/entt.hpp>
#include <variant>
#include "EcsComponents.h"

namespace lol_at_home_server {

// Actions triggered by any entity, e.g. a player moving their champion, a tower
// shot, minion auto-attack.
struct GameAction {
  entt::entity Source{};
};

struct MoveAction : GameAction {
  Position TargetPosition;
};

struct AbilityGameAction : GameAction {
  enum class Ability : uint8_t { Q, W, E, R, Summ1, Summ2 };
  Ability Ability{};
  Position StartPosition;
  Position EndPosition;
};

struct AutoAttackGameAction : GameAction {
  entt::entity Target{};
};

struct StopGameAction : GameAction {};

using GameActionVariant = std::variant<MoveAction,
                                       AbilityGameAction,
                                       AutoAttackGameAction,
                                       StopGameAction>;

}  // namespace lol_at_home_server
