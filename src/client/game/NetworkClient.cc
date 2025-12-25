#include "NetworkClient.h"
#include <spdlog/spdlog.h>
#include "c2s_message_generated.h"
#include "s2c_message_generated.h"

namespace lol_at_home_game {

NetworkClient::NetworkClient(
    std::shared_ptr<ThreadSafeQueue<InboundEvent>> inboundEvents,
    std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outboundEvents)
    : inboundEvents_(std::move(inboundEvents)),
      outboundEvents_(std::move(outboundEvents)) {
  if (enet_initialize() != 0) {
    spdlog::error("Failed to initialize ENet");
    throw std::runtime_error("ENet initialization failed");
  }

  client_ = enet_host_create(nullptr, 1, 1, 0, 0);

  if (client_ == nullptr) {
    spdlog::error("Failed to create ENet client host");
    enet_deinitialize();
    throw std::runtime_error("Failed to create ENet client");
  }
}

NetworkClient::~NetworkClient() {
  Disconnect();

  if (client_ != nullptr) {
    enet_host_destroy(client_);
  }

  enet_deinitialize();
}

auto NetworkClient::Connect(const char* host, uint16_t port) -> bool {
  if (connected_) {
    spdlog::warn("Already connected");
    return true;
  }

  ENetAddress address;
  enet_address_set_host(&address, host);
  address.port = port;
  serverPeer_ = enet_host_connect(client_, &address, 2, 0);

  if (serverPeer_ == nullptr) {
    spdlog::error("No available peers for initiating connection");
    return false;
  }

  ENetEvent event;
  if (enet_host_service(client_, &event, 5000) > 0 &&
      event.type == ENET_EVENT_TYPE_CONNECT) {
    spdlog::info("Connected to server");
    connected_ = true;

    // lmaooo todo be able to choose champ/team
    sendChampionSelect();

    return true;
  }

  spdlog::error("Connection to server failed");
  enet_peer_reset(serverPeer_);
  serverPeer_ = nullptr;
  return false;
}

void NetworkClient::Disconnect() {
  if (!connected_ || serverPeer_ == nullptr) {
    return;
  }

  enet_peer_disconnect(serverPeer_, 0);

  ENetEvent event;
  bool disconnected = false;

  while (enet_host_service(client_, &event, 3000) > 0) {
    switch (event.type) {
      case ENET_EVENT_TYPE_RECEIVE:
        enet_packet_destroy(event.packet);
        break;
      case ENET_EVENT_TYPE_DISCONNECT:
        spdlog::info("Disconnected from server");
        disconnected = true;
        break;
      default:
        break;
    }

    if (disconnected) {
      break;
    }
  }

  if (!disconnected) {
    enet_peer_reset(serverPeer_);
  }

  serverPeer_ = nullptr;
  connected_ = false;
}

void NetworkClient::Poll() {
  if (client_ == nullptr) {
    return;
  }

  ENetEvent event;

  while (enet_host_service(client_, &event, 0) > 0) {
    switch (event.type) {
      case ENET_EVENT_TYPE_RECEIVE: {
        std::vector<std::byte> data(
            reinterpret_cast<std::byte*>(event.packet->data),
            reinterpret_cast<std::byte*>(event.packet->data) +
                event.packet->dataLength);

        const auto* s2cMessage =
            lol_at_home_shared::GetS2CMessageFB(data.data());

        switch (s2cMessage->message_type()) {
          case lol_at_home_shared::S2CDataFB::GameStateDeltaFB: {
            spdlog::debug("Received GameStateDeltaFB");

            const auto* gameStateDelta =
                s2cMessage->message_as_GameStateDeltaFB();

            if (gameStateDelta->entities() != nullptr) {
              for (const auto* entityFB : *gameStateDelta->entities()) {
                EntityUpdatedEvent updateEvent;
                updateEvent.serverEntityId = entityFB->id();

                if (entityFB->position() != nullptr) {
                  updateEvent.position.x = entityFB->position()->x();
                  updateEvent.position.y = entityFB->position()->y();
                }

                InboundEvent event;
                event.event = updateEvent;
                inboundEvents_->Push(event);
              }
            }

            if (gameStateDelta->deleted_entity_ids() != nullptr) {
              for (uint32_t deletedId : *gameStateDelta->deleted_entity_ids()) {
                EntityDeletedEvent deleteEvent{};
                deleteEvent.serverEntityId = deletedId;

                InboundEvent event;
                event.event = deleteEvent;
                inboundEvents_->Push(event);
              }
            }

            break;
          }
          case lol_at_home_shared::S2CDataFB::PlayerAssignmentFB: {
            spdlog::debug("Received PlayerAssignmentFB");

            const auto* assignment =
                s2cMessage->message_as_PlayerAssignmentFB();

            PlayerAssignedEvent assignEvent{};
            assignEvent.myEntityId = assignment->assigned_entity();

            InboundEvent event;
            event.event = assignEvent;
            inboundEvents_->Push(event);

            break;
          }
          case lol_at_home_shared::S2CDataFB::ChatBroadcastFB:
            break;
          case lol_at_home_shared::S2CDataFB::NONE: {
            spdlog::warn("Received empty or unknown message type");
            break;
          }
        }

        enet_packet_destroy(event.packet);
        break;
      }

      case ENET_EVENT_TYPE_DISCONNECT:
        spdlog::info("Server disconnected");
        connected_ = false;
        serverPeer_ = nullptr;
        break;

      default:
        break;
    }
  }

  pushOutbound();
}

void NetworkClient::pushOutbound() {
  std::queue<OutboundEvent> outbound = outboundEvents_->PopAll();
  while (!outbound.empty()) {
    auto& event = outbound.front();

    ENetPacket* packet =
        enet_packet_create(event.c2sMessage.data(), event.c2sMessage.size(),
                           ENET_PACKET_FLAG_RELIABLE);

    enet_peer_send(serverPeer_, 0, packet);
    outbound.pop();
  }

  enet_host_flush(client_);
}

void NetworkClient::sendChampionSelect() {
  flatbuffers::FlatBufferBuilder builder;

  auto championSelect = lol_at_home_shared::CreateChampionSelectFB(
      builder, lol_at_home_shared::ChampionIdFB::Garen,
      lol_at_home_shared::TeamColorFB::Blue);

  auto c2sMessage = lol_at_home_shared::CreateC2SMessageFB(
      builder, lol_at_home_shared::C2SDataFB::ChampionSelectFB,
      championSelect.Union());

  builder.Finish(c2sMessage);

  ENetPacket* packet = enet_packet_create(
      builder.GetBufferPointer(), builder.GetSize(), ENET_PACKET_FLAG_RELIABLE);

  enet_peer_send(serverPeer_, 0, packet);
  enet_host_flush(client_);

  spdlog::info("Sent champion select");
}

}  // namespace lol_at_home_game
