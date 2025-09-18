#include "Entity.h"
#include <atomic>

namespace lol_at_home_server {

Entity::Entity(Vector2 position) : position_(position) {
  static std::atomic<EntityId> newId{1};
  id_ = newId++;
}

}  // namespace lol_at_home_server
