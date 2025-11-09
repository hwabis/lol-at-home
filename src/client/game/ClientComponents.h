#pragma once

#include "domain/EcsComponents.h"

namespace lol_at_home_game {

struct VisualPosition {
  lol_at_home_shared::Position Position;
  lol_at_home_shared::Position Velocity{.X = 0.0,
                                        .Y = 0.0};  // Units per second
  double TimeSinceLastUpdate = 0.0;
};

}  // namespace lol_at_home_game
