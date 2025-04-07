# encoder/setup.py

from machine import Pin

from encoder import KY040
from settings import settings


def setup_encoder(screen_manager):
    # Define GPIO pins
    clk_pin = Pin(settings.get("encoder_clk_pin"), Pin.IN, Pin.PULL_UP)
    dt_pin = Pin(settings.get("encoder_dt_pin"), Pin.IN, Pin.PULL_UP)
    sw_pin = Pin(settings.get("encoder_sw_pin"), Pin.IN, Pin.PULL_UP)

    return KY040(
        screen_manager=screen_manager,
        encoder_clk=clk_pin,
        encoder_dt=dt_pin,
        encoder_sw=sw_pin,
        encoder_step=2,      # Filters jitter
        half_step=True,
        debounce_ms=55,      # Debounce timing
        step_ms=200,         # Normal step delay
        fast_ms=35,          # Fast rotate threshold
        hold_ms=1000,        # Long press
        click_ms=400         # Max time for single click
    )
