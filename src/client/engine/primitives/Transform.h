#pragma once

#include "primitives/Vector2.h"

namespace lol_at_home_engine {

struct Transform {
  // todo world position
  Vector2 position{};
  float rotation{};
  float scale{1.0F};
};

}  // namespace lol_at_home_engine
