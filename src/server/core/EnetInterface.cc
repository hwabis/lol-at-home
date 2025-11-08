#include "EnetInterface.h"
#include <spdlog/spdlog.h>
#include "GameActionSerializer.h"

namespace lol_at_home_server {

EnetInterface::EnetInterface(
    std::shared_ptr<ThreadSafeQueue<InboundEvent>> inbound,
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
  std::queue<OutboundPacket> packets = outbound_->PopAll();

  while (!packets.empty()) {
    OutboundPacket packet = std::move(packets.front());
    packets.pop();

    ENetPacket* enetPacket = enet_packet_create(
        packet.data.data(), packet.data.size(), packet.flags);

    if (packet.peer == nullptr) {
      enet_host_broadcast(host_, packet.channel, enetPacket);
    } else {
      enet_peer_send(packet.peer, packet.channel, enetPacket);
    }
  }

  enet_host_flush(host_);
}

void EnetInterface::populateInbound() {
  ENetEvent event;
  while (enet_host_service(host_, &event, 1) > 0) {
    switch (event.type) {
      case ENET_EVENT_TYPE_CONNECT: {
        spdlog::info("Client connected");
        inbound_->Push(InboundEvent{.peer = event.peer,
                                     .action = ClientConnectedEvent{}});
        break;
      }

      case ENET_EVENT_TYPE_RECEIVE: {
        spdlog::info("Received " + std::to_string(event.packet->dataLength) +
                     " bytes from client");

        std::vector<std::byte> data(
            reinterpret_cast<const std::byte*>(event.packet->data),
            reinterpret_cast<const std::byte*>(event.packet->data) +
                event.packet->dataLength);
        std::optional<lol_at_home_shared::GameActionVariant> actionVariant =
            lol_at_home_shared::GameActionSerializer::Deserialize(data);

        if (actionVariant.has_value()) {
          inbound_->Push(
              InboundEvent{.peer = event.peer, .action = *actionVariant});
        }

        enet_packet_destroy(event.packet);
        break;
      }

      case ENET_EVENT_TYPE_DISCONNECT: {
        spdlog::info("Client disconnected");
        inbound_->Push(InboundEvent{.peer = event.peer,
                                     .action = ClientDisconnectedEvent{}});
        break;
      }

      case ENET_EVENT_TYPE_NONE:
        break;
    }
  }
}

}  // namespace lol_at_home_server
