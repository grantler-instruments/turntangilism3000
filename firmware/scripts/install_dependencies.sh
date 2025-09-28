#/bin/sh
OS="$(uname -s)"

if [ "$OS" = "Linux" ]; then
    echo "Running on Linux"
    curl -fsSL -o get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
    sudo python3 get-platformio.py
    sudo ln -s /root/.platformio/penv/bin/platformio /usr/local/bin/pio
elif [ "$OS" = "Darwin" ]; then
    echo "Running on macOS"
    brew install platformio
else
    echo "Unknown operating system"
fi

exit


