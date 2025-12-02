#include "GameplayScene.h"
#include <spdlog/spdlog.h>
#include "ClientComponents.h"
#include "Renderer.h"
#include "c2s_message_generated.h"
#include "domain/EcsComponents.h"
#include "game_actions_generated.h"
#include "s2c_message_generated.h"

namespace lol_at_home_game {

GameplayScene::GameplayScene(std::shared_ptr<lol_at_home_engine::Camera> camera,
                             lol_at_home_engine::Input& input,
                             lol_at_home_engine::Renderer& renderer)
    : camera_(std::move(camera)), input_(input), renderer_(renderer) {}

GameplayScene::~GameplayScene() {
  if (client_ != nullptr) {
    enet_host_destroy(client_);
  }
  enet_deinitialize();
}

void GameplayScene::OnStart() {
  spdlog::info("GameplayScene started");

  if (enet_initialize() != 0) {
    spdlog::error("Failed to initialize ENet");
    return;
  }

  connectToServer();

  camera_->SetPosition({.x = 100.0, .y = 200.0});
  camera_->SetZoom(1.0F);
}

void GameplayScene::Update(double deltaTime) {
  handleInput();
  handleNetwork();
  updateInterpolation(deltaTime);
}

void GameplayScene::Render(lol_at_home_engine::Renderer& renderer) {
  renderEntities(renderer);
}

void GameplayScene::OnEnd() {
  if (peer_ != nullptr && connected_) {
    enet_peer_disconnect(peer_, 0);

    ENetEvent event;
    while (enet_host_service(client_, &event, 3000) > 0) {
      if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
        break;
      }
    }
  }

  spdlog::info("GameplayScene ended");
}

auto GameplayScene::ShouldContinue() -> bool {
  return connected_;
}

void GameplayScene::connectToServer() {
  client_ = enet_host_create(nullptr, 1, 2, 0, 0);
  if (client_ == nullptr) {
    spdlog::error("Failed to create ENet client");
    return;
  }

  ENetAddress address;
  enet_address_set_host(&address, "127.0.0.1");
  address.port = 12345;

  peer_ = enet_host_connect(client_, &address, 2, 0);
  if (peer_ == nullptr) {
    spdlog::error("No available peers for connection");
    return;
  }

  ENetEvent event;
  if (enet_host_service(client_, &event, 5000) > 0 &&
      event.type == ENET_EVENT_TYPE_CONNECT) {
    spdlog::info("Connected to server!");
    connected_ = true;
  } else {
    spdlog::error("Connection to server failed");
    enet_peer_reset(peer_);
    peer_ = nullptr;
  }
}

void GameplayScene::handleInput() {
  if (!connected_ || myEntityId_ == entt::null) {
    return;
  }

  if (input_.IsMouseButtonPressed(SDL_BUTTON_RIGHT)) {
    lol_at_home_engine::Vector2 screenPos = input_.GetMousePosition();
    lol_at_home_engine::Vector2 worldPos =
        camera_->ScreenToWorld(screenPos, renderer_.GetScreenSize());

    flatbuffers::FlatBufferBuilder builder(256);

    auto targetPos = lol_at_home_shared::PositionFB(worldPos.x, worldPos.y);
    auto moveAction =
        lol_at_home_shared::CreateMoveActionFB(builder, &targetPos);

    auto gameAction = lol_at_home_shared::CreateGameActionFB(
        builder, static_cast<uint32_t>(myEntityId_),
        lol_at_home_shared::GameActionDataFB::MoveActionFB, moveAction.Union());

    auto c2sMessage = lol_at_home_shared::CreateC2SMessageFB(
        builder, lol_at_home_shared::C2SDataFB::GameActionFB,
        gameAction.Union());

    builder.Finish(c2sMessage);

    ENetPacket* packet =
        enet_packet_create(builder.GetBufferPointer(), builder.GetSize(),
                           ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer_, 0, packet);
  }
}

void GameplayScene::handleNetwork() {
  if (!connected_) {
    return;
  }

  ENetEvent event;
  while (enet_host_service(client_, &event, 0) > 0) {
    switch (event.type) {
      case ENET_EVENT_TYPE_RECEIVE: {
        handleReceive(event);
        enet_packet_destroy(event.packet);
        break;
      }

      case ENET_EVENT_TYPE_DISCONNECT:
        spdlog::info("Disconnected from server");
        connected_ = false;
        break;

      case ENET_EVENT_TYPE_NONE:
      case ENET_EVENT_TYPE_CONNECT:
        break;
    }
  }
}

