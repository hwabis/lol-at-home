#pragma once

namespace lah::shared {

constexpr float ARENA_MIN_X = 0.0F;
constexpr float ARENA_MIN_Y = 0.0F;
constexpr float ARENA_MAX_X = 1000.0F;
constexpr float ARENA_MAX_Y = 1000.0F;

constexpr auto IsInArena(float x, float y) -> bool {
  return x >= ARENA_MIN_X && x <= ARENA_MAX_X && y >= ARENA_MIN_Y &&
         y <= ARENA_MAX_Y;
}

}  // namespace lah::shared
