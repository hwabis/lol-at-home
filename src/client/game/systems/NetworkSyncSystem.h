#pragma once

#include "IEcsSystem.h"
#include "InboundEvent.h"
#include "InboundEventVisitor.h"
#include "ThreadSafeQueue.h"

namespace lah::game {

class NetworkSyncSystem : public lah::engine::IEcsSystem {
 public:
  explicit NetworkSyncSystem(
      std::shared_ptr<ThreadSafeQueue<InboundEvent>> inboundEvents)
      : inboundEvents_(std::move(inboundEvents)) {}

  void Cycle(entt::registry& registry,
             lah::engine::SceneInfo& /*info*/,
             std::chrono::duration<double, std::milli> /*deltaTime*/) override {
    std::queue<InboundEvent> events = inboundEvents_->PopAll();

    while (!events.empty()) {
      auto& event = events.front();
      std::visit(InboundEventVisitor{&registry}, event.event);
      events.pop();
    }
  }

 private:
  std::shared_ptr<ThreadSafeQueue<InboundEvent>> inboundEvents_;
};

}  // namespace lah::game
