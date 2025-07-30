# MyTrace
## Overview
`MyTrace` is a network utility that traces the route from your computer to given address
## Installation
### Requirements
- Linux-based OS
- C compiler
### Build
```
# Clone the repository
https://github.com/Smet-k/MyTrace
cd MyTrace

# Build the utility
./build.sh

# Run
sudo ./build/myTrace
```

## Usage
```
myTraceroute [-m maxttl] [-t timeout] destination
```

where:

 - \-m, --maxttl: Set max TTL(0-255) for packets
 - \-t, --timeout: Set timeout(1-255) in seconds for packets
 - \-h, --help: Shows a help message