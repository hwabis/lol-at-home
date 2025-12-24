#pragma once

#include "Components.h"
#include "IEcsSystem.h"

namespace lol_at_home_game {

class InputSystem : public lol_at_home_engine::IEcsSystem {
 public:
  void Cycle(entt::registry& registry,
             lol_at_home_engine::SceneInfo& info,
             std::chrono::duration<double, std::milli> /*deltaTime*/) override {
    if (info.input.IsMouseButtonPressed(SDL_BUTTON_RIGHT)) {
      auto mousePos = info.input.GetMousePosition();
      auto worldPos = info.camera.ScreenToWorld(mousePos);

      // todo send to server
    }
  }
};

}  // namespace lol_at_home_game
