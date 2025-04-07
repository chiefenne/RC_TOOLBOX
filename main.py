"""
RC Toolbox

A collection of tools to test/adjust servos, measure flap deflection or angles
between wing chord and vertical stabilizer, etc.

Author: Dipl.-Ing. A. Ennemoser
Date: 03-2025
"""
from machine import Pin

from splash import show_splash
from screens import ScreenManager
from encoder.setup import setup_encoder
from menus import build_menus
from language import language_manager as lm
from settings import settings


# Apply saved language
lm.set_language(settings.get("language"))

# Show splash screen
show_splash()

# Build menus and screens
top_level_screens = build_menus()

# Setup screen manager
screen_manager = ScreenManager(top_level_screens)

# Setup encoder
ky40 = setup_encoder(screen_manager)
