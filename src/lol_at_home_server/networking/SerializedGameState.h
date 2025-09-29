#pragma once

#include <cstddef>
#include <vector>

namespace lol_at_home_server {

enum class MessageType : uint8_t { OUT_FULL_STATE, OUT_DELTA_STATE, IN_ACTION };

struct SerializedGameState {
  std::vector<std::byte> Data;
  MessageType Type{};
};

}  // namespace lol_at_home_server
