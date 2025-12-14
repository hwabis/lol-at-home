#pragma once

#include <enet/enet.h>
#include <entt/entt.hpp>
#include "Scene.h"

namespace lol_at_home_game {

class GameplayScene : public lol_at_home_engine::Scene {
 public:
  explicit GameplayScene(SDL_Renderer* renderer);
};

}  // namespace lol_at_home_game
