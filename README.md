# Deribit Trading System

A high-performance order execution and management system for trading on Deribit Test platform, implemented in C++.

## Table of Contents
- [Features](#features)
- [Requirements](#requirements)
- [Quick Start](#quick-start)
- [Installation](#installation)
- [Configuration](#configuration)
- [Usage](#usage)
- [Architecture](#architecture)
- [Documentation](#documentation)
- [Performance](#performance)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

## Features

### Order Management
- Place market and limit orders
- Cancel existing orders
- Modify order parameters
- View and track open orders
- Real-time order status updates

### Market Data
- Real-time order book data
- WebSocket market data streaming
- Instrument information retrieval
- Price and volume tracking

### Position Management
- Real-time position tracking
- Position risk monitoring
- P&L calculation
- Position history

### Performance
- Low-latency execution
- Optimized memory management
- Efficient network communication
- Thread pool implementation
- Lock-free data structures

## Requirements

### System Requirements
- Linux/Unix-based system
- C++17 compatible compiler
- CMake 3.10+
- 4GB RAM minimum
- Stable internet connection

### Dependencies
- libcurl
- nlohmann/json
- WebSocket++
- OpenSSL
- Boost libraries

## Quick Start

```bash
# Clone repository
git clone [repository-url]
cd trading-system

# Install dependencies
sudo apt-get update
sudo apt-get install -y build-essential cmake libcurl4-openssl-dev \
    nlohmann-json3-dev libssl-dev libwebsocketpp-dev libboost-all-dev

# Build project
mkdir build && cd build
cmake ..
make -j4

# Configure
cp ../config.example.ini config.ini
# Edit config.ini with your API credentials

# Run
./trading_system
```

## Installation

Detailed installation instructions available in [docs/guides/setup.md](docs/guides/setup.md)

### Basic Installation Steps
1. Install system dependencies
2. Clone repository
3. Build project
4. Configure API credentials
5. Run system tests

### Compiration
```bash
g++ -std=c++17 -Wall -Wextra -g3 main.cpp -o output/main -I/usr/local/opt/curl/include -L/usr/local/opt/curl/lib -lcurl
```
### Run
```
 ./output/main
```

## Configuration

### API Configuration
```ini
[API]
endpoint=test.deribit.com
client_id=your_client_id
client_secret=your_client_secret
```

### System Configuration
```ini
[System]
log_level=INFO
max_connections=10
retry_attempts=3
timeout_ms=5000
```

### Trading Configuration
```ini
[Trading]
default_leverage=10
risk_limit=100000
max_position_size=10
```

## Usage

### Basic Operations

1. Start the system:
```bash
./trading_system
```

2. Main Menu Options:
- 1: Place New Order
- 2: Get Order Book
- 3: Get Position
- 4: Get Open Orders
- 5: Cancel Order
- 6: Modify Order
- 7: List Available Instruments
- 0: Exit

### Example: Placing an Order
```bash
1  # Select Place New Order
BTC-PERPETUAL  # Enter instrument
1.0  # Enter amount
limit  # Choose order type
50000  # Enter price
```

### Example: Checking Position
```bash
3  # Select Get Position
BTC-PERPETUAL  # Enter instrument
```

## Architecture

### Component Overview
```
├── Core Components
│   ├── Authentication
│   ├── Order Management
│   ├── Market Data
│   └── Position Tracking
├── Support Systems
│   ├── Error Handling
│   ├── Logging
│   └── Configuration
└── External Interfaces
    ├── REST API
    └── WebSocket
```

### Data Flow
1. Market Data Flow
2. Order Flow
3. Position Management Flow

## Documentation

Comprehensive documentation available in the `docs` directory:

- [Setup Guide](docs/guides/setup.md)
- [Performance Guide](docs/guides/performance.md)
- [Troubleshooting Guide](docs/guides/troubleshooting.md)
- [API Reference](docs/reference/api.md)

## Performance

### Optimizations
- Memory pool implementation
- Lock-free data structures
- Connection pooling
- Thread pool management
- Efficient market data processing

### Benchmarks
- Order placement latency: <1ms
- Market data processing: <100μs
- WebSocket message handling: <50μs

## Troubleshooting

Common issues and solutions available in [docs/guides/troubleshooting.md](docs/guides/troubleshooting.md)

### Quick Troubleshooting
1. Authentication Issues
   - Verify API credentials
   - Check network connectivity
   - Validate SSL certificates

2. Performance Issues
   - Monitor system resources
   - Check network latency
   - Review log files

## Contributing

### Development Setup
1. Fork repository
2. Create feature branch
3. Implement changes
4. Run tests
5. Submit pull request

### Code Style
- Follow C++ Core Guidelines
- Use clang-format
- Maintain documentation

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Deribit API Documentation
- C++ Libraries Used
- Contributors

## Contact

For support or queries:
- Email: [robinsingh248142@gmail.com]