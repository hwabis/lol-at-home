#pragma once

#include <optional>
#include "domain/GameAction.h"
#include "game_actions_generated.h"

namespace lol_at_home_shared {

class GameActionSerializer {
 public:
  GameActionSerializer() = delete;

  // Called on client
  static auto Serialize(flatbuffers::FlatBufferBuilder& builder,
                        const GameActionVariant& action)
      -> flatbuffers::Offset<lol_at_home_shared::GameActionFB>;

  // Called on server
  static auto Deserialize(const lol_at_home_shared::GameActionFB& action)
      -> std::optional<GameActionVariant>;
};

}  // namespace lol_at_home_shared
