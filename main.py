'''
Menu screen manager for a simple menu system.
Navigation using the KY040 rotary encoder.

Author: Dipl.-Ing. A. Ennemoser
Date: 03-2025

'''

from machine import Pin

from encoder import KY040
from screens import ScreenManager, MenuScreen, ValueScreen, SplashScreen
from custom_screens import (
    PWMIncrementScreen,
    PWMMinScreen,
    PWMMaxScreen,
    WiFiSSIDScreen,
    WiFiPasswordScreen,
    LanguageScreen,
)


# Show splash screen first
splash = SplashScreen(
    app_name="SERVO Tester",
    version="v1.0",
    company="(c) MHB Electronics",
    duration=4
)
splash.show()

# Top-Level Menus
servo_menu = MenuScreen('SERVO')
wifi_menu = MenuScreen('WIFI')
settings_menu = MenuScreen('SETTINGS')
help_menu = MenuScreen('HELP')

# Servo Menu
servo_submenu_1 = ValueScreen('Servo PWM', initial_value=1500, increment=10,
                              bargraph=True, min_value=1000, max_value=2000)
servo_submenu_2 = PWMIncrementScreen('PWM Increment', initial_value=10, increment=1,
                                     min_value=1, max_value=100)
servo_submenu_3 = PWMMinScreen('PWM Minimum', initial_value=1000, increment=10,
                               min_value=900, max_value=1100)
servo_submenu_4 = PWMMaxScreen('PWM Maximum', initial_value=2000, increment=10,
                               min_value=1900, max_value=2100)
servo_menu.add_child(servo_submenu_1)
servo_menu.add_child(servo_submenu_2)
servo_menu.add_child(servo_submenu_3)
servo_menu.add_child(servo_submenu_4)

# WiFi Menu
wifi_ssid = WiFiSSIDScreen('WiFi SSID', initial_value='MySSID')
wifi_password = WiFiPasswordScreen('WiFi Password', initial_value='MyPassword')
wifi_menu.add_child(wifi_ssid)
wifi_menu.add_child(wifi_password)

# Settings Menu
settings_language = LanguageScreen('Language')
option2 = ValueScreen('Option 2')
settings_menu.add_child(settings_language)
settings_menu.add_child(option2)

# Help Menu
help_option1 = ValueScreen('Help Option 1')
help_option2 = ValueScreen('Help Option 2')
help_menu.add_child(help_option1)
help_menu.add_child(help_option2)

# Create the Screen Manager
top_level_screens = [servo_menu, wifi_menu, settings_menu, help_menu]
screen_manager = ScreenManager(top_level_screens)

# Encoder Setup
encoder_clk = Pin(4, Pin.IN, Pin.PULL_UP)
encoder_dt = Pin(2, Pin.IN, Pin.PULL_UP)
encoder_sw = Pin(1, Pin.IN, Pin.PULL_UP)

ky40 = KY040(
    screen_manager=screen_manager,
    encoder_clk=encoder_clk,
    encoder_dt=encoder_dt,
    encoder_sw=encoder_sw,
    encoder_step=2, # used to filter out pin jitter
    half_step=True,
    debounce_ms=55, # used to filter out pin jitter
    step_ms=200,
    fast_ms=35,
    hold_ms=1000,
    click_ms=400
)
