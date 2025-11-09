#include "PlayerAssignmentSerializer.h"

namespace lol_at_home_shared {

auto PlayerAssignmentSerializer::Serialize(const PlayerAssignment& assignment)
    -> std::vector<std::byte> {
  std::vector<std::byte> bytes(sizeof(uint32_t));

  auto entityId = static_cast<uint32_t>(assignment.AssignedEntity);
  std::memcpy(bytes.data(), &entityId, sizeof(uint32_t));

  return bytes;
}

auto PlayerAssignmentSerializer::Deserialize(const std::vector<std::byte>& data)
    -> PlayerAssignment {
  if (data.size() < sizeof(uint32_t)) {
    throw std::runtime_error("Invalid player assignment data");
  }

  uint32_t entityId = 0;
  std::memcpy(&entityId, data.data(), sizeof(uint32_t));

  return PlayerAssignment{.AssignedEntity =
                              static_cast<entt::entity>(entityId)};
}

}  // namespace lol_at_home_shared
