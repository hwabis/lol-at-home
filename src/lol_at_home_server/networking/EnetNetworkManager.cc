#include "EnetNetworkManager.h"
#include <enet/enet.h>
#include <spdlog/spdlog.h>

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
    std::function<void(const GameActionVariant&)> onActionReceived) {
  isRunning_ = true;
  onActionReceived_ = std::move(onActionReceived);
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

    std::this_thread::sleep_for(
        std::chrono::milliseconds(10));  // todo configurable
  }
}

void EnetNetworkManager::handleIncoming() {
  ENetEvent event;
  while (enet_host_service(host_, &event, 1) > 0) {
    switch (event.type) {
      case ENET_EVENT_TYPE_CONNECT: {
        spdlog::info("Client connected");
        std::lock_guard lock(peersMutex_);
        peers_.push_back(event.peer);
        break;
      }

      case ENET_EVENT_TYPE_RECEIVE: {
        GameActionVariant action;
        // todo deserialize event.packet->data into action
        onActionReceived_(action);

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
  // If entities is empty, serialize all entities
  // Otherwise serialize only the specified entities
  // Extract Position, Health, etc. components and pack into bytes
}

}  // namespace lol_at_home_server
