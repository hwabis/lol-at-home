#pragma once

#include <vector>
#include "EntityStats.h"
#include "GameAction.h"

namespace lol_at_home_server {

struct GameStateDelta {
  std::vector<GameAction> Actions;
  std::vector<EntityStats> UpdatedEntities;
};

}  // namespace lol_at_home_server
