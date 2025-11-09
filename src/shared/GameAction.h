#pragma once

#include <entt/entt.hpp>
#include <variant>
#include "EcsComponents.h"
#include "abilities/AbilitySlot.h"
#include "abilities/AbilityTarget.h"

namespace lol_at_home_shared {

struct MoveAction {
  entt::entity source{};
  lol_at_home_shared::Position targetPosition;
};

struct AbilityAction {
  entt::entity source{};
  lol_at_home_shared::AbilitySlot slot{};
  AbilityTargetVariant target;
};

struct AutoAttackAction {
  entt::entity source{};
  entt::entity target{};
};

struct StopGameAction {
  entt::entity source{};
};

using GameActionVariant =
    std::variant<MoveAction, AbilityAction, AutoAttackAction, StopGameAction>;

}  // namespace lol_at_home_shared
