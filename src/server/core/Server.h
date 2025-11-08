#pragma once

#include <memory>
#include "core/InboundEvent.h"
#include "core/OutboundPacket.h"
#include "util/ThreadSafeQueue.h"

namespace lol_at_home_server {

class Server {
 public:
  void Run();

 private:
  std::shared_ptr<ThreadSafeQueue<InboundEvent>> incoming_ =
      std::make_shared<ThreadSafeQueue<InboundEvent>>();
  std::shared_ptr<ThreadSafeQueue<OutboundPacket>> outgoing_ =
      std::make_shared<ThreadSafeQueue<OutboundPacket>>();
};

}  // namespace lol_at_home_server
