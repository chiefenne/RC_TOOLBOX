
from machine import Pin
from drivers.ky040.rotary_encoder_rp2 import RotaryEncoderRP2
from drivers.ky040.rotary_encoder import RotaryEncoderEvent
import uasyncio as asyncio


DEBUG = False

class KY040:

    def __init__(self,
                 screen_manager,
                 encoder_clk=4,
                 encoder_dt=2,
                 encoder_sw=1,
                 encoder_step=2,
                 half_step=False,
                 debounce_ms=50,
                 hold_ms=1000,
                 step_ms=200,
                 fast_ms=50,
                 click_ms=400):

        self.screen_manager = screen_manager

        # Create the rotary encoder object
        self.encoder = RotaryEncoderRP2(
            pin_clk=encoder_clk,
            pin_dt=encoder_dt,
            pin_sw=encoder_sw,
            half_step=half_step,
            debounce_ms=debounce_ms,
            encoder_step=encoder_step,
            hold_ms=hold_ms,
            step_ms=step_ms,
            fast_ms=fast_ms,
            click_ms=click_ms
        )

        self.handlers()

        # Start the encoder event loop
        asyncio.run(self.encoder.async_tick())

    # Listeners
    def any_event_listener(self, event, clicks):
        if DEBUG:
            print(f"ANY Event ID: {event} Clicks: {clicks}")

    def single_click_listener(self):
        if DEBUG:
            print(f"Single Click")
        self.screen_manager.handle_input("single_click")

    def multy_click_listener(self, clicks):
        if DEBUG:
            print(f"Multiple Clicks: {clicks}")
        if clicks == 2:
            self.screen_manager.handle_input("double_click")
        elif clicks == 3:
            self.screen_manager.handle_input("triple_click")

    def held_listener(self):
        if DEBUG:
            print(f"Press and hold")
        self.screen_manager.handle_input("long_press")

    def released_listener(self):
        if DEBUG:
            print(f"Released")

    def turn_left_listener(self):
        if DEBUG:
            print(f"Turn Left")
        self.screen_manager.handle_input("left")

    def turn_left_hold(self):
        if DEBUG:
            print(f"Turn Left Hold")

    def turn_left_fast_listener(self):
        if DEBUG:
            print(f"Turn Left Fast")
        self.screen_manager.handle_input("left_fast")

    def turn_left_super_fast_listener(self):
        if DEBUG:
            print(f"Turn Left Super Fast")
        self.screen_manager.handle_input("left_super_fast")

    def turn_left_fast_hold(self):
        if DEBUG:
            print(f"Turn Left Fast Hold")

    def turn_right_listener(self):
        if DEBUG:
            print(f"Turn Right")
        self.screen_manager.handle_input("right")

    def turn_right_hold(self):
        if DEBUG:
            print(f"Turn Right Hold")
        if DEBUG:
            print(f"Turn Right Hold")

    def turn_right_fast_listener(self):
        if DEBUG:
            print(f"Turn Right Fast")
        self.screen_manager.handle_input("right_fast")

    def turn_right_super_fast_listener(self):
        if DEBUG:
            print(f"Turn Right Super Fast")
        self.screen_manager.handle_input("right_super_fast")

    def turn_right_fast_hold(self):
        if DEBUG:
            print(f"Turn Right Fast Hold")

    def handlers(self):
        # subscribe to events
        # encoder.on(RotaryEncoderEvent.ANY, any_event_listener)
        self.encoder.on(RotaryEncoderEvent.CLICK, self.single_click_listener)
        self.encoder.on(RotaryEncoderEvent.MULTIPLE_CLICK, self.multy_click_listener)
        self.encoder.on(RotaryEncoderEvent.HELD, self.held_listener)
        self.encoder.on(RotaryEncoderEvent.RELEASED, self.released_listener)
        self.encoder.on(RotaryEncoderEvent.TURN_LEFT, self.turn_left_listener)
        self.encoder.on(RotaryEncoderEvent.TURN_LEFT_HOLD, self.turn_left_hold)
        self.encoder.on(RotaryEncoderEvent.TURN_LEFT_FAST, self.turn_left_fast_listener)
        self.encoder.on(RotaryEncoderEvent.TURN_LEFT_SUPER_FAST, self.turn_left_super_fast_listener)
        self.encoder.on(RotaryEncoderEvent.TURN_LEFT_FAST_HOLD, self.turn_left_fast_hold)
        self.encoder.on(RotaryEncoderEvent.TURN_RIGHT, self.turn_right_listener)
        self.encoder.on(RotaryEncoderEvent.TURN_RIGHT_HOLD, self.turn_right_hold)
        self.encoder.on(RotaryEncoderEvent.TURN_RIGHT_FAST, self.turn_right_fast_listener)
        self.encoder.on(RotaryEncoderEvent.TURN_RIGHT_SUPER_FAST, self.turn_right_super_fast_listener)
        self.encoder.on(RotaryEncoderEvent.TURN_RIGHT_FAST_HOLD, self.turn_right_fast_hold)

