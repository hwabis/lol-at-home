#pragma once

#include <vector>
#include "EntityStats.h"
#include "GameAction.h"

namespace lol_at_home_server {

struct GameStateDelta {
  std::vector<GameActionVariant> Actions;
  std::vector<EntityStatsVariant> UpdatedEntities;
};

}  // namespace lol_at_home_server