void GameplayScene::handleReceive(const ENetEvent& event) {
  std::vector<std::byte> data(
      reinterpret_cast<const std::byte*>(event.packet->data),
      reinterpret_cast<const std::byte*>(event.packet->data) +
          event.packet->dataLength);

  const auto* message = flatbuffers::GetRoot<lol_at_home_shared::S2CMessageFB>(
      reinterpret_cast<const uint8_t*>(data.data()));

  auto messageType = message->message_type();

  switch (messageType) {
    case lol_at_home_shared::S2CDataFB::PlayerAssignmentFB: {
      const auto* assignment = message->message_as_PlayerAssignmentFB();
      myEntityId_ = static_cast<entt::entity>(assignment->assigned_entity());
      spdlog::info("Assigned to entity: " +
                   std::to_string(assignment->assigned_entity()));
      break;
    }

    case lol_at_home_shared::S2CDataFB::GameStateDeltaFB: {
      const auto* delta = message->message_as_GameStateDeltaFB();

      if (delta->deleted_entity_ids() != nullptr) {
        for (auto entityId : *delta->deleted_entity_ids()) {
          auto entity = static_cast<entt::entity>(entityId);
          if (registry_.valid(entity)) {
            registry_.destroy(entity);
          }
        }
      }

      if (delta->entities() != nullptr) {
        spdlog::info("Updating entities");
        for (const auto* entityFB : *delta->entities()) {
          spdlog::info("Processing entity");

          auto entity = static_cast<entt::entity>(entityFB->id());

          if (!registry_.valid(entity)) {
            entity = registry_.create(entity);

            if (!registry_.valid(entity)) {
              spdlog::error("Couldn't create entity using hint");
              return;
            }
          }

          if (entityFB->position() != nullptr) {
            spdlog::info("Updating position");
            auto& pos =
                registry_.get_or_emplace<lol_at_home_shared::Position>(entity);
            pos.x = entityFB->position()->x();
            pos.y = entityFB->position()->y();
          }

          if (entityFB->health() != nullptr) {
            spdlog::info("Updating health");
            auto& health =
                registry_.get_or_emplace<lol_at_home_shared::Health>(entity);
            health.currentHealth = entityFB->health()->current_health();
            health.maxHealth = entityFB->health()->max_health();
            health.healthRegenPerSec =
                entityFB->health()->health_regen_per_sec();
          }

          if (entityFB->team() != nullptr) {
            spdlog::info("Updating team");
            auto& team =
                registry_.get_or_emplace<lol_at_home_shared::Team>(entity);
            switch (entityFB->team()->color()) {
              case lol_at_home_shared::TeamColorFB::Blue:
                team.teamColor = lol_at_home_shared::Team::Color::Blue;
                break;
              case lol_at_home_shared::TeamColorFB::Red:
                team.teamColor = lol_at_home_shared::Team::Color::Red;
                break;
            }
          }

          if (entityFB->movable() != nullptr) {
            spdlog::info("Updating movable");
            auto& movable =
                registry_.get_or_emplace<lol_at_home_shared::Movable>(entity);
            movable.speed = entityFB->movable()->speed();
            switch (entityFB->movable()->state()) {
              case lol_at_home_shared::MovementStateFB::Idle:
                movable.state = lol_at_home_shared::MovementState::Idle;
                break;
              case lol_at_home_shared::MovementStateFB::Moving:
                movable.state = lol_at_home_shared::MovementState::Moving;
                break;
            }
            movable.targetPosition.x = entityFB->movable()->target_pos().x();
            movable.targetPosition.y = entityFB->movable()->target_pos().y();
          }

          spdlog::info("Adding visual position");
          if (registry_.try_get<VisualPosition>(entity) == nullptr) {
            if (auto* pos =
                    registry_.try_get<lol_at_home_shared::Position>(entity)) {
              registry_.emplace<VisualPosition>(
                  entity, VisualPosition{.position = *pos,
                                         .velocity = {},
                                         .timeSinceLastUpdate = {}});
            }
          }

          spdlog::info("Entity processing complete");
        }
        spdlog::info("Updated entities successfully");
      }

      break;
    }

    case lol_at_home_shared::S2CDataFB::ChatBroadcastFB:
      // TODO: handle chat later
      break;

    default:
      spdlog::error("Unknown S2C message type");
      break;
  }
}

