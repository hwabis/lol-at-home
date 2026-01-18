#pragma once

#include <SDL3/SDL.h>
#include "Camera.h"
#include "GameConfig.h"
#include "InputAccessor.h"

namespace lah::engine {

struct SceneInfo {
  explicit SceneInfo(const GameConfig& config)
      : config(config), camera(config.windowSize) {
    camera.SetPosition(config.initialCameraPosition);
  }

  // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
  GameConfig config;
  Camera camera;
  InputAccessor input{};
  SDL_Window* window = nullptr;
  SDL_Renderer* sdlRenderer = nullptr;
  // NOLINTEND(misc-non-private-member-variables-in-classes)
};

}  // namespace lah::engine
