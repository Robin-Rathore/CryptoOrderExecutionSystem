#include <iostream>
#include <string>
#include <curl/curl.h>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <vector>
#include <algorithm>
#include "include/json.hpp"

using json = nlohmann::json;

// Function to handle the response from the cURL request
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// General function to send a cURL request with optional access token
std::string sendRequest(const std::string& url, const json& payload, const std::string& accessToken = "") {
    std::string readBuffer;
    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        
        // Set the request payload
        std::string jsonStr = payload.dump();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
        
        // Set headers, including Authorization if accessToken is provided
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        if (!accessToken.empty()) {
            headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        // Set up the write callback to capture the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        // Perform the request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
        }
        
        // Free Resources
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return readBuffer;
}

// Function to get the list of available instruments
std::vector<std::string> getAvailableInstruments(const std::string& accessToken) {
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "public/get_instruments"},
        {"params", {{"currency", "BTC"}, {"kind", "future"}}},
        {"id", 2}
    };
    
    std::string response = sendRequest("https://test.deribit.com/api/v2/public/get_instruments", payload, accessToken);
    auto responseJson = json::parse(response);
    
    std::vector<std::string> instruments;
    if (responseJson.contains("result")) {
        for (const auto& instrument : responseJson["result"]) {
            instruments.push_back(instrument["instrument_name"].get<std::string>());
        }
    }
    return instruments;
}

// Function to get instrument specifications
json getInstrumentSpecs(const std::string& accessToken, const std::string& instrument) {
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "public/get_instrument"},
        {"params", {{"instrument_name", instrument}}},
        {"id", 16}
    };
    std::string response = sendRequest("https://test.deribit.com/api/v2/public/get_instrument", payload, accessToken);
    return json::parse(response);
}

// Function to round to tick size
double roundToTickSize(double price, double tickSize) {
    return std::round(price / tickSize) * tickSize;
}

// Function to get market data
json getMarketData(const std::string& accessToken, const std::string& instrument) {
    json payload = {
        {"jsonrpc", "2.0"},
        {"method", "public/get_order_book"},
        {"params", {{"instrument_name", instrument}}},
        {"id", 15}
    };
    std::string response = sendRequest("https://test.deribit.com/api/v2/public/get_order_book", payload, accessToken);
    return json::parse(response);
}

// Function to get the access token
std::string getAccessToken(const std::string& clientId, const std::string& clientSecret) {
    json payload = {
        {"id", 0},
        {"method", "public/auth"},
        {"params", {
            {"grant_type", "client_credentials"},
            {"scope", "session:apiconsole-c5i26ds6dsr expires:2592000"},
            {"client_id", clientId},
            {"client_secret", clientSecret}
        }},
        {"jsonrpc", "2.0"}
    };
    
    std::string response = sendRequest("https://test.deribit.com/api/v2/public/auth", payload);
    auto responseJson = json::parse(response);
    
    if (responseJson.contains("result") && responseJson["result"].contains("access_token")) {
        return responseJson["result"]["access_token"];
    }
    std::cerr << "Failed to retrieve access token." << std::endl;
    return "";
}

