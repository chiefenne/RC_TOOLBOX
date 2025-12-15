# Servo Tester

Test and calibrate up to 6 servos simultaneously with precise PWM control.

![Servo Screen](../assets/Servo_screen.png)

## Interface

### Servo Selection (Sidebar)

The left sidebar shows buttons **1-6** for each servo channel:

- **Tap** a button to toggle that servo on/off
- **Long press** anywhere to toggle all servos on/off
- Selected servos are highlighted in blue
- Multiple servos can be active simultaneously

When multiple servos are selected, they all respond to the same controls but maintain their individual positions.

### PWM Display

Shows the current PWM pulse width in microseconds (µs).

- When multiple servos are selected, displays the primary (lowest numbered) servo's value with an asterisk (*) suffix

### Mode Buttons

| Button | Description |
|--------|-------------|
| **Auto** | Automatic sweep mode |
| **Manual** | Direct PWM control |

---

## Auto Mode

In Auto mode, selected servos sweep continuously between min and max PWM values.

### Controls

- **Start** – Begin sweeping
- **Stop** – Stop sweeping (button turns red while running)

### Encoder Actions

| Action | Function |
|--------|----------|
| **Rotate** | Adjust sweep speed (while running) |
| **Double-click** | Switch to Manual mode |
| **Long press** | Toggle all servos on/off |

The sweep speed setting is saved and persists across sessions.

---

## Manual Mode

In Manual mode, you have direct control over servo position.

### Controls

- **Slider** – Drag to set PWM value
- **Min** – Jump to minimum PWM
- **Center** – Jump to center PWM
- **Max** – Jump to maximum PWM

### Encoder Actions

| Action | Function |
|--------|----------|
| **Rotate** | Adjust PWM value (with acceleration) |
| **Double-click** | Switch back to Auto mode |
| **Long press** | Toggle all servos on/off |

Rotation speed affects step size:
- Slow rotation = fine adjustment
- Fast rotation = larger steps (up to 5× base step)

---

## PWM Settings

Configure PWM parameters in [Settings](settings.md):

| Setting | Default | Range |
|---------|---------|-------|
| PWM Min | 1000 µs | 500–1500 µs |
| PWM Center | 1500 µs | 1000–2000 µs |
| PWM Max | 2000 µs | 1500–2500 µs |
| Frequency | 50 Hz | 50 Hz / 333 Hz |

### Protocol Presets

| Preset | Min | Center | Max | Frequency |
|--------|-----|--------|-----|-----------|
| Standard | 1000 | 1500 | 2000 | 50 Hz |
| Extended | 500 | 1500 | 2500 | 50 Hz |
| Sanwa | 920 | 1520 | 2120 | 50 Hz |
| Futaba | 900 | 1500 | 2100 | 50 Hz |
| Digital Fast | 1000 | 1500 | 2000 | 333 Hz |
| Custom | User-defined | | | |

---

## Hardware Outputs

Servos are connected to these GPIO pins (defined in `include/pins.h`):

| Servo | GPIO |
|-------|------|
| 1 | 6 |
| 2 | 15 |
| 3 | 16 |
| 4 | 17 |
| 5 | 18 |
| 6 | 21 |

Use standard 3-pin servo connectors (Signal, VCC, GND). Ensure adequate power supply for your servos – do not power multiple servos directly from the ESP32's 3.3V/5V pins.
