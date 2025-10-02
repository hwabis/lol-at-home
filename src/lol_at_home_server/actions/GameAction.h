#pragma once

#include <entt/entt.hpp>
#include <variant>
#include "EcsComponents.h"
#include "abilities/AbilitySlot.h"
#include "abilities/AbilityTarget.h"

namespace lol_at_home_server {

// Actions triggered by any entity, e.g. a player moving their champion, a tower
// shot, minion auto-attack.
struct GameAction {
  entt::entity Source{};
};

struct MoveAction : GameAction {
  lol_at_home_shared::Position TargetPosition;
};

struct AbilityAction : GameAction {
  lol_at_home_shared::AbilitySlot Slot{};
  AbilityTargetVariant Target;
};

struct AutoAttackAction : GameAction {
  entt::entity Target{};
};

struct StopGameAction : GameAction {};

using GameActionVariant =
    std::variant<MoveAction, AbilityAction, AutoAttackAction, StopGameAction>;

}  // namespace lol_at_home_server
