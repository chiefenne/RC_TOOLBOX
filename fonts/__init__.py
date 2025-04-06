
from fonts import Arial_12
from fonts import Arial_14
from fonts import Arial_16
from fonts import Arial_Bold_12
from fonts import Arial_Bold_14
from fonts import Arial_Bold_16
from fonts import Courier_New_12
from fonts import Courier_New_14
from fonts import Courier_New_16

FONTS = {
    "Arial_12": Arial_12,
    "Arial_14": Arial_14,
    "Arial_16": Arial_16,
    "Arial_Bold_12": Arial_Bold_12,
    "Arial_Bold_14": Arial_Bold_14,
    "Arial_Bold_16": Arial_Bold_16,
    "Courier_New_12": Courier_New_12,
    "Courier_New_14": Courier_New_14,
    "Courier_New_16": Courier_New_16,
}

# check that all imports are available in FONTS
for font_name, font in FONTS.items():
    if font_name not in globals():
        raise ImportError(f"Font {font_name} not found in globals")
    if font_name != font.__name__.split('.')[-1]:
        raise ImportError(f"Font {font_name} does not match its module name {font.__name__.split('.')[-1]}")
