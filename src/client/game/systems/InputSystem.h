#pragma once

#include <utility>

#include "IEcsSystem.h"
#include "OutboundEvent.h"
#include "ThreadSafeQueue.h"

namespace lol_at_home_game {

class InputSystem : public lol_at_home_engine::IEcsSystem {
 public:
  explicit InputSystem(
      std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outboundEvents)
      : outboundEvents_(std::move(outboundEvents)) {}

  void Cycle(entt::registry& registry,
             lol_at_home_engine::SceneInfo& info,
             std::chrono::duration<double, std::milli> /*deltaTime*/) override {
    if (info.input.IsMouseButtonPressed(SDL_BUTTON_RIGHT)) {
      auto mousePos = info.input.GetMousePosition();
      auto worldPos = info.camera.ScreenToWorld(mousePos);

      // todo send to server
    }
  }

 private:
  std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outboundEvents_;
};

}  // namespace lol_at_home_game
