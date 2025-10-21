#pragma once

#include <cstdint>

namespace lol_at_home_engine {

struct Vector2 {
  double X{};
  double Y{};

  auto operator+(const Vector2& other) const -> Vector2 {
    return {.X = X + other.X, .Y = Y + other.Y};
  }

  auto operator-(const Vector2& other) const -> Vector2 {
    return {.X = X - other.X, .Y = Y - other.Y};
  }

  auto operator*(double scalar) const -> Vector2 {
    return {.X = X * scalar, .Y = Y * scalar};
  }

  auto operator/(double scalar) const -> Vector2 {
    return {.X = X / scalar, .Y = Y / scalar};
  }
};

struct Color {
  uint8_t r{};
  uint8_t g{};
  uint8_t b{};
  uint8_t a{255};
};

}  // namespace lol_at_home_engine
