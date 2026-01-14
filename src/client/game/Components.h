#pragma once

#include <cstdint>

namespace lol_at_home_game {

// keep in mind all structs here should be client-only.
// stuff server should also see goes in lol_at_home_shared

struct LocalPlayer {
  uint32_t serverEntityId;
};

}  // namespace lol_at_home_game
