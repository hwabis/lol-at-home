#pragma once

#include <cstdint>

namespace lah::game {

// keep in mind all structs here should be client-only.
// stuff server should also see goes in lah::shared

// Tagged on the entity that the player controls
struct LocalPlayer {
  uint32_t serverEntityId;
};

// Tagged on all entities that are tracked server-sid
struct ServerEntityId {
  uint32_t id;
};

}  // namespace lah::game
