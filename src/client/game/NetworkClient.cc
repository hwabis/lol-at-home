#include "NetworkClient.h"
#include <spdlog/spdlog.h>
#include <random>
#include "serialization/C2SMessageSerializer.h"
#include "serialization/S2CMessageSerializer.h"

namespace lah::game {

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

    // lmaooo todo be able to choose champ/team later, through ui or smth. for
    // now let's make it completely random.. if you wanna switch teams just
    // relaunch the game xDD
    std::mt19937 rng{std::random_device{}()};
    auto team = std::uniform_int_distribution<>(0, 1)(rng) == 0
                    ? lah::shared::Team::Color::Blue
                    : lah::shared::Team::Color::Red;
    sendChampionSelect(team);

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
        std::span<const std::byte> data(
            reinterpret_cast<const std::byte*>(event.packet->data),
            event.packet->dataLength);

        switch (lah::shared::S2CMessageSerializer::GetMessageType(data)) {
          case lah::shared::S2CMessageType::GameStateDelta: {
            spdlog::debug("Received GameStateDelta");
            handleGameStateDelta(data);
            break;
          }
          case lah::shared::S2CMessageType::PlayerAssignment: {
            spdlog::debug("Received PlayerAssignment");
            if (auto assignment = lah::shared::S2CMessageSerializer::
                    DeserializePlayerAssignment(data)) {
              PlayerAssignedEvent assignEvent{};
              assignEvent.myEntityId = assignment->assignedEntityId;
              inboundEvents_->Push(InboundEvent{.event = assignEvent});
            }
            break;
          }
          case lah::shared::S2CMessageType::ChatBroadcast:
            // TODO: handle chat
            break;
          case lah::shared::S2CMessageType::Unknown: {
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

void NetworkClient::sendChampionSelect(lah::shared::Team::Color team) {
  auto data = lah::shared::C2SMessageSerializer::SerializeChampionSelect(
      lah::shared::ChampionId::Garen, team);

  ENetPacket* packet =
      enet_packet_create(data.data(), data.size(), ENET_PACKET_FLAG_RELIABLE);

  enet_peer_send(serverPeer_, 0, packet);
  enet_host_flush(client_);

  spdlog::info("Sent champion select");
}

void NetworkClient::handleGameStateDelta(std::span<const std::byte> data) {
  auto deltaData =
      lah::shared::S2CMessageSerializer::DeserializeGameStateDelta(data);

  if (!deltaData) {
    return;
  }

  for (const auto& entity : deltaData->entities) {
    inboundEvents_->Push(InboundEvent{.event = entity});
  }

  for (uint32_t deletedId : deltaData->deletedEntityIds) {
    EntityDeletedEvent deleteEvent{.entityId = deletedId};
    inboundEvents_->Push(InboundEvent{.event = deleteEvent});
  }
}

}  // namespace lah::game
