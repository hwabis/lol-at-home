#pragma once

#include <enet/enet.h>
#include "InboundEvent.h"
#include "OutboundEvent.h"
#include "util/IPeriodic.h"
#include "util/ThreadSafeQueue.h"
#include "util/ThreadSafeRegistry.h"

namespace lol_at_home_server {

class GameState : public IPeriodic {
 public:
  GameState(std::shared_ptr<ThreadSafeRegistry> registry,
            std::shared_ptr<ThreadSafeQueue<InboundEvent>> inbound,
            std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outbound);

  auto Cycle(std::chrono::milliseconds timeElapsed) -> void override;

 private:
  void processInbound();
  void updateSimulation(std::chrono::milliseconds timeElapsed,
                        std::vector<entt::entity>& dirtyEntities);
  void pushOutbound(const std::vector<entt::entity>& dirtyEntities);

  void updateMovementSystem(std::chrono::milliseconds timeElapsed,
                            std::vector<entt::entity>& dirtyEntities);
  void updateHealthSystem(std::chrono::milliseconds timeElapsed,
                          std::vector<entt::entity>& dirtyEntities);

  std::unordered_map<ENetPeer*, entt::entity> peerToEntityMap_;
  std::shared_ptr<ThreadSafeRegistry> registry_;
  std::shared_ptr<ThreadSafeQueue<InboundEvent>> inbound_;
  std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outbound_;
};

}  // namespace lol_at_home_server
