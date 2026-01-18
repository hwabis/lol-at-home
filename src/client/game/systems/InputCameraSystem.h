#pragma once

#include <SDL3/SDL.h>
#include "IEcsSystem.h"

namespace lah::game {

class InputCameraSystem : public lah::engine::IEcsSystem {
 public:
  void Cycle(entt::registry& /*registry*/,
             lah::engine::SceneInfo& info,
             std::chrono::duration<double, std::milli> deltaTime) override {
    constexpr float cameraPanSpeed = 300.0F;  // units per second

    auto cameraDelta = lah::engine::Vector2{.x = 0.0F, .y = 0.0F};

    auto deltaSeconds = static_cast<float>(deltaTime.count()) / 1000.0F;
    float panAmount = cameraPanSpeed * deltaSeconds;

    if (info.input.IsKeyPressed(SDLK_UP)) {
      cameraDelta.y -= panAmount;
    }
    if (info.input.IsKeyPressed(SDLK_DOWN)) {
      cameraDelta.y += panAmount;
    }
    if (info.input.IsKeyPressed(SDLK_LEFT)) {
      cameraDelta.x -= panAmount;
    }
    if (info.input.IsKeyPressed(SDLK_RIGHT)) {
      cameraDelta.x += panAmount;
    }

    auto currentPos = info.camera.GetPosition();
    info.camera.SetPosition(currentPos + cameraDelta);
  }
};

}  // namespace lah::game
