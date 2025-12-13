#pragma once

#include <cstdint>

namespace lol_at_home_engine {

struct Color {
  static constexpr int maxValue = 255;

  uint8_t r{};
  uint8_t g{};
  uint8_t b{};
  uint8_t a{maxValue};
};

}  // namespace lol_at_home_engine
