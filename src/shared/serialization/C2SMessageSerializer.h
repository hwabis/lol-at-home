#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <vector>
#include "domain/ChampionId.h"
#include "domain/EcsComponents.h"
#include "domain/GameAction.h"

namespace lah::shared {

struct ChampionSelectData {
  ChampionId championId;
  Team::Color teamColor;
};

enum class C2SMessageType : uint8_t {
  GameAction,
  ChampionSelect,
  ChatMessage,
  Unknown,
};

class C2SMessageSerializer {
 public:
  C2SMessageSerializer() = delete;

  static auto GetMessageType(std::span<const std::byte> data) -> C2SMessageType;

  static auto SerializeGameAction(const GameActionVariant& action)
      -> std::vector<std::byte>;

  static auto SerializeChampionSelect(ChampionId champion, Team::Color team)
      -> std::vector<std::byte>;

  static auto DeserializeGameAction(std::span<const std::byte> data)
      -> std::optional<GameActionVariant>;

  static auto DeserializeChampionSelect(std::span<const std::byte> data)
      -> std::optional<ChampionSelectData>;
};

}  // namespace lah::shared
