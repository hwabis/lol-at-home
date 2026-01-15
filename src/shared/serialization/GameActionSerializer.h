#pragma once

#include <optional>
#include "domain/GameAction.h"
#include "game_actions_generated.h"

namespace lah::shared {

class GameActionSerializer {
 public:
  GameActionSerializer() = delete;

  // Called on client
  static auto Serialize(flatbuffers::FlatBufferBuilder& builder,
                        const GameActionVariant& action)
      -> flatbuffers::Offset<lah_shared::GameActionFB>;

  // Called on server
  static auto Deserialize(const lah_shared::GameActionFB& action)
      -> std::optional<GameActionVariant>;
};

}  // namespace lah::shared
