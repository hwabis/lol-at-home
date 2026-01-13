#pragma once

#include <entt/entt.hpp>
#include "Vector2.h"

namespace lol_at_home_game {

struct LocalPlayer {
  uint32_t serverEntityId;
};

struct Transform {
  lol_at_home_engine::Vector2 worldPosition{};
  // todo default init this, and in EntityUpdatedEvent creation, set it to the
  // value from the flatbuffer
  float championRadius = 50.0F;
};

struct Health {
  float current{};
  float regenPerSec{};
  float max{};
};

struct Team {
  // todo add a neutral team
  enum class Color : uint8_t {
    Blue,
    Red,
  } color;
};

}  // namespace lol_at_home_game
