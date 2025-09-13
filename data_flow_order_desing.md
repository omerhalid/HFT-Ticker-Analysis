I'll explain the complete order flow of this HFT ticker analysis project. Let me trace through the entire execution path from start to finish.


[3 tools called]

Now let me create a comprehensive flow diagram to visualize the entire execution order:


[1 tool called]

Let me explain the complete order flow of this HFT ticker analysis project:

## �� **Complete Order Flow of the HFT Ticker Analysis Project**

### **1. Application Startup (`main.cpp`)**

```
main() function starts
├── Parse command line arguments (-p product, -o output file)
├── Set up signal handlers (SIGINT, SIGTERM) for graceful shutdown
├── Create CoinbaseTickerAnalyzer instance
└── Call analyzer->start()
```

### **2. Analyzer Initialization (`CoinbaseTickerAnalyzer::start()`)**

```
start() method called
├── Check if already running
├── Call initializeComponents()
│   ├── Create WebSocketClient instance
│   ├── Set message callback: handleWebSocketMessage()
│   ├── Create EMACalculator(5) - 5-second interval
│   └── Create CSVLogger with output filename
├── Start data processing thread: processDataThread()
├── Connect to WebSocket: wss://ws-feed.exchange.coinbase.com
├── Wait 1 second for connection
├── Subscribe to ticker channel for product (e.g., BTC-USD)
└── Set running = true
```

### **3. WebSocket Connection (`WebSocketClient::connect()`)**

```
connect() method
├── Parse URI (wss://ws-feed.exchange.coinbase.com)
├── Create libwebsockets context with SSL support
├── Set up client connection info
├── Create WebSocket connection (lws_client_connect_via_info)
├── Start I/O thread: runIO()
└── Wait for connection establishment
```

### **4. WebSocket I/O Thread (`WebSocketClient::runIO()`)**

```
runIO() runs in separate thread
└── Continuous loop: lws_service() with 50ms timeout
    ├── Handles incoming messages → callback()
    ├── Handles outgoing messages → callback()
    └── Manages connection state
```

### **5. WebSocket Callback System (`WebSocketClient::callback()`)**

```
callback() handles libwebsockets events:
├── LWS_CALLBACK_CLIENT_ESTABLISHED
│   └── Set m_connected = true
├── LWS_CALLBACK_CLIENT_RECEIVE
│   └── Call m_messageCallback(message) → handleWebSocketMessage()
├── LWS_CALLBACK_CLIENT_WRITEABLE
│   └── Send pending message via lws_write()
└── LWS_CALLBACK_CLIENT_CLOSED/ERROR
    └── Set m_connected = false
```

### **6. Message Processing Flow**

```
WebSocket receives ticker message
├── callback() → LWS_CALLBACK_CLIENT_RECEIVE
├── handleWebSocketMessage(message)
│   ├── JSONParser::parseTickerMessage(message, tickerData)
│   ├── Add tickerData to m_dataQueue (thread-safe)
│   └── Notify processing thread: m_queueCondition.notify_one()
```

### **7. Data Processing Thread (`processDataThread()`)**

```
processDataThread() runs continuously:
├── Wait for data or shutdown signal
├── Process all queued ticker data:
│   └── For each TickerData:
│       ├── Pop from queue
│       └── Call processTickerData(data)
```

### **8. Ticker Data Processing (`processTickerData()`)**

```
processTickerData(TickerData& data):
├── Calculate Price EMA:
│   └── m_emaCalculator->updatePriceEMA(price, timestamp)
├── Calculate Mid-Price EMA:
│   └── m_emaCalculator->updateMidPriceEMA(mid_price, timestamp)
├── Log to CSV:
│   └── m_csvLogger->logTickerData(data)
└── Print to console for monitoring
```

### **9. EMA Calculation (`EMACalculator`)**

```
updatePriceEMA() / updateMidPriceEMA():
├── Check if should update (5-second interval)
├── If first time: Initialize with current value
├── If subsequent: Apply EMA formula:
│   └── newEMA = alpha * currentValue + (1-alpha) * previousEMA
└── Update timestamp and return new EMA
```

### **10. CSV Logging (`CSVLogger`)**

```
logTickerData():
├── Lock file mutex
├── Write CSV row with all ticker data + EMAs
└── Flush to disk
```

### **11. Main Thread Loop (`main.cpp`)**

```
Main thread continues:
└── while (analyzer->isRunning()):
    └── Sleep 100ms and check running status
```

### **12. Graceful Shutdown**

```
Signal received (Ctrl+C):
├── signalHandler() called
├── analyzer->stop()
│   ├── Set m_running = false
│   ├── cleanupComponents()
│   │   ├── Set m_processingEnabled = false
│   │   ├── Notify processing thread to wake up
│   │   ├── Join processing thread
│   │   ├── Disconnect WebSocket
│   │   └── Close CSV file
│   └── Print shutdown message
└── Application exits
```

## 🔄 **Thread Architecture**

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Main Thread   │    │ WebSocket I/O    │    │ Data Processing │
│                 │    │ Thread           │    │ Thread          │
├─────────────────┤    ├──────────────────┤    ├─────────────────┤
│ • Parse args    │    │ • runIO()        │    │ • processData   │
│ • Create analyzer│    │ • lws_service()  │    │   Thread()      │
│ • Start analyzer│    │ • Handle events  │    │ • Process queue │
│ • Monitor loop  │    │ • Call callbacks │    │ • Calculate EMAs│
│ • Signal handler│    │                  │    │ • Log to CSV    │
└─────────────────┘    └──────────────────┘    └─────────────────┘
         │                       │                       │
         └───────────────────────┼───────────────────────┘
                                 │
                    ┌─────────────────────┐
                    │   Shared Data       │
                    │                     │
                    │ • m_dataQueue       │
                    │ • m_queueMutex      │
                    │ • m_queueCondition  │
                    │ • m_emaCalculator   │
                    │ • m_csvLogger       │
                    └─────────────────────┘
```

## �� **Data Flow Summary**

1. **WebSocket** receives JSON ticker messages from Coinbase
2. **JSONParser** extracts ticker data (price, bid, ask, etc.)
3. **Queue** stores ticker data for processing
4. **Processing Thread** calculates EMAs and logs to CSV
5. **EMA Calculator** maintains separate 5-second EMAs for price and mid-price
6. **CSV Logger** writes all data to file
7. **Console** displays real-time processing status

This architecture ensures **high performance** and **thread safety** for HFT applications by separating I/O, processing, and logging into dedicated threads with proper synchronization.