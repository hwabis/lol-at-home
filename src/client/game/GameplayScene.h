#pragma once

#include <enet/enet.h>
#include <entt/entt.hpp>
#include "Scene.h"

namespace lol_at_home_game {

class GameplayScene : public lol_at_home_engine::Scene {
 public:
  GameplayScene(SDL_Renderer* renderer, int width, int height);
};

}  // namespace lol_at_home_game
