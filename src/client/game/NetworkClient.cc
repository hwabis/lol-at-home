#include "NetworkClient.h"
#include <spdlog/spdlog.h>

namespace lol_at_home_game {

NetworkClient::NetworkClient(
    std::shared_ptr<ThreadSafeQueue<InboundEvent>> inboundEvents,
    std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outboundEvents)
    : inboundEvents_(std::move(inboundEvents)),
      outboundEvents_(std::move(outboundEvents)) {
  if (enet_initialize() != 0) {
    spdlog::error("Failed to initialize ENet");
    throw std::runtime_error("ENet initialization failed");
  }

  client_ = enet_host_create(nullptr, 1, 1, 0, 0);

  if (client_ == nullptr) {
    spdlog::error("Failed to create ENet client host");
    enet_deinitialize();
    throw std::runtime_error("Failed to create ENet client");
  }
}

NetworkClient::~NetworkClient() {
  Disconnect();

  if (client_ != nullptr) {
    enet_host_destroy(client_);
  }

  enet_deinitialize();
}

auto NetworkClient::Connect(const char* host, uint16_t port) -> bool {
  if (connected_) {
    spdlog::warn("Already connected");
    return true;
  }

  ENetAddress address;
  enet_address_set_host(&address, host);
  address.port = port;
  serverPeer_ = enet_host_connect(client_, &address, 2, 0);

  if (serverPeer_ == nullptr) {
    spdlog::error("No available peers for initiating connection");
    return false;
  }

  ENetEvent event;
  if (enet_host_service(client_, &event, 5000) > 0 &&
      event.type == ENET_EVENT_TYPE_CONNECT) {
    spdlog::info("Connected to server");
    connected_ = true;
    return true;
  }

  spdlog::error("Connection to server failed");
  enet_peer_reset(serverPeer_);
  serverPeer_ = nullptr;
  return false;
}

void NetworkClient::Disconnect() {
  if (!connected_ || serverPeer_ == nullptr) {
    return;
  }

  enet_peer_disconnect(serverPeer_, 0);

  ENetEvent event;
  bool disconnected = false;

  while (enet_host_service(client_, &event, 3000) > 0) {
    switch (event.type) {
      case ENET_EVENT_TYPE_RECEIVE:
        enet_packet_destroy(event.packet);
        break;
      case ENET_EVENT_TYPE_DISCONNECT:
        spdlog::info("Disconnected from server");
        disconnected = true;
        break;
      default:
        break;
    }

    if (disconnected) {
      break;
    }
  }

  if (!disconnected) {
    enet_peer_reset(serverPeer_);
  }

  serverPeer_ = nullptr;
  connected_ = false;
}

void NetworkClient::Poll() {
  if (client_ == nullptr) {
    return;
  }

  ENetEvent event;

  while (enet_host_service(client_, &event, 0) > 0) {
    switch (event.type) {
      case ENET_EVENT_TYPE_RECEIVE: {
        std::vector<std::byte> data(
            reinterpret_cast<std::byte*>(event.packet->data),
            reinterpret_cast<std::byte*>(event.packet->data) +
                event.packet->dataLength);

        // todo push stuff to inboundEvents_

        enet_packet_destroy(event.packet);
        break;
      }

      case ENET_EVENT_TYPE_DISCONNECT:
        spdlog::info("Server disconnected");
        connected_ = false;
        serverPeer_ = nullptr;
        break;

      default:
        break;
    }
  }

  // todo send outbound events
}

}  // namespace lol_at_home_game
