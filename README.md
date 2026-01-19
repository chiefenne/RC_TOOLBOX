<p align="center">
  <img src="assets/RC_Toolbox_Logo.png" width="280">
</p>

> [!WARNING]
> **Schematic issue in versions < v200:** Early schematics contained a severe error in the LiPo measurement resistor divider design.
> - The divider resistor values were originally selected for **5V ADC operation**, but the ADC actually runs at **3.3V** (fixed in v200+).
> - More importantly, the divider resistors were **wired incorrectly** with respect to the balancer connector voltage levels (fixed in v200+).
>
> The PCB can still be used, but only if you **place/wire the divider resistors according to the corrected v200+ schematics** and use the **correct divider values for 3.3V ADC operation**.

> **⚠️ Work in Progress**
> This project is currently under active development. Features and documentation may change frequently.

# RC TOOLBOX

RC TOOLBOX is a suite of tools tailored for RC enthusiasts.
It is designed to run mainly on ESP32-based boards and provides a versatile platform for RC setup, testing, and measurement tasks.

## 📘 Documentation

The **full user documentation** (features, modules, roadmap, hardware, usage, simulator, ESP32 build instructions) is available here:

👉 **<https://chiefenne.github.io/RC_TOOLBOX/>**

## Highlights

- Servo Tester (up to 6 channels)
- Servo Signal Analyzer
- LiPo Checker
- NFC reader/writer
  - Tags for batteries (buying date, capacity, charge current, etc.)
  - Tags planes (CG location, weight, span, etc.)
  - UAS operator ID (EU regulation)
- CG Scale remote
- Flap deflection & angle of incidence tools
- Touch-driven UI (ILI9341)
- User input via rotary encoder with button
- Multi-language UI
- GUI simulator for rapid development

## Hardware

### PCB

<p align="center">
    <span>
        <img src="docs/assets/RC_TOOLBOX_PCB_ISO_front.png" width="400" style="vertical-align:middle; margin-right:16px;">
        <img src="docs/assets/RC_TOOLBOX_PCB_ISO_back.png" width="400" style="vertical-align:middle;">
    </span>
    <br>
    <em>PCB for the RC TOOLBOX</em>
</p>

<br>

<p align="center">
    <span>
        <img src="docs/assets/RC_TOOLBOX_Enclosure_03.png" width="600" style="vertical-align:middle; margin-right:16px;">
    </span>
    <br>
    <em>RC TOOLBOX installed in the enclosure</em>
</p>

<br>

The PCB design sources and manufacturing outputs are stored in the repository under the `hardware/` folder:

- Board layout: `hardware/RC TOOLBOX.fbrd`
- Schematic: `hardware/RC TOOLBOX.fsch`
- Schematic PDF: `hardware/RC TOOLBOX_Schematic_v200.pdf`
- Gerbers: `hardware/RC TOOLBOX_Gerber_v432.zip`

Direct link: <https://github.com/chiefenne/RC_TOOLBOX/tree/main/hardware>

<br>

> **Note:**
> The PCB has not been fully validated. Order based on the supplied Gerbers at your own risk.

> **Note:**
> Some modules (see below) come in different variants (pinouts, and component layouts vary). Before ordering or assembling, verify that your specific module physically fits the PCB footprint and matches the expected connections.

### Modules

The PCB is designed around a small set of off-the-shelf modules.

#### ESP32-S3 DevKitC-1 (Microcontroller)

- **What it does:** Main controller module running the RC TOOLBOX firmware and driving the UI and peripherals.

<p align="center">
    <span>
        <img src="docs/assets/ESP32-S3_DevKitC-1_board.png" width="400" style="vertical-align:middle; margin-right:16px;">
    </span>
    <br>
    <em>ESP32-S3 DevKitC-1</em>
</p>

#### 2.4" TFT touch display (UI)

- **What it does:** Main user interface with capacitive touch input. Uses an **ILI9341 display driver** and **XPT2046 touch controller**.
- **Module style:** One of the common **red Arduino/ESP maker TFT touch displays**.

<p align="center">
    <span>
        <img src="docs/assets/2.4inch_TFT_touch display.png" width="400" style="vertical-align:middle; margin-right:16px;">
    </span>
    <br>
    <em>2.4" TFT touch display</em>
</p>

#### MP2307-based DC/DC buck modules (Power, x2)

- **What it does:** Converts a **2S/3S LiPo input** to regulated rails.
- **Outputs:** One module provides **3.3V**, one provides **5V**.

<p align="center">
    <span>
        <img src="docs/assets/MP2307_HW133ABC_board.jpg" width="250" style="vertical-align:middle; margin-right:16px;">
    </span>
    <br>
    <em>MP2307 based board (e.g., HW133ABC)</em>
</p>

> **Note:**
> If you have problems with these modules, [this video](https://www.youtube.com/watch?v=gQ5fs7hofPk) might help.

#### ADS1115 16-bit ADC modules (LiPo voltage measurement, x2)

- **What it does:** Accurate LiPo voltage readings **up to 6S** using external 16-bit ADCs.
- **Capacity:** With two ADS1115 modules, **2 ADC channels remain free** after the 6S measurement inputs.
- **Resistors:** For best accuracy, use high-accuracy resistors (**1% or better**) with **low thermal drift** in the voltage divider network so the ADS1115 resolution can be fully utilized (see page 2 on the [schematic PDF](https://github.com/chiefenne/RC_TOOLBOX/blob/main/hardware/RC%20TOOLBOX_Schematic_v194.pdf)).

<p align="center">
    <span>
        <img src="docs/assets/ADS1115_board.jpg" width="200" style="vertical-align:middle; margin-right:16px;">
    </span>
    <br>
    <em>ADS1115 16-bit ADC</em>
</p>

#### PN532 NFC RFID Module (NFC tagging)

- **What it does:** NFC reader/writer module used for tagging and reading data.

<p align="center">
    <span>
        <img src="docs/assets/PN532_NFC_v3.png" width="300" style="vertical-align:middle; margin-right:16px;">
    </span>
    <br>
    <em>NFC reader/writer</em>
</p>

> **Note:**
> Only the I2C bus is wired on the PCB

## Credits

This project was inspired by following resources:

- <https://github.com/Ziege-One/Servotester_Deluxe>
- <https://github.com/TheDIYGuy999/Servotester_Deluxe>
- <https://github.com/shockyfan/Servotester_Deluxe>

## GUI Template for LVGL

- <https://github.com/chiefenne/LVGL_Simulator>
