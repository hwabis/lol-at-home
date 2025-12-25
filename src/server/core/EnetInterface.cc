#include "EnetInterface.h"
#include <spdlog/spdlog.h>
#include "c2s_message_generated.h"
#include "domain/EcsComponents.h"
#include "serialization/GameActionSerializer.h"

namespace lol_at_home_server {

EnetInterface::EnetInterface(
    std::shared_ptr<ThreadSafeQueue<InboundEvent>> inbound,
    std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outbound,
    uint16_t port)
    : inbound_(std::move(inbound)), outbound_(std::move(outbound)) {
  enet_initialize();
  ENetAddress address{.host = ENET_HOST_ANY, .port = port};
  constexpr int numOfPlayers = 10;  // technically doesn't handle spectators lol
  host_ = enet_host_create(&address, numOfPlayers, 1, 0, 0);
  if (host_ == nullptr) {
    spdlog::error("Failed to create ENet server host");
    throw std::runtime_error("ENet server creation failed");
  }

  spdlog::info("Server listening on port " + std::to_string(port));
}

EnetInterface::~EnetInterface() {
  enet_host_destroy(host_);
  enet_deinitialize();
}

auto EnetInterface::Cycle(std::chrono::milliseconds /*timeElapsed*/) -> void {
  sendOutbound();
  populateInbound();
}

void EnetInterface::sendOutbound() {
  std::queue<OutboundEvent> events = outbound_->PopAll();

  while (!events.empty()) {
    OutboundEvent event = std::move(events.front());
    events.pop();

    ENetPacket* packet =
        enet_packet_create(event.s2cMessage.data(), event.s2cMessage.size(),
                           ENET_PACKET_FLAG_RELIABLE);

    if (event.target == nullptr) {
      enet_host_broadcast(host_, 0, packet);
    } else {
      enet_peer_send(event.target, 0, packet);
    }
  }

  enet_host_flush(host_);
}

void EnetInterface::populateInbound() {
  ENetEvent event;
  while (enet_host_service(host_, &event, 1) > 0) {
    switch (event.type) {
      case ENET_EVENT_TYPE_CONNECT: {
        spdlog::info("Client connected - awaiting champion select");
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

            std::optional<lol_at_home_shared::GameActionVariant> actionVariant =
                lol_at_home_shared::GameActionSerializer::Deserialize(*action);

            if (actionVariant.has_value()) {
              inbound_->Push(
                  InboundEvent{.peer = event.peer, .event = *actionVariant});
            }

            break;
          }

          case lol_at_home_shared::C2SDataFB::ChatMessageFB: {
            const lol_at_home_shared::ChatMessageFB* chat_fb =
                c2sMessage->message_as_ChatMessageFB();

            InboundChatEvent chatEvent{.message = chat_fb->text()->str()};

            inbound_->Push(
                InboundEvent{.peer = event.peer, .event = chatEvent});
            break;
          }

          case lol_at_home_shared::C2SDataFB::ChampionSelectFB: {
            const auto* champSelect = c2sMessage->message_as_ChampionSelectFB();
            auto champId = static_cast<lol_at_home_shared::ChampionId>(
                champSelect->champion_id());
            auto team = champSelect->team_color() ==
                                lol_at_home_shared::TeamColorFB::Red
                            ? lol_at_home_shared::Team::Color::Red
                            : lol_at_home_shared::Team::Color::Blue;

            inbound_->Push(
                InboundEvent{.peer = event.peer,
                             .event = ChampionSelectedEvent{
                                 .championId = champId, .teamColor = team}});

            break;
          }

          case lol_at_home_shared::C2SDataFB::NONE: {
            spdlog::warn("Received empty or unknown message type");
            break;
          }
        }

        enet_packet_destroy(event.packet);
        break;
      }

      case ENET_EVENT_TYPE_DISCONNECT: {
        spdlog::info("Client disconnected");
        inbound_->Push(InboundEvent{.peer = event.peer,
                                    .event = ClientDisconnectedEvent{}});
        break;
      }

      case ENET_EVENT_TYPE_NONE:
        break;
    }
  }
}

}  // namespace lol_at_home_server
