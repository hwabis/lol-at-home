#pragma once

namespace lah::engine {

struct Vector2 {
  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
  float x{};
  float y{};
  // NOLINTEND(misc-non-private-member-variables-in-classes)

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

}  // namespace lah::engine
