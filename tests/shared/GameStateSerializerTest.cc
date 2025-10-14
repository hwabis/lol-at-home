#include <gtest/gtest.h>
#include <entt/entt.hpp>
#include "EcsComponents.h"
#include "GameStateSerializer.h"

using namespace lol_at_home_shared;

class GameStateSerializerTest : public ::testing::Test {
 protected:
  entt::registry registry;
};

namespace {

auto getEntityCount(const entt::registry& registry) -> size_t {
  std::unordered_set<entt::entity> entities;

  for (auto&& typeIndex : registry.storage()) {
    const auto& pool = typeIndex.second;
    for (auto entity : pool) {
      entities.insert(entity);
    }
  }

  return entities.size();
}

}  // namespace

TEST_F(GameStateSerializerTest, SerializeDeserializePosition) {
  auto entity = registry.create();
  registry.emplace<Position>(entity, 123.45, 678.90);

  auto bytes = GameStateSerializer::Serialize(registry, {});

  entt::registry newRegistry;
  GameStateSerializer::Deserialize(newRegistry, bytes);

  ASSERT_TRUE(newRegistry.valid(entity));
  auto* pos = newRegistry.try_get<Position>(entity);
  ASSERT_NE(pos, nullptr);
  EXPECT_DOUBLE_EQ(pos->X, 123.45);
  EXPECT_DOUBLE_EQ(pos->Y, 678.90);
}

TEST_F(GameStateSerializerTest, SerializeDeserializeHealth) {
  auto entity = registry.create();
  registry.emplace<Health>(entity, 85.5, 100.0, 5.0);

  auto bytes = GameStateSerializer::Serialize(registry, {});

  entt::registry newRegistry;
  GameStateSerializer::Deserialize(newRegistry, bytes);

  ASSERT_TRUE(newRegistry.valid(entity));
  auto* health = newRegistry.try_get<Health>(entity);
  ASSERT_NE(health, nullptr);
  EXPECT_DOUBLE_EQ(health->CurrentHealth, 85.5);
  EXPECT_DOUBLE_EQ(health->MaxHealth, 100.0);
  EXPECT_DOUBLE_EQ(health->HealthRegenPerSec, 5.0);
}

TEST_F(GameStateSerializerTest, SerializeDeserializeMana) {
  auto entity = registry.create();
  registry.emplace<Mana>(entity, 200.0, 300.0, 10.0);

  auto bytes = GameStateSerializer::Serialize(registry, {});

  entt::registry newRegistry;
  GameStateSerializer::Deserialize(newRegistry, bytes);

  ASSERT_TRUE(newRegistry.valid(entity));
  auto* mana = newRegistry.try_get<Mana>(entity);
  ASSERT_NE(mana, nullptr);
  EXPECT_DOUBLE_EQ(mana->Mana, 200.0);
  EXPECT_DOUBLE_EQ(mana->MaxMana, 300.0);
  EXPECT_DOUBLE_EQ(mana->ManaRegenPerSec, 10.0);
}

TEST_F(GameStateSerializerTest, SerializeDeserializeMovable) {
  auto entity = registry.create();
  registry.emplace<Movable>(entity, 350.0);

  auto bytes = GameStateSerializer::Serialize(registry, {});

  entt::registry newRegistry;
  GameStateSerializer::Deserialize(newRegistry, bytes);

  ASSERT_TRUE(newRegistry.valid(entity));
  auto* movable = newRegistry.try_get<Movable>(entity);
  ASSERT_NE(movable, nullptr);
  EXPECT_DOUBLE_EQ(movable->Speed, 350.0);
}

TEST_F(GameStateSerializerTest, SerializeDeserializeMoving) {
  auto entity = registry.create();
  registry.emplace<Moving>(entity, Position{500.0, 600.0});

  auto bytes = GameStateSerializer::Serialize(registry, {});

  entt::registry newRegistry;
  GameStateSerializer::Deserialize(newRegistry, bytes);

  ASSERT_TRUE(newRegistry.valid(entity));
  auto* moving = newRegistry.try_get<Moving>(entity);
  ASSERT_NE(moving, nullptr);
  EXPECT_DOUBLE_EQ(moving->TargetPosition.X, 500.0);
  EXPECT_DOUBLE_EQ(moving->TargetPosition.Y, 600.0);
}

