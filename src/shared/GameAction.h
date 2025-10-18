#pragma once

#include <entt/entt.hpp>
#include <variant>
#include "EcsComponents.h"
#include "abilities/AbilitySlot.h"
#include "abilities/AbilityTarget.h"

namespace lol_at_home_shared {

struct MoveAction {
  entt::entity Source{};
  lol_at_home_shared::Position TargetPosition;
};

struct AbilityAction {
  entt::entity Source{};
  lol_at_home_shared::AbilitySlot Slot{};
  AbilityTargetVariant Target;
};

struct AutoAttackAction {
  entt::entity Source{};
  entt::entity Target{};
};

struct StopGameAction {
  entt::entity Source{};
};

using GameActionVariant =
    std::variant<MoveAction, AbilityAction, AutoAttackAction, StopGameAction>;

}  // namespace lol_at_home_shared
