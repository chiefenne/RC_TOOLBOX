
from .writer import Writer
from .fonts import *
from .fonts import FONTS


DEBUG = False


class TextRenderer:
    def __init__(self, display, font='Arial_14'):
        self.display = display
        self.set_font(font)

    def set_font(self, font):
        if DEBUG:
            print(f"Setting font: {font}")
        self.font = FONTS[font]
        if DEBUG:
            print(f"Font height: {self.font.height()}")
        self.writer = Writer(self.display, self.font, verbose=False)
        self.char_height = self.font.height()
        self.screen_width = self.display.width
        self.screen_height = self.display.height

    def draw(self, text, x=None, y=0, align="left", font=None, clear_screen=False, invert=False, show=False):
            if font and font != self.font:
                self.set_font(font)

            text_width = self.writer.stringlen(text)

            if x is None:
                if align == "left":
                    x = 0
                elif align == "center":
                    x = max(0, (self.screen_width - text_width) // 2)
                elif align == "right":
                    x = max(0, self.screen_width - text_width)
                else:
                    raise ValueError("align must be 'left', 'center', or 'right'")

            if clear_screen:
                self.display.fill(0)
            else:
                self.display.fill_rect(x, y, text_width, self.char_height, 0)

            Writer.set_textpos(self.display, y, x)
            self.writer.printstring(text, invert=invert)

            if show:
                self.display.show()