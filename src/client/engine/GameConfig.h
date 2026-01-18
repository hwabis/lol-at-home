#pragma once

#include <string>
#include "Vector2.h"

namespace lah::engine {

struct GameConfig {
  enum class WindowType : uint8_t {
    Windowed,
    Fullscreen,
    BorderlessFullscreen,
  };

  std::string windowTitle;
  WindowType windowType;
  Vector2 windowSize{};  // only used in windowed mode
  int targetFPS{};
  Vector2 initialCameraPosition{};
};

}  // namespace lah::engine
