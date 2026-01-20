#include "EnetInterface.h"
#include <spdlog/spdlog.h>
#include "c2s_message_generated.h"
#include "domain/GameAction.h"
#include "serialization/C2SMessageSerializer.h"

namespace lah::server {

namespace {

void logGameAction(const lah::shared::GameActionVariant& action) {
  std::visit(
      [](const auto& a) {
        using T = std::decay_t<decltype(a)>;
        if constexpr (std::is_same_v<T, lah::shared::MoveAction>) {
          spdlog::info("Received MoveAction: source=" +
                       std::to_string(static_cast<uint32_t>(a.source)) +
                       " target=(" + std::to_string(a.targetX) + ", " +
                       std::to_string(a.targetY) + ")");
        } else if constexpr (std::is_same_v<T, lah::shared::AutoAttackAction>) {
          spdlog::info(
              "Received AutoAttackAction: source=" +
              std::to_string(static_cast<uint32_t>(a.source)) +
              " target=" + std::to_string(static_cast<uint32_t>(a.target)));
        } else if constexpr (std::is_same_v<T, lah::shared::AbilityAction>) {
          spdlog::info("Received AbilityAction: source=" +
                       std::to_string(static_cast<uint32_t>(a.source)) +
                       " slot=" + std::to_string(static_cast<int>(a.slot)));
        } else if constexpr (std::is_same_v<T, lah::shared::StopGameAction>) {
          spdlog::info("Received StopGameAction: source=" +
                       std::to_string(static_cast<uint32_t>(a.source)));
        }
      },
      action);
}

}  // namespace

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
  populateInbound();
  sendOutbound();
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
        std::span<const std::byte> data(
            reinterpret_cast<const std::byte*>(event.packet->data),
            event.packet->dataLength);

        switch (lah::shared::C2SMessageSerializer::GetMessageType(data)) {
          case lah::shared::C2SMessageType::GameAction: {
            if (auto actionVariant =
                    lah::shared::C2SMessageSerializer::DeserializeGameAction(
                        data)) {
              logGameAction(*actionVariant);
              inbound_->Push(
                  InboundEvent{.peer = event.peer, .event = *actionVariant});
            }
            break;
          }

          case lah::shared::C2SMessageType::ChampionSelect: {
            if (auto champSelect = lah::shared::C2SMessageSerializer::
                    DeserializeChampionSelect(data)) {
              inbound_->Push(
                  InboundEvent{.peer = event.peer,
                               .event = ChampionSelectedEvent{
                                   .championId = champSelect->championId,
                                   .teamColor = champSelect->teamColor}});
            }
            break;
          }

          case lah::shared::C2SMessageType::ChatMessage: {
            // todo use DeserializeChatMessage
            const lah_shared::C2SMessageFB* c2sMessage =
                lah_shared::GetC2SMessageFB(data.data());
            const lah_shared::ChatMessageFB* chatFb =
                c2sMessage->message_as_ChatMessageFB();

            InboundChatEvent chatEvent{.message = chatFb->text()->str()};
            inbound_->Push(
                InboundEvent{.peer = event.peer, .event = chatEvent});
            break;
          }

          case lah::shared::C2SMessageType::Unknown:
            spdlog::warn("Received empty or unknown message type");
            break;
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

}  // namespace lah::server
