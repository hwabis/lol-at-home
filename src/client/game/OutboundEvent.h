#pragma once

#include <cstddef>
#include <vector>

namespace lol_at_home_game {

struct OutboundEvent {
  std::vector<std::byte> c2sMessage;
};

}  // namespace lol_at_home_game
