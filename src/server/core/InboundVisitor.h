#pragma once

#include <enet/enet.h>
#include <entt/entt.hpp>
#include <unordered_map>
#include "InboundPacket.h"

namespace lol_at_home_server {

class InboundVisitor {
 public:
  InboundVisitor(ENetPeer* peer,
                 entt::registry* registry,
                 std::unordered_map<ENetPeer*, entt::entity>* peerToEntityMap);

  void operator()(const ClientConnectedEvent& event) const;
  void operator()(const ClientDisconnectedEvent& event) const;
  void operator()(const lol_at_home_shared::GameActionVariant& action) const;

 private:
  ENetPeer* peer_;
  entt::registry* registry_;
  std::unordered_map<ENetPeer*, entt::entity>* peerToEntityMap_;
};

}  // namespace lol_at_home_server
