#pragma once

#include <flatbuffers/flatbuffers.h>
#include "OutboundEvent.h"

namespace lol_at_home_server {

class OutboundEventVisitor {
 public:
  OutboundEventVisitor(const entt::registry* registry,
                       flatbuffers::FlatBufferBuilder* builder);

  void operator()(const SendGameStateEvent& event) const;
  void operator()(const SendPlayerAssignmentEvent& event) const;
  void operator()(const BroadcastChatEvent& event) const;

 private:
  const entt::registry* registry_;
  flatbuffers::FlatBufferBuilder* builder_;
};

}  // namespace lol_at_home_server
