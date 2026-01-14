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
  float targetX{};
  float targetY{};
};

// Rumble R, Viktor E
struct TwoPointSkillshot {
  float target1X{};
  float target1Y{};
  float target2X{};
  float target2Y{};
};

using AbilityTargetVariant =
    std::variant<NoTarget, EntityTarget, OnePointSkillshot, TwoPointSkillshot>;

}  // namespace lol_at_home_shared
