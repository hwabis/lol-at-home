#include "EnetInterface.h"
#include <spdlog/spdlog.h>

namespace lol_at_home_server {

EnetInterface::EnetInterface(
    std::shared_ptr<ThreadSafeQueue<InboundPacket>> inbound,
    std::shared_ptr<ThreadSafeQueue<OutboundPacket>> outbound)
    : inbound_(std::move(inbound)), outbound_(std::move(outbound)) {
  enet_initialize();
  ENetAddress address{.host = ENET_HOST_ANY, .port = 12345};
  host_ = enet_host_create(&address, 32, 2, 0, 0);
}

EnetInterface::~EnetInterface() {
  enet_host_destroy(host_);
  enet_deinitialize();
}

auto EnetInterface::Cycle(std::chrono::milliseconds timeElapsed) -> void {
  sendOutbound();
  populateInbound();
}

void EnetInterface::sendOutbound() {
  // todo pop all outbound_, serialize and send or smth lol
  // enet_peer_send or enet_host_broadcast?
}

void EnetInterface::populateInbound() {
  ENetEvent event;
  while (enet_host_service(host_, &event, 1) > 0) {
    switch (event.type) {
      case ENET_EVENT_TYPE_CONNECT: {
        spdlog::info("Client connected");
        // todo PlayerAssignmentSerializer::Serialize or something, push to
        // inbound? no idea how we're doing proper server-side player assignment
        // yet send the peer the full game state?
        break;
      }

      case ENET_EVENT_TYPE_RECEIVE: {
        spdlog::info("Received " + std::to_string(event.packet->dataLength) +
                     " bytes from client");

        // todo GameActionSerializer::Deserialize i guess, push to inbound

        enet_packet_destroy(event.packet);
        break;
      }
    }
  }
}

}  // namespace lol_at_home_server