TEST_F(GameStateSerializerTest, SerializeDeserializeTeam) {
  auto blueEntity = registry.create();
  registry.emplace<Team>(blueEntity, Team::Color::Blue);

  auto redEntity = registry.create();
  registry.emplace<Team>(redEntity, Team::Color::Red);

  auto bytes = GameStateSerializer::Serialize(registry, {});

  entt::registry newRegistry;
  GameStateSerializer::Deserialize(newRegistry, bytes);

  auto* blueTeam = newRegistry.try_get<Team>(blueEntity);
  ASSERT_NE(blueTeam, nullptr);
  EXPECT_EQ(blueTeam->TeamColor, Team::Color::Blue);

  auto* redTeam = newRegistry.try_get<Team>(redEntity);
  ASSERT_NE(redTeam, nullptr);
  EXPECT_EQ(redTeam->TeamColor, Team::Color::Red);
}

TEST_F(GameStateSerializerTest, SerializeDeserializeAbilities) {
  auto entity = registry.create();

  Abilities abilities;
  abilities.Abilities[AbilitySlot::Q] =
      Abilities::Ability{.Tag = AbilityTag::GAREN_Q,
                         .CooldownRemaining = 2.5f,
                         .Rank = 3,
                         .CurrentCharges = 1,
                         .MaxCharges = 1};
  abilities.Abilities[AbilitySlot::W] =
      Abilities::Ability{.Tag = AbilityTag::GAREN_W,
                         .CooldownRemaining = 0.0f,
                         .Rank = 5,
                         .CurrentCharges = 2,
                         .MaxCharges = 2};

  registry.emplace<Abilities>(entity, abilities);

  auto bytes = GameStateSerializer::Serialize(registry, {});

  entt::registry newRegistry;
  GameStateSerializer::Deserialize(newRegistry, bytes);

  auto* deserializedAbilities = newRegistry.try_get<Abilities>(entity);
  ASSERT_NE(deserializedAbilities, nullptr);
  ASSERT_EQ(deserializedAbilities->Abilities.size(), 2);

  auto& qAbility = deserializedAbilities->Abilities[AbilitySlot::Q];
  EXPECT_EQ(qAbility.Tag, AbilityTag::GAREN_Q);
  EXPECT_FLOAT_EQ(qAbility.CooldownRemaining, 2.5f);
  EXPECT_EQ(qAbility.Rank, 3);
  EXPECT_EQ(qAbility.CurrentCharges, 1);
  EXPECT_EQ(qAbility.MaxCharges, 1);

  auto& wAbility = deserializedAbilities->Abilities[AbilitySlot::W];
  EXPECT_EQ(wAbility.Tag, AbilityTag::GAREN_W);
  EXPECT_FLOAT_EQ(wAbility.CooldownRemaining, 0.0f);
  EXPECT_EQ(wAbility.Rank, 5);
  EXPECT_EQ(wAbility.CurrentCharges, 2);
  EXPECT_EQ(wAbility.MaxCharges, 2);
}

