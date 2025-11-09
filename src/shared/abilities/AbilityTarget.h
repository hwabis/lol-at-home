#pragma once

#include <entt/entt.hpp>
#include <variant>
#include "domain/EcsComponents.h"

namespace lol_at_home_shared {

struct NoTarget {};

struct EntityTarget {
  entt::entity Target;
};

struct OnePointSkillshot {
  lol_at_home_shared::Position Target;
};

// Rumble R, Viktor E
struct TwoPointSkillshot {
  lol_at_home_shared::Position Target1;
  lol_at_home_shared::Position Target2;
};

using AbilityTargetVariant =
    std::variant<NoTarget, EntityTarget, OnePointSkillshot, TwoPointSkillshot>;

}  // namespace lol_at_home_shared
