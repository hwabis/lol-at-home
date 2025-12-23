#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <utility>
#include "Camera.h"
#include "InputAccessor.h"

namespace lol_at_home_engine {

struct GameConfig {
  std::string windowTitle;
  Vector2 windowSize{};
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

}  // namespace lol_at_home_engine
