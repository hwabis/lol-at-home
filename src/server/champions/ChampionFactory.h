#pragma once

#include <entt/entt.hpp>
#include "domain/ChampionId.h"
#include "domain/EcsComponents.h"

namespace lol_at_home_server {

class ChampionFactory {
 public:
  ChampionFactory() = delete;

  static auto CreateChampion(entt::registry& registry,
                             lol_at_home_shared::ChampionId championId)
      -> entt::entity {
    // todo probably wanna split each champ into own class
    switch (championId) {
      case lol_at_home_shared::ChampionId::Garen:
        return createGaren(registry);
      case lol_at_home_shared::ChampionId::Teemo:
        // todo
        return entt::entity(0);
    }
  }

 private:
  static auto createGaren(entt::registry& registry) -> entt::entity {
    using namespace lol_at_home_shared;

    entt::entity entity = registry.create();

    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,
    // readability-magic-numbers)

    registry.emplace<Position>(entity, 100.0, 200.0);
    registry.emplace<Health>(entity, 616.0, 616.0, 8.0);
    // registry.emplace<Mana>(entity, 0.0, 0.0, 0.0);  // Garen has no mana
    registry.emplace<Movable>(entity, 340.0);
    registry.emplace<Team>(entity,
                           Team::Color::Blue);  // todo assign based on team

    Abilities abilities;
    abilities.abilities[AbilitySlot::Q] = {.tag = AbilityTag::Garen_Q};
    abilities.abilities[AbilitySlot::W] = {.tag = AbilityTag::Garen_W};
    abilities.abilities[AbilitySlot::E] = {.tag = AbilityTag::Garen_E};
    abilities.abilities[AbilitySlot::R] = {.tag = AbilityTag::Garen_R};
    registry.emplace<Abilities>(entity, abilities);
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers,
    // readability-magic-numbers)

    return entity;
  }
};

}  // namespace lol_at_home_server
