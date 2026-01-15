#pragma once

#include <enet/enet.h>
#include <entt/entt.hpp>
#include "InboundEvent.h"
#include "OutboundEvent.h"
#include "ThreadSafeQueue.h"
#include "game_common_generated.h"

namespace lah::game {

class NetworkClient {
 public:
  explicit NetworkClient(
      std::shared_ptr<ThreadSafeQueue<InboundEvent>> inboundEvents,
      std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outboundEvents);
  ~NetworkClient();

  NetworkClient(const NetworkClient&) = delete;
  auto operator=(const NetworkClient&) -> NetworkClient& = delete;
  NetworkClient(NetworkClient&&) = default;
  auto operator=(NetworkClient&&) -> NetworkClient& = default;

  auto Connect(const char* host, uint16_t port) -> bool;
  auto Disconnect() -> void;
  auto Poll() -> void;

 private:
  auto pushOutbound() -> void;
  auto sendChampionSelect(lah_shared::TeamColorFB team) -> void;

  ENetHost* client_ = nullptr;
  ENetPeer* serverPeer_ = nullptr;
  bool connected_ = false;

  std::shared_ptr<ThreadSafeQueue<InboundEvent>> inboundEvents_;
  std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outboundEvents_;
};

}  // namespace lah::game
