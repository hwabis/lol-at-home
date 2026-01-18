#pragma once

#include <flatbuffers/flatbuffers.h>
#include <spdlog/spdlog.h>
#include "Components.h"
#include "IEcsSystem.h"
#include "OutboundEvent.h"
#include "ThreadSafeQueue.h"
#include "c2s_message_generated.h"

namespace lah::game {

class InputMovementSystem : public lah::engine::IEcsSystem {
 public:
  explicit InputMovementSystem(
      std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outboundEvents)
      : outboundEvents_(std::move(outboundEvents)) {}

  void Cycle(entt::registry& registry,
             lah::engine::SceneInfo& info,
             std::chrono::duration<double, std::milli> /*deltaTime*/) override {
    if (info.input.IsMouseButtonPressed(SDL_BUTTON_RIGHT)) {
      auto mousePos = info.input.GetMousePosition();
      auto worldPos = info.camera.ScreenToWorld(mousePos);

      auto view = registry.view<LocalPlayer>();
      if (view.empty()) {
        spdlog::warn("No LocalPlayer entity found for InputMovementSystem");
        return;
      }

      auto& localPlayer = view.get<LocalPlayer>(view.front());

      flatbuffers::FlatBufferBuilder builder;

      auto targetPos = lah_shared::PositionFB(worldPos.x, worldPos.y);
      auto moveAction = lah_shared::CreateMoveActionFB(builder, &targetPos);

      auto gameAction = lah_shared::CreateGameActionFB(
          builder, localPlayer.serverEntityId,
          lah_shared::GameActionDataFB::MoveActionFB, moveAction.Union());

      auto c2sMessage = lah_shared::CreateC2SMessageFB(
          builder, lah_shared::C2SDataFB::GameActionFB, gameAction.Union());

      builder.Finish(c2sMessage);

      OutboundEvent event;
      event.c2sMessage = std::vector<std::byte>(
          reinterpret_cast<const std::byte*>(builder.GetBufferPointer()),
          reinterpret_cast<const std::byte*>(builder.GetBufferPointer()) +
              builder.GetSize());
      outboundEvents_->Push(event);
    }
  }

 private:
  std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outboundEvents_;
};

}  // namespace lah::game
