# This script sets up the display for the project.

from machine import I2C
import gc

from .sh1106 import SH1106_I2C as OLED


# Setup an I2C interface for the OLED display
scl_pin = 7
sda_pin = 6
# i2c = SoftI2C(scl=scl_pin, sda=sda_pin)
i2c = I2C(1, scl=scl_pin, sda=sda_pin, freq=1000000)

# Setup the OLED display
oled_width = 128
oled_height = 64
gc.collect()  # Precaution before instantiating framebuf
oled = OLED(oled_width, oled_height, i2c)
print(f'OLED setup done ({oled_width}x{oled_height})')
