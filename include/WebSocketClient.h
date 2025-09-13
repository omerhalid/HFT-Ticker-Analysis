/**
 * @file WebSocketClient.h
 * @brief WebSocket client for Coinbase ticker data using libwebsockets
 */

#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <libwebsockets.h>
#include <iostream>
#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

/**
 * @class WebSocketClient
 * @brief WebSocket client implementation using libwebsockets
 * 
 * This class provides a reliable WebSocket client for connecting to
 * Coinbase's WebSocket feed and handling real-time ticker data.
 */
class WebSocketClient {
public:
    using MessageCallback = std::function<void(const std::string&)>;

    /**
     * @brief Constructor
     */
    WebSocketClient();

    /**
     * @brief Destructor
     */
    ~WebSocketClient();

    /**
     * @brief Set message callback function
     * @param callback Function to call when a message is received
     */
    void setMessageCallback(MessageCallback callback);

    /**
     * @brief Connect to WebSocket server
     * @param uri WebSocket URI (e.g., "wss://ws-feed.exchange.coinbase.com")
     * @return true if connection successful, false otherwise
     */
    bool connect(const std::string& uri);

    /**
     * @brief Disconnect from WebSocket server
     */
    void disconnect();

    /**
     * @brief Check if connected
     * @return true if connected, false otherwise
     */
    bool isConnected() const;

    /**
     * @brief Check if running
     * @return true if running, false otherwise
     */
    bool isRunning() const;

    /**
     * @brief Send message to server
     * @param message Message to send
     * @return true if sent successfully, false otherwise
     */
    bool sendMessage(const std::string& message);

    /**
     * @brief Subscribe to ticker channel
     * @param productId Product ID to subscribe to (e.g., "BTC-USD")
     * @return true if subscription successful, false otherwise
     */
    bool subscribeToTicker(const std::string& productId);

    /**
     * @brief Static callback for libwebsockets
     */
    static int callback(struct lws* wsi, enum lws_callback_reasons reason,
                       void* user, void* in, size_t len);

private:
    struct lws_context* m_context;
    struct lws* m_wsi;
    std::thread m_ioThread;
    std::atomic<bool> m_connected;
    std::atomic<bool> m_running;
    MessageCallback m_messageCallback;
    std::mutex m_sendMutex;
    std::string m_pendingMessage;

    /**
     * @brief I/O thread function
     */
    void runIO();

    /**
     * @brief Get instance from user data
     */
    static WebSocketClient* getInstance(void* user);
};

#endif // WEBSOCKETCLIENT_H