TEST_F(GameStateSerializerTest, SerializeDeserializeMultipleComponents) {
  auto entity = registry.create();
  registry.emplace<Position>(entity, 100.0, 200.0);
  registry.emplace<Health>(entity, 500.0, 600.0, 8.0);
  registry.emplace<Mana>(entity, 250.0, 400.0, 12.0);
  registry.emplace<Movable>(entity, 325.0);
  registry.emplace<Team>(entity, Team::Color::Red);

  auto bytes = GameStateSerializer::Serialize(registry, {});

  entt::registry newRegistry;
  GameStateSerializer::Deserialize(newRegistry, bytes);

  ASSERT_TRUE(newRegistry.valid(entity));

  auto* pos = newRegistry.try_get<Position>(entity);
  ASSERT_NE(pos, nullptr);
  EXPECT_DOUBLE_EQ(pos->X, 100.0);
  EXPECT_DOUBLE_EQ(pos->Y, 200.0);

  auto* health = newRegistry.try_get<Health>(entity);
  ASSERT_NE(health, nullptr);
  EXPECT_DOUBLE_EQ(health->CurrentHealth, 500.0);

  auto* mana = newRegistry.try_get<Mana>(entity);
  ASSERT_NE(mana, nullptr);
  EXPECT_DOUBLE_EQ(mana->Mana, 250.0);

  auto* movable = newRegistry.try_get<Movable>(entity);
  ASSERT_NE(movable, nullptr);
  EXPECT_DOUBLE_EQ(movable->Speed, 325.0);

  auto* team = newRegistry.try_get<Team>(entity);
  ASSERT_NE(team, nullptr);
  EXPECT_EQ(team->TeamColor, Team::Color::Red);
}

TEST_F(GameStateSerializerTest, SerializeDeserializeMultipleEntities) {
  auto entity1 = registry.create();
  registry.emplace<Position>(entity1, 10.0, 20.0);
  registry.emplace<Health>(entity1, 100.0, 100.0, 5.0);

  auto entity2 = registry.create();
  registry.emplace<Position>(entity2, 30.0, 40.0);
  registry.emplace<Mana>(entity2, 200.0, 200.0, 10.0);

  auto entity3 = registry.create();
  registry.emplace<Position>(entity3, 50.0, 60.0);

  auto bytes = GameStateSerializer::Serialize(registry, {});

  entt::registry newRegistry;
  GameStateSerializer::Deserialize(newRegistry, bytes);

  EXPECT_EQ(getEntityCount(registry), 3);

  auto* pos1 = newRegistry.try_get<Position>(entity1);
  ASSERT_NE(pos1, nullptr);
  EXPECT_DOUBLE_EQ(pos1->X, 10.0);

  auto* pos2 = newRegistry.try_get<Position>(entity2);
  ASSERT_NE(pos2, nullptr);
  EXPECT_DOUBLE_EQ(pos2->X, 30.0);

  auto* mana2 = newRegistry.try_get<Mana>(entity2);
  ASSERT_NE(mana2, nullptr);
  EXPECT_DOUBLE_EQ(mana2->Mana, 200.0);

  auto* pos3 = newRegistry.try_get<Position>(entity3);
  ASSERT_NE(pos3, nullptr);
  EXPECT_DOUBLE_EQ(pos3->X, 50.0);
}

TEST_F(GameStateSerializerTest, SerializeDeltaUpdateSpecificEntities) {
  auto entity1 = registry.create();
  registry.emplace<Position>(entity1, 10.0, 20.0);

  auto entity2 = registry.create();
  registry.emplace<Position>(entity2, 30.0, 40.0);

  auto entity3 = registry.create();
  registry.emplace<Position>(entity3, 50.0, 60.0);

  // Serialize only entity2
  auto bytes = GameStateSerializer::Serialize(registry, {entity2});

  entt::registry newRegistry;
  GameStateSerializer::Deserialize(newRegistry, bytes);

  // Only entity2 should be in the new registry
  EXPECT_EQ(getEntityCount(registry), 1);
  ASSERT_TRUE(newRegistry.valid(entity2));

  auto* pos = newRegistry.try_get<Position>(entity2);
  ASSERT_NE(pos, nullptr);
  EXPECT_DOUBLE_EQ(pos->X, 30.0);
  EXPECT_DOUBLE_EQ(pos->Y, 40.0);
}

TEST_F(GameStateSerializerTest, SerializeEmptyRegistry) {
  auto bytes = GameStateSerializer::Serialize(registry, {});

  entt::registry newRegistry;
  GameStateSerializer::Deserialize(newRegistry, bytes);

  EXPECT_EQ(getEntityCount(registry), 0);
}
