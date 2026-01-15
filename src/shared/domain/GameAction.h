#pragma once

#include <entt/entt.hpp>
#include <variant>
#include "abilities/AbilitySlot.h"
#include "abilities/AbilityTarget.h"

namespace lah::shared {

struct MoveAction {
  entt::entity source{};
  float targetX{};
  float targetY{};
};

struct AbilityAction {
  entt::entity source{};
  lah::shared::AbilitySlot slot{};
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

}  // namespace lah::shared
