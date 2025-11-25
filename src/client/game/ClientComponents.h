#pragma once

#include <chrono>
#include "domain/EcsComponents.h"

namespace lol_at_home_game {

struct VisualPosition {
  lol_at_home_shared::Position position{};
  lol_at_home_shared::Position velocity{};
  std::chrono::milliseconds timeSinceLastUpdate{};
};

}  // namespace lol_at_home_game
