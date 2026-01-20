#pragma once

#include <cstddef>
#include <enet/enet.h>
#include <entt/entt.hpp>
#include <span>
#include "InboundEvent.h"
#include "OutboundEvent.h"
#include "ThreadSafeQueue.h"

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
  auto sendChampionSelect(lah::shared::Team::Color team) -> void;
  auto handleGameStateDelta(std::span<const std::byte> data) -> void;

  ENetHost* client_ = nullptr;
  ENetPeer* serverPeer_ = nullptr;
  bool connected_ = false;

  std::shared_ptr<ThreadSafeQueue<InboundEvent>> inboundEvents_;
  std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outboundEvents_;
};

}  // namespace lah::game
