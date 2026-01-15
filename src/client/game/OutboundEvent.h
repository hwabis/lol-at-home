#pragma once

#include <cstddef>
#include <vector>

namespace lah::game {

struct OutboundEvent {
  std::vector<std::byte> c2sMessage;
};

}  // namespace lah::game
