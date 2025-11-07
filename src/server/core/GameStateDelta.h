#pragma once

#include <entt/entt.hpp>
#include "GameAction.h"

namespace lol_at_home_server {

// todo erm this isnt used anywhere anymore now lol prob used somewhere in the
// (de)serialization step?
struct GameStateDelta {
  std::vector<lol_at_home_shared::GameActionVariant> Actions;
  std::vector<entt::entity> ChangedEntities;
};

}  // namespace lol_at_home_server
