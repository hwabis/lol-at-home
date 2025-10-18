#pragma once

#include <entt/entt.hpp>
#include "GameAction.h"

namespace lol_at_home_server {

struct GameStateDelta {
  std::vector<lol_at_home_shared::GameActionVariant> Actions;
  std::vector<entt::entity> ChangedEntities;
};

}  // namespace lol_at_home_server