// Function to place an order with user-specified parameters
bool placeOrder(const std::string& accessToken, const std::string& amount, const std::string& instrument) {
    try {
        // Get instrument specifications
        auto instrumentSpecs = getInstrumentSpecs(accessToken, instrument);
        if (!instrumentSpecs.contains("result")) {
            std::cerr << "Failed to get instrument specifications" << std::endl;
            return false;
        }
        
        double tickSize = instrumentSpecs["result"]["tick_size"].get<double>();
        
        // Get market data
        auto marketData = getMarketData(accessToken, instrument);
        if (!marketData.contains("result")) {
            std::cerr << "Failed to get market data" << std::endl;
            return false;
        }
        
        // Get bid and ask prices
        double bestBid = marketData["result"]["best_bid_price"].get<double>();
        double bestAsk = marketData["result"]["best_ask_price"].get<double>();
        double markPrice = marketData["result"]["mark_price"].get<double>();
        
        // Let user choose the price
        double userPrice;
        std::cout << "Current market prices:\n";
        std::cout << "Best Bid: " << bestBid << "\n";
        std::cout << "Best Ask: " << bestAsk << "\n";
        std::cout << "Mark Price: " << markPrice << "\n";
        std::cout << "Enter your desired price (0 for market price): ";
        std::cin >> userPrice;
        
        double priceValue;
        if (userPrice == 0) {
            // Use mark price as reference for market orders
            priceValue = roundToTickSize(markPrice, tickSize);
        } else {
            priceValue = roundToTickSize(userPrice, tickSize);
        }
        
        // Format price with correct precision
        std::stringstream ss;
        ss << std::fixed << std::setprecision(8) << priceValue;
        std::string price = ss.str();
        
        // Let user choose order type
        std::string orderType;
        std::cout << "Choose order type (market/limit): ";
        std::cin >> orderType;
        std::transform(orderType.begin(), orderType.end(), orderType.begin(), ::tolower);
        
        // Let user choose direction
        std::string direction;
        std::cout << "Choose direction (buy/sell): ";
        std::cin >> direction;
        std::transform(direction.begin(), direction.end(), direction.begin(), ::tolower);
        
        // Prepare the payload
        json payload = {
            {"jsonrpc", "2.0"},
            {"method", "private/" + direction},
            {"params", {
                {"instrument_name", instrument},
                {"type", orderType},
                {"price", price},
                {"amount", amount},
                {"post_only", true},
                {"time_in_force", "good_til_cancelled"}
            }},
            {"id", 1}
        };
        
        std::string response = sendRequest("https://test.deribit.com/api/v2/private/" + direction, payload, accessToken);
        std::cout << "Place Order Response: " << response << std::endl;
        
        auto responseJson = json::parse(response);
        return !responseJson.contains("error");
    } catch (const std::exception& e) {
        std::cerr << "Error placing order: " << e.what() << std::endl;
        return false;
    }
}

// Function to cancel an order
void cancelOrder(const std::string& accessToken, const std::string& orderID) {
    try {
        json payload = {
            {"jsonrpc", "2.0"},
            {"method", "private/cancel"},
            {"params", {{"order_id", orderID}}},
            {"id", 6}
        };
        std::string response = sendRequest("https://test.deribit.com/api/v2/private/cancel", payload, accessToken);
        std::cout << "Cancel Order Response: " << response << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error canceling order: " << e.what() << std::endl;
    }
}

