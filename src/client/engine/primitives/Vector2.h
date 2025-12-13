#pragma once

namespace lol_at_home_engine {

struct Vector2 {
  float x{};
  float y{};

  auto operator+(const Vector2& other) const -> Vector2 {
    return {.x = x + other.x, .y = y + other.y};
  }

  auto operator-(const Vector2& other) const -> Vector2 {
    return {.x = x - other.x, .y = y - other.y};
  }

  auto operator*(float scalar) const -> Vector2 {
    return {.x = x * scalar, .y = y * scalar};
  }

  auto operator/(float scalar) const -> Vector2 {
    return {.x = x / scalar, .y = y / scalar};
  }
};

}  // namespace lol_at_home_engine
