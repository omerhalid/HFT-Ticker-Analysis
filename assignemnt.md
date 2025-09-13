
# HFT Developer Project

Create a C++ application that subscribes to Coinbase ticker data via WebSockets.

### Requirements

1. **Subscription**
   - Connect to Coinbase public ticker data using WebSockets.
   - *Note:* No API keys are required. The data is publicly available even though the documentation may suggest authentication.

2. **Data Parsing**
   - Parse the incoming ticker JSON messages.

3. **Calculations**
   - Calculate the **Exponential Moving Average (EMA)** of:
     - The `price` field.
     - The **mid-price**, defined as:
       ```
       (best_bid_price + best_ask_price) / 2
       ```
     - Use a 5-second interval for EMA calculations.

4. **Logging**
   - Log all fields from the incoming message **plus** the calculated EMAs.
   - Save the logs to a **CSV file**.

5. **Testing**
   - Write test cases

### Notes
- You may use third-party libraries for WebSocket handling and JSON parsing.