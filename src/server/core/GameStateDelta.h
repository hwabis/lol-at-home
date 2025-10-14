#pragma once

#include <entt/entt.hpp>
#include "actions/GameAction.h"

namespace lol_at_home_server {

struct GameStateDelta {
  std::vector<GameActionVariant> Actions;
  std::vector<entt::entity> ChangedEntities;
};

}  // namespace lol_at_home_server
