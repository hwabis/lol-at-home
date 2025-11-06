#include "EnetNetworkManager.h"
#include <spdlog/spdlog.h>
#include "GameActionSerializer.h"
#include "GameStateSerializer.h"
#include "NetworkChannels.h"
#include "PlayerAssignment.h"
#include "PlayerAssignmentSerializer.h"

namespace lol_at_home_server {

EnetNetworkManager::EnetNetworkManager() {
  enet_initialize();
  ENetAddress address{.host = ENET_HOST_ANY, .port = 12345};
  host_ = enet_host_create(&address, 32, 2, 0, 0);
}

EnetNetworkManager::~EnetNetworkManager() {
  Stop();
}

void EnetNetworkManager::Start(
    std::function<void(const lol_at_home_shared::GameActionVariant&)>
        onActionReceived,
    std::function<entt::entity()> onClientConnected,
    std::function<std::vector<std::byte>()> onGetFullState) {
  isRunning_ = true;
  onActionReceived_ = std::move(onActionReceived);
  onClientConnected_ = std::move(onClientConnected);
  onGetFullState_ = std::move(onGetFullState);
  networkThread_ = std::jthread([this] { runNetworkLoop(); });
}

void EnetNetworkManager::Stop() {
  isRunning_ = false;
  if (host_ != nullptr) {
    enet_host_destroy(host_);
    host_ = nullptr;
  }
  enet_deinitialize();
}

void EnetNetworkManager::Send(const entt::registry& registry,
                              const std::vector<entt::entity>& entities) {
  if (!isRunning_) {
    spdlog::warn("what are you doing bruh");
    return;
  }

  auto bytes = serialize(registry, entities);

  std::lock_guard lock(outgoingMutex_);
  outgoingBytes_.push_back(bytes);
}

void EnetNetworkManager::runNetworkLoop() {
  while (isRunning_) {
    handleIncoming();
    sendQueue();

    constexpr int sleepMs = 10;
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
  }
}

void EnetNetworkManager::handleIncoming() {
  ENetEvent event;
  while (enet_host_service(host_, &event, 1) > 0) {
    switch (event.type) {
      case ENET_EVENT_TYPE_CONNECT: {
        spdlog::info("Client connected");

        auto entity = onClientConnected_();

        lol_at_home_shared::PlayerAssignment assignment{entity};
        auto assignmentBytes =
            lol_at_home_shared::PlayerAssignmentSerializer::Serialize(
                assignment);
        ENetPacket* assignmentPacket =
            enet_packet_create(assignmentBytes.data(), assignmentBytes.size(),
                               ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(event.peer, lol_at_home_shared::NetworkChannels::Control,
                       assignmentPacket);

        auto stateBytes = onGetFullState_();
        ENetPacket* statePacket = enet_packet_create(
            stateBytes.data(), stateBytes.size(), ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(event.peer,
                       lol_at_home_shared::NetworkChannels::GameState,
                       statePacket);

        enet_host_flush(host_);

        {
          std::lock_guard lock(peersMutex_);
          peers_.push_back(event.peer);
        }

        break;
      }

      case ENET_EVENT_TYPE_RECEIVE: {
        spdlog::info("Received " + std::to_string(event.packet->dataLength) +
                     " bytes from client");

        try {
          std::vector<std::byte> data(
              reinterpret_cast<const std::byte*>(event.packet->data),
              reinterpret_cast<const std::byte*>(event.packet->data) +
                  event.packet->dataLength);
          auto action =
              lol_at_home_shared::GameActionSerializer::Deserialize(data);
          spdlog::info("Successfully deserialized action");

          onActionReceived_(action);
        } catch (const std::exception& e) {
          spdlog::error("Failed to deserialize action: " +
                        std::string(e.what()));
        }

        enet_packet_destroy(event.packet);
        break;
      }

      case ENET_EVENT_TYPE_DISCONNECT: {
        spdlog::info("Client disconnected");
        {
          std::lock_guard lock(peersMutex_);
          std::erase(peers_, event.peer);
        }
        break;
      }

      default:
        break;
    }
  }
}

void EnetNetworkManager::sendQueue() {
  std::lock_guard lock(outgoingMutex_);
  for (const auto& bytes : outgoingBytes_) {
    std::lock_guard lock(peersMutex_);
    for (auto* peer : peers_) {
      ENetPacket* packet = enet_packet_create(bytes.data(), bytes.size(),
                                              ENET_PACKET_FLAG_RELIABLE);
      enet_peer_send(peer, 0, packet);
    }
  }

  if (!outgoingBytes_.empty()) {
    enet_host_flush(host_);
  }

  outgoingBytes_.clear();
}

auto EnetNetworkManager::serialize(const entt::registry& registry,
                                   const std::vector<entt::entity>& entities)
    -> std::vector<std::byte> {
  return lol_at_home_shared::GameStateSerializer::Serialize(registry, entities);
}

}  // namespace lol_at_home_server
