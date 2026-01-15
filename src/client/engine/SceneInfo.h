#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <utility>
#include "Camera.h"
#include "InputAccessor.h"

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
};

struct SceneInfo {
  explicit SceneInfo(GameConfig config)
      : config(std::move(config)), camera(config.windowSize) {}

  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
  GameConfig config;
  Camera camera;
  InputAccessor input{};
  SDL_Window* window = nullptr;
  SDL_Renderer* sdlRenderer = nullptr;
  // NOLINTEND(misc-non-private-member-variables-in-classes)
};

}  // namespace lah::engine
