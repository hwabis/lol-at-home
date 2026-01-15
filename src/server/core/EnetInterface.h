#pragma once

#include <enet/enet.h>
#include "InboundEvent.h"
#include "OutboundEvent.h"
#include "util/IPeriodic.h"
#include "util/ThreadSafeQueue.h"

namespace lah::server {

class EnetInterface : public IPeriodic {
 public:
  EnetInterface(std::shared_ptr<ThreadSafeQueue<InboundEvent>> inbound,
                std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outbound,
                uint16_t port);

  ~EnetInterface() override;

  EnetInterface(const EnetInterface&) = delete;
  auto operator=(const EnetInterface&) -> EnetInterface& = delete;
  EnetInterface(EnetInterface&&) = delete;
  auto operator=(EnetInterface&&) -> EnetInterface& = delete;

  auto Cycle(std::chrono::milliseconds timeElapsed) -> void override;

 private:
  auto sendOutbound() -> void;
  auto populateInbound() -> void;

  ENetHost* host_;
  std::shared_ptr<ThreadSafeQueue<InboundEvent>> inbound_;
  std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outbound_;
};

}  // namespace lah::server
