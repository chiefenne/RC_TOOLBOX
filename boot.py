# boot.py -- run on boot-up
# this script is executed when the microcontroller boots up. It sets
# up various configuration options for the board.
# can run arbitrary Python, but best to keep it minimal

import sys
from machine import Pin
import time


# check platform
print('BOOT - INFO: Platform:', sys.platform)
print('BOOT - INFO: Operating system:', sys.version)
print('BOOT - INFO: Underlying machine:', sys.implementation._machine)


def blink(led, seconds, hz):
    """
    Blink LED for 'seconds' and 'hz' frequency.
    """
    if hz <= 0 or seconds <= 0:
        return
    interval = 1 / hz
    for _ in range(int(seconds * hz)):
        led.value(not led.value())
        time.sleep(interval)
    led.value(1)  # ensure LED is off

# SEEED XIAO RP2040 RGB pins; NeoPixel LED is 11 (power) and 12 (data)
rp2_led_pins = {'r': 17, 'g': 16, 'b': 25}
try:
    ledr = Pin(rp2_led_pins['r'], Pin.OUT)
    ledg = Pin(rp2_led_pins['g'], Pin.OUT)
    ledb = Pin(rp2_led_pins['b'], Pin.OUT)
    for led in (ledr, ledg, ledb):
        led.value(1)
    blink(ledr, 2, 10)
    blink(ledg, 2, 20)
    ledg.value(0)  # indicate active
except:
    print("JETI BOOT - ERROR: LED setup failed")


# main script to run after this one
# if not specified "main.py" will be executed
