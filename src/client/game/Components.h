#pragma once

#include <cstdint>

namespace lah::game {

// keep in mind all structs here should be client-only.
// stuff server should also see goes in lah::shared

struct LocalPlayer {
  uint32_t serverEntityId;
};

}  // namespace lah::game
