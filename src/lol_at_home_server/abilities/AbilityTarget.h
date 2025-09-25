#pragma once

#include <entt/entt.hpp>
#include <variant>
#include "ecs/EcsComponents.h"

namespace lol_at_home_server {

struct NoTarget {};

struct EntityTarget {
  entt::entity Target;
};

struct OnePointSkillshot {
  Position Target;
};

// Rumble R, Viktor E
struct TwoPointSkillshot {
  Position Target1;
  Position Target2;
};

using AbilityTargetVariant =
    std::variant<NoTarget, EntityTarget, OnePointSkillshot, TwoPointSkillshot>;

}  // namespace lol_at_home_server