// Function to modify an order
void modifyOrder(const std::string& accessToken, const std::string& orderID, int amount, double price) {
    try {
        // Get order state to get instrument name
        json statePayload = {
            {"jsonrpc", "2.0"},
            {"method", "private/get_order_state"},
            {"params", {{"order_id", orderID}}},
            {"id", 11}
        };
        
        std::string stateResponse = sendRequest("https://test.deribit.com/api/v2/private/get_order_state", statePayload, accessToken);
        auto orderState = json::parse(stateResponse);
        
        if (orderState.contains("result")) {
            std::string instrument = orderState["result"]["instrument_name"];
            auto instrumentSpecs = getInstrumentSpecs(accessToken, instrument);
            
            if (instrumentSpecs.contains("result")) {
                double tickSize = instrumentSpecs["result"]["tick_size"].get<double>();
                price = roundToTickSize(price, tickSize);
                
                std::stringstream ss;
                ss << std::fixed << std::setprecision(8) << price;
                
                json modifyPayload = {
                    {"jsonrpc", "2.0"},
                    {"method", "private/edit"},
                    {"params", {
                        {"order_id", orderID},
                        {"amount", amount},
                        {"price", ss.str()}
                    }},
                    {"id", 11}
                };
                
                std::string response = sendRequest("https://test.deribit.com/api/v2/private/edit", modifyPayload, accessToken);
                std::cout << "Modify Order Response: " << response << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error modifying order: " << e.what() << std::endl;
    }
}

// Function to retrieve the order book
void getOrderBook(const std::string& accessToken, const std::string& instrument) {
    try {
        json payload = {
            {"jsonrpc", "2.0"},
            {"method", "public/get_order_book"},
            {"params", {{"instrument_name", instrument}}},
            {"id", 15}
        };
        std::string response = sendRequest("https://test.deribit.com/api/v2/public/get_order_book", payload, accessToken);
        auto responseJson = json::parse(response);
        
        if (responseJson.contains("result")) {
            std::cout << "Order Book for " << instrument << ":\n\n";
            
            std::cout << "Best Bid Price: " << responseJson["result"]["best_bid_price"] 
                      << ", Amount: " << responseJson["result"]["best_bid_amount"] << '\n';
            std::cout << "Best Ask Price: " << responseJson["result"]["best_ask_price"] 
                      << ", Amount: " << responseJson["result"]["best_ask_amount"] << '\n';
            
            std::cout << "\nAsks (Sell Orders):\n";
            for (const auto& ask : responseJson["result"]["asks"]) {
                std::cout << "Price: " << ask[0] << ", Amount: " << ask[1] << '\n';
            }
            
            std::cout << "\nBids (Buy Orders):\n";
            for (const auto& bid : responseJson["result"]["bids"]) {
                std::cout << "Price: " << bid[0] << ", Amount: " << bid[1] << '\n';
            }
            
            std::cout << "\nMark Price: " << responseJson["result"]["mark_price"] << '\n';
            std::cout << "Open Interest: " << responseJson["result"]["open_interest"] << '\n';
            std::cout << "Timestamp: " << responseJson["result"]["timestamp"] << '\n';
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting order book: " << e.what() << std::endl;
    }
}

// Function to get position details
void getPosition(const std::string& accessToken, const std::string& instrument) {
    try {
        json payload = {
            {"jsonrpc", "2.0"},
            {"method", "private/get_position"},
            {"params", {{"instrument_name", instrument}}},
            {"id", 20}
        };
        std::string response = sendRequest("https://test.deribit.com/api/v2/private/get_position", payload, accessToken);
        auto responseJson = json::parse(response);
        
        if (responseJson.contains("result")) {
            std::cout << "Position Details for " << instrument << ":\n\n";
            auto result = responseJson["result"];
            
            std::cout << "Size: " << result["size"] << '\n'
                      << "Direction: " << result["direction"] << '\n'
                      << "Average Price: " << result["average_price"] << '\n'
                      << "Leverage: " << result["leverage"] << '\n'
                      << "Mark Price: " << result["mark_price"] << '\n'
                      << "Floating Profit Loss: " << result["floating_profit_loss"] << '\n'
                      << "Total Profit Loss: " << result["total_profit_loss"] << '\n'
                      << "Open Orders Margin: " << result["open_orders_margin"] << '\n'
                      << "Initial Margin: " << result["initial_margin"] << '\n'
                      << "Maintenance Margin: " << result["maintenance_margin"] << '\n';
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting position: " << e.what() << std::endl;
    }
}

void getOpenOrders(const std::string& accessToken) {
    try {
        json payload = {
            {"jsonrpc", "2.0"},
            {"method", "private/get_open_orders_by_currency"},
            {"params", {{"currency", "BTC"}}},
            {"id", 25}
        };
        std::string response = sendRequest("https://test.deribit.com/api/v2/private/get_open_orders_by_currency", payload, accessToken);
        auto responseJson = json::parse(response);
        
        if (responseJson.contains("result")) {
            std::cout << "Open Orders:\n\n";
            for (const auto& order : responseJson["result"]) {
                std::cout << "Instrument: " << order["instrument_name"]
                          << "\nOrder ID: " << order["order_id"]
                          << "\nDirection: " << order["direction"]
                          << "\nPrice: " << order["price"]
                          << "\nAmount: " << order["amount"]
                          << "\nFilled Amount: " << order["filled_amount"]
                          << "\nOrder Type: " << order["order_type"]
                          << "\nOrder State: " << order["order_state"]
                          << "\n-------------------\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting open orders: " << e.what() << std::endl;
    }
}

int main() {
    std::string clientId = "26izQvaT";
    std::string clientSecret = "zkQnObmK0mz1LTZZvcMLtZ06x_X6GPeSDXBQMVEBRl8";
    std::string accessToken = getAccessToken(clientId, clientSecret);
    
    if (!accessToken.empty()) {
        std::vector<std::string> availableInstruments = getAvailableInstruments(accessToken);
        
        while(1) {
            int choice;
            
            // Display menu
            std::cout << "\n=== Trading Operations Menu ===\n";
            std::cout << "1. Place New Order\n";
            std::cout << "2. Get Order Book\n";
            std::cout << "3. Get Position\n";
            std::cout << "4. Get Open Orders\n";
            std::cout << "5. Cancel Order\n";
            std::cout << "6. Modify Order\n";
            std::cout << "7. List Available Instruments\n";
            std::cout << "0. Exit\n";
            std::cout << "Enter your choice: ";
            
            std::cin >> choice;
            
            try {
                switch (choice) {
                    case 1: {
                        std::string instrument;
                        std::string amount;
                        
                        std::cout << "\nAvailable instruments:\n";
                        for (const auto& inst : availableInstruments) {
                            std::cout << inst << "\n";
                        }
                        
                        std::cout << "\nEnter instrument name: ";
                        std::cin >> instrument;
                        
                        // Validate instrument
                        if (std::find(availableInstruments.begin(), availableInstruments.end(), instrument) == availableInstruments.end()) {
                            std::cout << "Invalid instrument name!\n";
                            break;
                        }
                        
                        std::cout << "Enter amount to trade: ";
                        std::cin >> amount;
                        
                        if (placeOrder(accessToken, amount, instrument)) {
                            std::cout << "Order placed successfully\n";
                        } else {
                            std::cout << "Failed to place order\n";
                        }
                        break;
                    }
                    
                    case 2: {
                        std::string instrument;
                        std::cout << "Enter instrument name (e.g., BTC-PERPETUAL): ";
                        std::cin >> instrument;
                        getOrderBook(accessToken, instrument);
                        break;
                    }
                    
                    case 3: {
                        std::string instrument;
                        std::cout << "Enter instrument name to check position: ";
                        std::cin >> instrument;
                        getPosition(accessToken, instrument);
                        break;
                    }
                    
                    case 4:
                        getOpenOrders(accessToken);
                        break;
                        
                    case 5: {
                        std::string order_id;
                        std::cout << "Enter order ID to cancel: ";
                        std::cin >> order_id;
                        cancelOrder(accessToken, order_id);
                        break;
                    }
                        
                    case 6: {
                        std::string order_id;
                        double price;
                        int amount;
                        std::cout << "Enter order ID to modify: ";
                        std::cin >> order_id;
                        std::cout << "Enter new amount: ";
                        std::cin >> amount;
                        std::cout << "Enter new price: ";
                        std::cin >> price;
                        modifyOrder(accessToken, order_id, amount, price);
                        break;
                    }
                    
                    case 7: {
                        std::cout << "\nAvailable instruments:\n";
                        for (const auto& inst : availableInstruments) {
                            std::cout << inst << "\n";
                        }
                        break;
                    }
                        
                    case 0:
                        std::cout << "Exiting program...\n";
                        return 0;
                        
                    default:
                        std::cout << "Invalid choice! Please try again.\n";
                        break;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }
    }
    return 0;
}