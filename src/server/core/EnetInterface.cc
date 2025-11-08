#include "EnetInterface.h"
#include <spdlog/spdlog.h>
#include "GameActionSerializer.h"
#include "InboundEvent.h"
#include "c2s_message_generated.h"

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
        inbound_->Push(
            InboundEvent{.peer = event.peer, .action = ClientConnectedEvent{}});
        break;
      }

      case ENET_EVENT_TYPE_RECEIVE: {
        spdlog::info("Received " + std::to_string(event.packet->dataLength) +
                     " bytes from client");

        std::vector<std::byte> data(
            reinterpret_cast<const std::byte*>(event.packet->data),
            reinterpret_cast<const std::byte*>(event.packet->data) +
                event.packet->dataLength);
        const lol_at_home_shared::C2SMessageFB* c2sMessage =
            lol_at_home_shared::GetC2SMessageFB(data.data());

        switch (c2sMessage->message_type()) {
          case lol_at_home_shared::C2SDataFB::GameActionFB: {
            const lol_at_home_shared::GameActionFB* action =
                c2sMessage->message_as_GameActionFB();

            std::optional<lol_at_home_shared::GameActionVariant>
                action_variant =
                    lol_at_home_shared::GameActionSerializer::UnpackGameAction(
                        action);

            if (action_variant.has_value()) {
              inbound_->Push(
                  InboundEvent{.peer = event.peer, .action = *action_variant});
            }

            break;
          }

          case lol_at_home_shared::C2SDataFB::ChatMessageFB: {
            const lol_at_home_shared::ChatMessageFB* chat_fb =
                c2sMessage->message_as_ChatMessageFB();

            InboundChatEvent chatEvent{.message = chat_fb->text()->str()};

            inbound_->Push(
                InboundEvent{.peer = event.peer, .action = chatEvent});
            break;
          }

          case lol_at_home_shared::C2SDataFB::NONE:
            spdlog::warn("Received empty or unknown message type");
            break;
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
