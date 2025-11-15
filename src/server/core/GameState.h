#pragma once

#include <enet/enet.h>
#include "InboundEvent.h"
#include "OutboundEvent.h"
#include "ecs_systems/IEcsSystem.h"
#include "util/IPeriodic.h"
#include "util/ThreadSafeQueue.h"

namespace lol_at_home_server {

class GameState : public IPeriodic {
 public:
  GameState(std::shared_ptr<ThreadSafeQueue<InboundEvent>> inbound,
            std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outbound);

  auto Cycle(std::chrono::milliseconds timeElapsed) -> void override;

 private:
  void processInbound();
  auto validateInboundEventPeer(const InboundEvent& event) -> bool;
  void pushOutbound(const std::vector<entt::entity>& dirtyEntities,
                    const std::vector<entt::entity>& deletedEntities);

  std::unordered_map<ENetPeer*, entt::entity> peerToEntityMap_;
  entt::registry registry_;
  std::shared_ptr<ThreadSafeQueue<InboundEvent>> inbound_;
  std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outbound_;
  std::vector<std::unique_ptr<IEcsSystem>> systems_;
};

}  // namespace lol_at_home_server
