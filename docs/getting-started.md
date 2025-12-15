# Getting started

## Requirements

### Hardware

- **ESP32-S3 DevKitC-1** (or compatible ESP32-S3 board)
- **2.4" TFT touch display** with ILI9341 driver and XPT2046 touch controller
- **EC11 rotary encoder** with push button
- USB-C cable for programming and power
- Jumper wires or custom PCB for connections

→ See [Hardware Setup](hardware.md) for detailed pinout and wiring.

### Software

- [VS Code](https://code.visualstudio.com/) with [PlatformIO extension](https://platformio.org/install/ide?install=vscode)
- Git (to clone the repository)

## Quick start

1. **Clone the repository**
   ```bash
   git clone https://github.com/chiefenne/RC_TOOLBOX.git
   cd RC_TOOLBOX
   ```

2. **Open in VS Code**
   - Open the `RC_TOOLBOX` folder in VS Code
   - PlatformIO will automatically detect the project and install dependencies
    - The initial setup may take a few minutes as dependencies are downloaded and configured.

3. **Connect your hardware**
   - Wire the display, touch, and encoder according to [Hardware](hardware.md)
   - Connect the ESP32-S3 via USB

4. **Build and upload**
   - Click the PlatformIO "Upload" button (→ arrow in the bottom toolbar)
   - Or run: `pio run --target upload`

5. **Done!**
   - The device will reboot and show the RC TOOLBOX splash screen
   - Use touch or the rotary encoder to navigate

## Next steps

- [Hardware](hardware.md) – Detailed wiring and pinout
- [Firmware](firmware.md) – Build options and pre-built binaries
- [Usage](usage/overview.md) – How to use the tools
