#include "InboundVisitor.h"
#include <spdlog/spdlog.h>

namespace lol_at_home_server {

InboundVisitor::InboundVisitor(
    ENetPeer* peer,
    entt::registry* registry,
    std::unordered_map<ENetPeer*, entt::entity>* peerToEntityMap)
    : peer_(peer), registry_(registry), peerToEntityMap_(peerToEntityMap) {}

void InboundVisitor::operator()(const ClientConnectedEvent& event) const {
  entt::entity entity = registry_->create();
  registry_->emplace<lol_at_home_shared::Position>(entity, 100.0, 200.0);
  peerToEntityMap_->emplace(peer_, entity);

  // todo queue up outbound PlayerAssignment packet ? assuming we want
  // assignment to work that way
}

void InboundVisitor::operator()(const ClientDisconnectedEvent& event) const {
  // todo .. somehow have to know if there's a reconnect, we check if they match
  // an existing peer
}

void InboundVisitor::operator()(
    const lol_at_home_shared::GameActionVariant& action) const {
  // todo ACTION visitor, hook up GameActionProcessor here
}

}  // namespace lol_at_home_server
