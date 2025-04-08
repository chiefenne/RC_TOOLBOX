
import time
from display import oled
from display import TextRenderer
from display import create_bar_buffer, draw_bar_graph
from language import language_manager as lm


# Text Renderer Setup
text_render = TextRenderer(oled, font='Arial_14')

DEBUG = False


class Screen:
    def __init__(self, name='Screen'):
        self.name = name
        self.parent = None
        self.children = []
        self.is_active = False

    def show(self):
        raise NotImplementedError('Subclasses must implement this method.')

    def handle_input(self, input_data, speed=None):
        raise NotImplementedError('Subclasses must implement this method.')

    def add_child(self, child):
        self.children.append(child)
        child.parent = self

    def activate(self):
        self.is_active = True
        if DEBUG:
            print(f'{self.name} activated')
        self.show()

    def deactivate(self):
        self.is_active = False
        if DEBUG:
            print(f'{self.name} deactivated')

    def navigate_to_child(self, index):
        if 0 <= index < len(self.children):
            return self.children[index]
        else:
            return self

    def navigate_to_parent(self):
        return self.parent

    def navigate_to_top(self):
        pass


class MenuScreen(Screen):
    def __init__(self, name='Menu'):
        super().__init__(name)
        self.current_selection = 0

    def show(self):
        if DEBUG:
            print(f"MenuScreen - self.name: {self.name}")
            print(f"MenuScreen - lm.translate(self.name): {lm.translate(self.name)}")
        text_render.draw(f'{lm.translate(self.name)}', y=20, align='center',
                         clear_screen=True, font='Arial_Bold_16', show=True)

    def handle_input(self, input_data, speed=None): # speed is not used here
        if DEBUG:
            print(f'MenuScreen {self.name} received input: {input_data}')
        if self.parent is None and input_data in ('left', 'right'):
            return False  # Let ScreenManager handle top-level switching

        if input_data == 'left':
            self.current_selection = (self.current_selection + 1) % len(self.children)
            self.show()
            return True
        elif input_data == 'right':
            self.current_selection = (self.current_selection - 1) % len(self.children)
            self.show()
            return True
        elif input_data == 'single_click':
            return self.navigate_to_child(self.current_selection)
        elif input_data in ('double_click', 'long_press'):
            return self.navigate_to_parent()
        return False


class ValueScreen(Screen):
    def __init__(self, name='Value', initial_value=0, increment=1,
                 bargraph=False, min_value=0, max_value=10, action=None):
        super().__init__(name)
        self.value = initial_value
        self.increment = increment  # Base/default increment
        self.bargraph = bargraph
        self.min_value = min_value
        self.max_value = max_value
        self.edit_mode = False
        self.action = action  # # attach an Action (can be None if no hardware action)

        if self.bargraph:
            self.bar_fb = create_bar_buffer(88, 8)
            self.last_filled_width = None

    def get_increment(self, input_type, speed=None):
        """Return the scaled increment for encoder input, optionally speed-aware."""
        if speed is not None:
            # Cap the maximum scaling if needed
            speed = min(max(speed, 1.0), 3.0)  # clamp between 1.0 and 3.0
            return int(self.increment * speed)

        # Fallback for label-based input (if speed wasn't passed)
        if input_type == 'right':
            return self.increment
        elif input_type == 'right_fast':
            return self.increment * 2
        elif input_type == 'right_super_fast':
            return self.increment * 3
        elif input_type == 'left':
            return -self.increment
        elif input_type == 'left_fast':
            return -self.increment * 2
        elif input_type == 'left_super_fast':
            return -self.increment * 3

        return 0

    def show(self):
        invert = self.edit_mode
        text_render.draw(f'{lm.translate(self.name)}', y=10, align='center',
                         clear_screen=True, font='Arial_Bold_16', show=False)
        text_render.draw(f'{self.value}', y=30, align='center',
                         clear_screen=False, font='Courier_New_16', invert=invert, show=False)

        if self.edit_mode and self.bargraph:
            self.last_filled_width = draw_bar_graph(
                oled, self.bar_fb, 20, 50, 88, 8,
                self.value, self.min_value, self.max_value,
                previous_fill=self.last_filled_width
            )

        oled.show()

    def handle_input(self, input_data, speed=None):
        if self.edit_mode:
            delta = self.get_increment(input_data, speed)
            if delta != 0:
                self.value += delta
                self.value = max(self.min_value, min(self.value, self.max_value))
                self.show()
                # Immediate action trigger
                if self.action:
                    self.action.on_value_changed(self.value)
                return True

            if input_data in ('double_click', 'long_press'):
                self.edit_mode = False
                self.show()
                # On-demand action trigger (or final confirmation)
                if self.action:
                    self.action.on_value_confirmed(self.value)
                return True

        else:
            if input_data == 'right':
                siblings = self.parent.children
                index = siblings.index(self)
                return siblings[(index + 1) % len(siblings)]
            elif input_data == 'left':
                siblings = self.parent.children
                index = siblings.index(self)
                return siblings[(index - 1) % len(siblings)]
            elif input_data == 'single_click':
                self.edit_mode = True
                self.show()
                return True
            elif input_data in ('double_click', 'long_press'):
                return self.parent

        return False


class SplashScreen:
    def __init__(self, app_name="MyApp", version="v1.0", company="My Company", duration=3):
        self.app_name = app_name
        self.version = version
        self.company = company
        self.duration = duration  # in seconds
        self.text_render = TextRenderer(oled, font='Arial_14')  # Optional: use bold for splash

    def show(self):
        oled.fill(0)
        self.text_render.draw(self.app_name, y=10, align='center', font='Arial_Bold_16', show=False)
        self.text_render.draw(self.version, y=30, align='center', font='Arial_14', show=False)
        self.text_render.draw(self.company, y=50, align='center', font='Arial_14', show=True)
        time.sleep(self.duration)


class ScreenManager:
    def __init__(self, top_level_screens):
        self.top_level_screens = top_level_screens
        self.current_top_level_index = 0
        self.current_screen = self.top_level_screens[self.current_top_level_index]
        self.current_screen.activate()

    def switch_to(self, new_screen):
        if self.current_screen is not new_screen:
            self.current_screen.deactivate()
            self.current_screen = new_screen
            self.current_screen.activate()

    def handle_input(self, input_data, speed=None):
        result = self.current_screen.handle_input(input_data, speed=speed)

        if input_data == 'triple_click': # go to top level
            # switch of edit mode of current value screen
            if isinstance(self.current_screen, ValueScreen):
                self.current_screen.edit_mode = False

            self.current_top_level_index = 0
            result = self.top_level_screens[self.current_top_level_index]

        if isinstance(result, Screen):
            self.switch_to(result)
            return

        if result in (False, None) and self.current_screen.parent is None:
            if input_data == 'right':
                self.current_top_level_index = (self.current_top_level_index + 1) % len(self.top_level_screens)
                self.switch_to(self.top_level_screens[self.current_top_level_index])
            elif input_data == 'left':
                self.current_top_level_index = (self.current_top_level_index - 1) % len(self.top_level_screens)
                self.switch_to(self.top_level_screens[self.current_top_level_index])

