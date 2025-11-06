#pragma once

#include <memory>
#include "GameAction.h"
#include "GameState.h"
#include "OutboundPacket.h"
#include "util/ThreadSafeQueue.h"

namespace lol_at_home_server {

class Server {
 public:
  void Run();

 private:
  std::shared_ptr<ThreadSafeQueue<InboundPacket>> incoming_ =
      std::make_shared<ThreadSafeQueue<InboundPacket>>();
  std::shared_ptr<ThreadSafeQueue<OutboundPacket>> outgoing_ =
      std::make_shared<ThreadSafeQueue<OutboundPacket>>();
};

}  // namespace lol_at_home_server
