#pragma once

#include <enet/enet.h>
#include <functional>
#include <mutex>
#include "actions/GameAction.h"
#include "networking/SerializedGameState.h"

namespace lol_at_home_server {

class EnetNetworkManager {
 public:
  EnetNetworkManager();
  ~EnetNetworkManager();
  EnetNetworkManager(const EnetNetworkManager&) = delete;
  auto operator=(const EnetNetworkManager&) -> EnetNetworkManager& = delete;
  EnetNetworkManager(EnetNetworkManager&&) = delete;
  auto operator=(EnetNetworkManager&&) -> EnetNetworkManager& = delete;

  void Start(std::function<void(const GameActionVariant&)> onActionReceived);
  void Stop();
  void Send(const SerializedGameState& state);

 private:
  void runNetworkLoop();
  void handleIncoming();
  void sendQueue();

  std::jthread networkThread_;
  std::atomic<bool> isRunning_ = false;
  std::function<void(const GameActionVariant&)> onActionReceived_;
  std::vector<SerializedGameState> outgoingStates_;
  std::mutex outgoingMutex_;
  ENetHost* host_;
  std::vector<ENetPeer*> peers_;
  std::mutex peersMutex_;
};

}  // namespace lol_at_home_server
