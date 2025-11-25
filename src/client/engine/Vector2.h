#pragma once

#include <cstdint>

namespace lol_at_home_engine {

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

struct Color {
  static constexpr int maxValue = 255;

  uint8_t r{};
  uint8_t g{};
  uint8_t b{};
  uint8_t a{maxValue};
};

}  // namespace lol_at_home_engine