void GameplayScene::updateInterpolation(double deltaTime) {
  auto view = registry_.view<lol_at_home_shared::Position>();

  for (auto entity : view) {
    const auto& serverPos = view.get<lol_at_home_shared::Position>(entity);

    auto* visualPos = registry_.try_get<VisualPosition>(entity);
    if (visualPos == nullptr) {
      registry_.emplace<VisualPosition>(entity, serverPos);
      continue;
    }

    double dxToServer = serverPos.x - visualPos->position.x;
    double dyToServer = serverPos.y - visualPos->position.y;
    double distance =
        std::sqrt((dxToServer * dxToServer) + (dyToServer * dyToServer));

    if (distance < 0.5) {
      visualPos->position = serverPos;
      continue;
    }

    double speed = 0;
    if (auto* movable =
            registry_.try_get<lol_at_home_shared::Movable>(entity)) {
      speed = movable->speed;
    } else {
      visualPos->position = serverPos;
      continue;
    }

    double deltaSec = deltaTime / 1000.0;
    double moveDistance = speed * deltaSec;

    if (moveDistance >= distance) {
      visualPos->position = serverPos;
    } else {
      double ratio = moveDistance / distance;
      visualPos->position.x += dxToServer * ratio;
      visualPos->position.y += dyToServer * ratio;
    }
  }
}

void GameplayScene::renderEntities(lol_at_home_engine::Renderer& renderer) {
  auto view = registry_.view<lol_at_home_shared::Position>();

  for (auto entity : view) {
    lol_at_home_shared::Position renderPos;
    if (auto* visualPos = registry_.try_get<VisualPosition>(entity)) {
      renderPos = visualPos->position;
    } else {
      renderPos = view.get<lol_at_home_shared::Position>(entity);
    }

    lol_at_home_engine::Color color{.r = 100, .g = 200, .b = 255, .a = 255};

    if (auto* team = registry_.try_get<lol_at_home_shared::Team>(entity)) {
      color = team->teamColor == lol_at_home_shared::Team::Color::Blue
                  ? lol_at_home_engine::Color{.r = 100,
                                              .g = 150,
                                              .b = 255,
                                              .a = 255}
                  : lol_at_home_engine::Color{
                        .r = 255, .g = 100, .b = 100, .a = 255};
    }

    renderer.DrawCircle({.x = static_cast<float>(renderPos.x),
                         .y = static_cast<float>(renderPos.y)},
                        50.0, color);

    if (auto* health = registry_.try_get<lol_at_home_shared::Health>(entity)) {
      double healthPercent = health->currentHealth / health->maxHealth;
      lol_at_home_engine::Vector2 barPos{
          .x = static_cast<float>(renderPos.x - 30.0),
          .y = static_cast<float>(renderPos.y - 70.0)};
      lol_at_home_engine::Vector2 barSize{
          .x = static_cast<float>(60.0 * healthPercent), .y = 5.0F};

      renderer.DrawRect(barPos, barSize, {.r = 0, .g = 255, .b = 0, .a = 255});
    }

    if (auto* movable =
            registry_.try_get<lol_at_home_shared::Movable>(entity)) {
      if (movable->state == lol_at_home_shared::MovementState::Moving) {
        renderer.DrawCircle(
            {.x = static_cast<float>(movable->targetPosition.x),
             .y = static_cast<float>(movable->targetPosition.y)},
            10.0, {.r = 255, .g = 255, .b = 0, .a = 255});
        renderer.DrawLine({.x = static_cast<float>(renderPos.x),
                           .y = static_cast<float>(renderPos.y)},
                          {.x = static_cast<float>(movable->targetPosition.x),
                           .y = static_cast<float>(movable->targetPosition.y)},
                          {.r = 255, .g = 255, .b = 0, .a = 128});
      }
    }
  }
}

}  // namespace lol_at_home_game
