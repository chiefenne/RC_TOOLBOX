<p align="center">
  <img src="assets/RC_Toolbox_Logo.png" width="200">
</p>

> **⚠️ Work in Progress**
> This project is currently under active development. Features and documentation may change frequently. Contributions and feedback are welcome!

> **Note:** The implementation recently moved to C++ with LVGL. Core RC logic may later still be implemented in MicroPython depending on the module.

RC TOOLBOX is a suite of software tools tailored for RC enthusiasts. Designed to run on mainly on ESP32 based boards. It provides a versatile platform for enhancing your RC experience.

## GUI

The application features a graphical user interface (GUI) built with [LVGL](https://lvgl.io/) for displaying information and controlling the software. The GUI is optimized for **ILI9341 TFT touch displays** (320x240 resolution). User input is supported via:

- **Touch screen** – intuitive touch-based interaction
- **EN11 rotary encoder with push button** – alternative input method for precise control and navigation

<p align="center">
  <img src="assets/Splash_screen.png" width="400"><br>
  <em>Splash screen running in the LVGL simulator</em>
</p>

<p align="center">
  <img src="assets/Home_screen.png" width="400"><br>
  <em>Home screen running in the LVGL simulator</em>
</p>

<p align="center">
  <img src="assets/Servo_screen.png" width="400"><br>
  <em>Servo screen running in the LVGL simulator</em>
</p>

## Modules

It includes modules to:

- Servo tester
- Lipo tester
- Measure flap deflection
- Measure incidence angle
  - Angle between the wing chord and vertical stabilizer
- CG scale

## Credits

This project was inspired by following resources:

- [Servotester_Deluxe by "Der RC-Modellbauer"(Ziege-One)](https://github.com/Ziege-One/Servotester_Deluxe)
- [TheDIYGuy999](https://github.com/TheDIYGuy999/Servotester_Deluxe)
- [shockyfan](https://github.com/shockyfan/Servotester_Deluxe)

Special thanks to the RC modeling community for their invaluable contributions and insights.

## GUI Template for LVGL
- [LVGL Simulator Template](https://github.com/chiefenne/LVGL_Simulator) - GUI template for LVGL with macOS simulator

## Building

### Prerequisites
- **macOS**: Xcode command line tools, SDL2 (`brew install sdl2`)
- **VS Code**: PlatformIO extension for ESP32 development

### macOS Simulator (for rapid GUI development)
```bash
./simulator/build_sim.sh           # Release build
./simulator/build_sim_debug.sh     # Debug build
./binaries/lvgl_simulator_macOS    # Run simulator
```

### ESP32 (via VS Code + PlatformIO)
Use the PlatformIO sidebar in VS Code:
- **Build**: Click the checkmark icon or `Ctrl+Alt+B`
- **Upload**: Click the arrow icon or `Ctrl+Alt+U`
- **Monitor**: Click the plug icon for serial output

See [LVGL_Simulator](https://github.com/chiefenne/LVGL_Simulator) for detailed LVGL library setup instructions.
