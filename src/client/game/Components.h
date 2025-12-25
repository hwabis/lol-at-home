#pragma once

#include <entt/entt.hpp>
#include "Vector2.h"

namespace lol_at_home_game {

struct LocalPlayer {
  entt::entity serverEntityId;
};

struct Transform {
  lol_at_home_engine::Vector2 position{};
  float rotation{};
  float scale{1.0F};
};

struct RenderableCircle {
  float radius{1.0F};
};

}  // namespace lol_at_home_game
