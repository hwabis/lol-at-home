#pragma once

#include <entt/entt.hpp>
#include "Vector2.h"

namespace lol_at_home_game {

struct LocalPlayer {
  uint32_t serverEntityId;
};

struct Transform {
  lol_at_home_engine::Vector2 worldPosition{};
  float rotation{};
  float scale{1.0F};
};

struct RenderableCircle {
  float radius{1.0F};
};

struct Health {
  float current{};
  float regenPerSec{};
  float max{};
};

struct Team {
  enum class Color : uint8_t { Blue, Red } color;
};

}  // namespace lol_at_home_game
