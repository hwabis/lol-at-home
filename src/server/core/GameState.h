#pragma once

#include <enet/enet.h>
#include "InboundPacket.h"
#include "OutboundPacket.h"
#include "util/IPeriodic.h"
#include "util/ThreadSafeQueue.h"

namespace lol_at_home_server {

class GameState : public IPeriodic {
 public:
  GameState(std::shared_ptr<ThreadSafeQueue<InboundPacket>> inbound_,
            std::shared_ptr<ThreadSafeQueue<OutboundPacket>> outbound_);

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

  entt::registry registry_;
  std::unordered_map<ENetPeer*, entt::entity> peerToEntityMap_;
  std::shared_ptr<ThreadSafeQueue<InboundPacket>> inbound_;
  std::shared_ptr<ThreadSafeQueue<OutboundPacket>> outbound_;
};

}  // namespace lol_at_home_server
