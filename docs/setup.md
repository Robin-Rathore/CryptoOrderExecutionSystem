# Deribit Trading System Setup Guide

## Prerequisites

### System Requirements
- Linux/Unix-based system
- C++17 compatible compiler (GCC 7+ or Clang 5+)
- CMake 3.10+
- At least 4GB RAM
- Stable internet connection

### Required Libraries
```bash
# Update package list
sudo apt-get update

# Install essential build tools
sudo apt-get install -y \
    build-essential \
    cmake \
    git

# Install development libraries
sudo apt-get install -y \
    libcurl4-openssl-dev \
    nlohmann-json3-dev \
    libssl-dev \
    libwebsocketpp-dev \
    libboost-all-dev
```

## Building the Project

### 1. Clone the Repository
```bash
git clone [your-repository-url]
cd trading-system
```

### 2. Create Build Directory
```bash
mkdir build
cd build
```

### 3. Configure and Build
```bash
cmake ..
make -j4
```

### 4. Verify Installation
```bash
./trading_system --version
```

## Configuration Setup

### 1. API Credentials
1. Create a file `config.ini` in the project root:
```ini
[API]
endpoint=test.deribit.com
client_id=your_client_id
client_secret=your_client_secret
```

2. Set proper permissions:
```bash
chmod 600 config.ini
```

### 2. Logging Configuration
1. Create logging directory:
```bash
mkdir -p logs
```

2. Configure log settings in `logging.conf`:
```ini
[Logging]
level=INFO
file=logs/trading_system.log
max_size=10M
backup_count=5
```

## Initial Testing

### 1. Authentication Test
```bash
./trading_system --test-auth
```

### 2. Market Data Test
```bash
./trading_system --test-market-data
```

### 3. Order Test
```bash
./trading_system --test-order
```

## Troubleshooting

### Common Installation Issues

1. CMake Configuration Fails
```bash
# Solution: Install missing dependencies
sudo apt-get install -y cmake-curses-gui
ccmake ..
```

2. Library Not Found
```bash
# Solution: Add library path
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

3. Compilation Errors
```bash
# Solution: Update compiler
sudo apt-get install -y g++-9
export CXX=/usr/bin/g++-9
```

## Production Deployment

### 1. System Service Setup
Create systemd service file:
```ini
[Unit]
Description=Deribit Trading System
After=network.target

[Service]
ExecStart=/path/to/trading_system
WorkingDirectory=/path/to/project
User=trading
Group=trading
Restart=always

[Install]
WantedBy=multi-user.target
```

### 2. Monitoring Setup
```bash
# Install monitoring tools
sudo apt-get install -y prometheus node-exporter grafana

# Configure monitoring endpoints in trading_system.conf
[Monitoring]
prometheus_port=9090
metrics_endpoint=/metrics
```

### 3. Backup Configuration
```bash
# Create backup script
mkdir -p /backup/trading_system
cp config.ini /backup/trading_system/
cp -r logs /backup/trading_system/
```

## Security Considerations

### 1. File Permissions
```bash
# Set proper ownership
sudo chown -R trading:trading /path/to/project

# Set proper permissions
chmod 700 /path/to/project
chmod 600 config.ini
```

### 2. Network Security
```bash
# Configure firewall
sudo ufw allow 22/tcp
sudo ufw allow 9090/tcp
sudo ufw enable
```

## Maintenance

### Daily Checks
```bash
# Check system status
systemctl status trading_system

# Check logs
tail -f logs/trading_system.log

# Check disk space
df -h
```

### Weekly Tasks
```bash
# Backup configuration
./scripts/backup.sh

# Check for updates
git pull origin master
```