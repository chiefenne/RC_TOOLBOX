
from fonts import arial8
from fonts import arial10
from fonts import arial12
from fonts import arial14
from fonts import arial16
from fonts import arial_bold12
from fonts import arial_bold14
from fonts import arial_bold16
from fonts import courier_new10
from fonts import courier_new12
from fonts import courier_new14
from fonts import courier_new16


FONTS = {
    "arial8": arial8,
    "arial10": arial10,
    "arial12": arial12,
    "arial14": arial14,
    "arial16": arial16,
    "arial_bold12": arial_bold12,
    "arial_bold14": arial_bold14,
    "arial_bold16": arial_bold16,
    "courier_new10": courier_new10,
    "courier_new12": courier_new12,
    "courier_new14": courier_new14,
    "courier_new16": courier_new16,
}

# check that all imports are available in FONTS
for font_name, font in FONTS.items():
    if font_name not in globals():
        raise ImportError(f"Font {font_name} not found in globals")
    if font_name != font.__name__.split('.')[-1]:
        raise ImportError(f"Font {font_name} does not match its module name {font.__name__.split('.')[-1]}")
