
import framebuf


def create_bar_buffer(width, height):
    return framebuf.FrameBuffer(bytearray(width * height // 8), width, height, framebuf.MONO_VLSB)

def draw_bar_graph(display, buffer, x, y, width, height, value, min_value, max_value, previous_fill=None):
    """
    Draws a horizontal bar graph on the given framebuffer and blits it to the display.
    :param display: OLED display instance (e.g. SH1106)
    :param buffer: framebuffer created via create_bar_buffer()
    :param x, y: top-left corner on display
    :param width, height: dimensions of the bar graph
    :param value: current value
    :param min_value: minimum possible value
    :param max_value: maximum possible value
    :param previous_fill: last filled width (optional optimization)
    :return: the new filled width
    """
    # Clear and draw outer rectangle
    buffer.fill(0)
    buffer.rect(0, 0, width, height, 1)

    # Clamp and normalize value
    value = max(min_value, min(value, max_value))
    range_value = max_value - min_value
    relative_value = (value - min_value) / range_value
    relative_value = max(0, min(relative_value, 1))
    filled_width = int(relative_value * (width - 2))  # -2 for border

    # Draw fill
    buffer.fill_rect(1, 1, filled_width, height - 2, 1)

    # Blit to display
    display.blit(buffer, x, y)
    return filled_width
