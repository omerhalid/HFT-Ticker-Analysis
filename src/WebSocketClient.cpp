/**
 * @file WebSocketClient.cpp
 * @brief Implementation of WebSocket client for Coinbase ticker data using libwebsockets
 */

#include "WebSocketClient.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <cstring>
#include "JSONParser.h"
#include "ThreadUtils.h"

// Static instance for callback access
static WebSocketClient* g_instance = nullptr;

// Protocol definition
static struct lws_protocols protocols[] = {
    {
        "coinbase-protocol",
        WebSocketClient::callback,
        0,
        4096,
    },
    { nullptr, nullptr, 0, 0 } // terminator
};

WebSocketClient::WebSocketClient() 
    : m_context(nullptr)
    , m_wsi(nullptr)
    , m_connected(false)
    , m_running(false) {
    g_instance = this;
}

WebSocketClient::~WebSocketClient() {
    disconnect();
    g_instance = nullptr;
}

void WebSocketClient::setMessageCallback(MessageCallback callback) {
    m_messageCallback = callback;
}

bool WebSocketClient::connect(const std::string& uri) {
    // Parse URI
    std::string protocol, host, path;
    int port = 443;
    
    if (uri.find("wss://") == 0) {
        protocol = "wss";
        size_t start = 6; // length of "wss://"
        size_t slash = uri.find('/', start);
        if (slash == std::string::npos) {
            host = uri.substr(start);
            path = "/";
        } else {
            host = uri.substr(start, slash - start);
            path = uri.substr(slash);
        }
        
        // Check for port
        size_t colon = host.find(':');
        if (colon != std::string::npos) {
            port = std::stoi(host.substr(colon + 1));
            host = host.substr(0, colon);
        }
    } else {
        std::cerr << "Only wss:// URLs are supported" << std::endl;
        return false;
    }

    // Create libwebsockets context
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;
    info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    info.user = this;
    
    m_context = lws_create_context(&info);
    if (!m_context) {
        std::cerr << "Failed to create libwebsockets context" << std::endl;
        return false;
    }
    
    // Create WebSocket connection info
    struct lws_client_connect_info ccinfo;
    memset(&ccinfo, 0, sizeof(ccinfo));
    
    ccinfo.context = m_context;
    ccinfo.address = host.c_str();
    ccinfo.port = port;
    ccinfo.path = path.c_str();
    ccinfo.host = host.c_str();
    ccinfo.origin = host.c_str();
    ccinfo.protocol = "coinbase-protocol";
    ccinfo.ssl_connection = LCCSCF_USE_SSL;
    ccinfo.userdata = this;
    
    m_wsi = lws_client_connect_via_info(&ccinfo);
    if (!m_wsi) {
        std::cerr << "Failed to create WebSocket connection" << std::endl;
        lws_context_destroy(m_context);
        m_context = nullptr;
        return false;
    }
    
    m_running = true;
    
    // Start I/O thread
    m_ioThread = std::thread(&WebSocketClient::runIO, this);
    
    // Wait for connection to establish
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    return m_connected.load();
}

void WebSocketClient::disconnect() {
    m_running = false;
    m_connected = false;
    
    if (m_wsi) {
        lws_close_reason(m_wsi, LWS_CLOSE_STATUS_NORMAL, nullptr, 0);
        m_wsi = nullptr;
    }
    
    if (m_context) {
        lws_context_destroy(m_context);
        m_context = nullptr;
    }
    
    if (m_ioThread.joinable()) {
        m_ioThread.join();
    }
}

bool WebSocketClient::isConnected() const {
    return m_connected.load();
}

bool WebSocketClient::isRunning() const {
    return m_running.load();
}

bool WebSocketClient::sendMessage(const std::string& message) {
    if (!m_connected.load() || !m_wsi) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(m_sendMutex);
    m_pendingMessage = message;
    
    // Trigger write callback
    lws_callback_on_writable(m_wsi);
    
    return true;
}

bool WebSocketClient::subscribeToTicker(const std::string& productId) {
    std::string subscriptionMsg = JSONParser::createSubscriptionMessage(productId);
    return sendMessage(subscriptionMsg);
}

void WebSocketClient::runIO() {
    // Optimize thread for HFT
    ThreadUtils::optimizeForHFT("WebSocketIO", 1, 99);
    
    while (m_running.load()) {
        // Zero timeout for maximum responsiveness
        int result = lws_service(m_context, 0); // 0ms timeout for low latency
        if (result < 0) {
            std::cerr << "libwebsockets service error" << std::endl;
            break;
        }
        
        // Yield CPU if no work to prevent busy waiting
        if (result == 0) {
            std::this_thread::yield();
        }
    }
}

int WebSocketClient::callback(struct lws* wsi, enum lws_callback_reasons reason,
                              void* user, void* in, size_t len) {
    WebSocketClient* client = getInstance(user);
    if (!client) {
        return -1;
    }
    
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            std::cout << "WebSocket connection established" << std::endl;
            client->m_connected = true;
            break;
            
        case LWS_CALLBACK_CLIENT_RECEIVE:
            if (in && len > 0) {
                std::string message(static_cast<char*>(in), len);
                if (client->m_messageCallback) {
                    client->m_messageCallback(message);
                }
            }
            break;
            
        case LWS_CALLBACK_CLIENT_WRITEABLE:
            {
                std::lock_guard<std::mutex> lock(client->m_sendMutex);
                if (!client->m_pendingMessage.empty()) {
                    unsigned char* buf = new unsigned char[LWS_PRE + client->m_pendingMessage.length()];
                    memcpy(buf + LWS_PRE, client->m_pendingMessage.c_str(), client->m_pendingMessage.length());
                    
                    int result = lws_write(wsi, buf + LWS_PRE, client->m_pendingMessage.length(), LWS_WRITE_TEXT);
                    
                    delete[] buf;
                    client->m_pendingMessage.clear();
                    
                    if (result < 0) {
                        return -1;
                    }
                }
            }
            break;
            
        case LWS_CALLBACK_CLIENT_CLOSED:
            std::cout << "WebSocket connection closed" << std::endl;
            client->m_connected = false;
            break;
            
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            std::cerr << "WebSocket connection error" << std::endl;
            client->m_connected = false;
            break;
            
        default:
            break;
    }
    
    return 0;
}

WebSocketClient* WebSocketClient::getInstance(void* user) {
    return g_instance;
}