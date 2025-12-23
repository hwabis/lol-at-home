#pragma once

#include "Vector2.h"

namespace lol_at_home_game {

struct Transform {
  lol_at_home_engine::Vector2 position{};
  float rotation{};
  float scale{1.0F};
};

struct RenderableCircle {
  float radius{1.0F};
};

}  // namespace lol_at_home_game
