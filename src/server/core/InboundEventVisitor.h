#pragma once

#include <enet/enet.h>
#include <entt/entt.hpp>
#include <unordered_map>
#include "InboundEvent.h"
#include "OutboundEvent.h"
#include "util/ThreadSafeQueue.h"

namespace lah::server {

class InboundEventVisitor {
 public:
  InboundEventVisitor(
      ENetPeer* peer,
      entt::registry* registry,
      std::unordered_map<ENetPeer*, entt::entity>* peerToEntityMap,
      std::vector<entt::entity>* instantDirty,
      std::vector<entt::entity>* deletedEntities,
      ThreadSafeQueue<OutboundEvent>* outbound);

  void operator()(const ChampionSelectedEvent& event) const;
  void operator()(const ClientDisconnectedEvent& event) const;
  void operator()(const InboundChatEvent& event) const;
  void operator()(const lah::shared::GameActionVariant& action) const;

 private:
  ENetPeer* peer_;
  entt::registry* registry_;
  std::unordered_map<ENetPeer*, entt::entity>* peerToEntityMap_;
  std::vector<entt::entity>* instantDirty_;
  std::vector<entt::entity>* deletedEntities_;
  ThreadSafeQueue<OutboundEvent>* outbound_;
};

}  // namespace lah::server
