#pragma once

#include <memory>
#include "core/InboundEvent.h"
#include "core/OutboundEvent.h"
#include "util/ThreadSafeQueue.h"

namespace lah::server {

class Server {
 public:
  void Run();

 private:
  std::shared_ptr<ThreadSafeQueue<InboundEvent>> incoming_ =
      std::make_shared<ThreadSafeQueue<InboundEvent>>();
  std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outgoing_ =
      std::make_shared<ThreadSafeQueue<OutboundEvent>>();
};

}  // namespace lah::server
