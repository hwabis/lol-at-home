#pragma once

#include <enet/enet.h>
#include <functional>
#include <mutex>
#include "GameAction.h"

namespace lol_at_home_server {

class EnetNetworkManager {
 public:
  EnetNetworkManager();
  ~EnetNetworkManager();
  EnetNetworkManager(const EnetNetworkManager&) = delete;
  auto operator=(const EnetNetworkManager&) -> EnetNetworkManager& = delete;
  EnetNetworkManager(EnetNetworkManager&&) = delete;
  auto operator=(EnetNetworkManager&&) -> EnetNetworkManager& = delete;

  void Start(std::function<void(const lol_at_home_shared::GameActionVariant&)>
                 onActionReceived,
             std::function<entt::entity()> onClientConnected);
  void Stop();
  void Send(const entt::registry& registry,
            const std::vector<entt::entity>& entities);

 private:
  void runNetworkLoop();
  void handleIncoming();
  void sendQueue();
  static auto serialize(const entt::registry& registry,
                        const std::vector<entt::entity>& entities)
      -> std::vector<std::byte>;

  std::jthread networkThread_;
  std::atomic<bool> isRunning_ = false;
  std::function<void(const lol_at_home_shared::GameActionVariant&)>
      onActionReceived_;
  std::vector<std::vector<std::byte>> outgoingBytes_;
  std::mutex outgoingMutex_;
  ENetHost* host_;
  std::vector<ENetPeer*> peers_;
  std::mutex peersMutex_;
  std::function<entt::entity()> onClientConnected_;
};

}  // namespace lol_at_home_server
