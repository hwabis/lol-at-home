#pragma once

#include <enet/enet.h>
#include "InboundPacket.h"
#include "OutboundPacket.h"
#include "util/IPeriodic.h"
#include "util/ThreadSafeQueue.h"

namespace lol_at_home_server {

class EnetInterface : public IPeriodic {
 public:
  EnetInterface(
      std::shared_ptr<ThreadSafeQueue<InboundPacket>> incomingActions,
      std::shared_ptr<ThreadSafeQueue<OutboundPacket>> outgoingStates);

  ~EnetInterface() override;

  EnetInterface(const EnetInterface&) = delete;
  auto operator=(const EnetInterface&) -> EnetInterface& = delete;
  EnetInterface(const EnetInterface&&) = delete;
  auto operator=(const EnetInterface&&) -> EnetInterface& = delete;

  auto Cycle(std::chrono::milliseconds timeElapsed) -> void override;

 private:
  auto sendOutbound() -> void;
  auto populateInbound() -> void;

  ENetHost* host_;
  std::shared_ptr<ThreadSafeQueue<InboundPacket>> inbound_;
  std::shared_ptr<ThreadSafeQueue<OutboundPacket>> outbound_;
};

}  // namespace lol_at_home_server
