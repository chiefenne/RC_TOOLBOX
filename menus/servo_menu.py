# servo_menu.py

from screens import MenuScreen, ValueScreen
from screens import PWMIncrementScreen, PWMMinScreen, PWMMaxScreen
from actions import ServoPWMAction
from language import language_manager as lm


def build_servo_menu():
    menu = MenuScreen(lm.translate('SERVO'))

    servo_pwm = ValueScreen(
        lm.translate('Servo PWM'),
        initial_value=1500,
        increment=10,
        bargraph=True,
        min_value=1000,
        max_value=2000
    )
    servo_pwm.action = ServoPWMAction(
        pin_num=26,
        freq=50,
        min_us=1000,
        max_us=2000,
        immediate=True
    )

    pwm_increment = PWMIncrementScreen(
        lm.translate('PWM Increment'),
        initial_value=10,
        increment=1,
        min_value=1,
        max_value=100
    )

    pwm_min = PWMMinScreen(
        lm.translate('PWM Minimum'),
        initial_value=1000,
        increment=10,
        min_value=900,
        max_value=1100
    )

    pwm_max = PWMMaxScreen(
        lm.translate('PWM Maximum'),
        initial_value=2000,
        increment=10,
        min_value=1900,
        max_value=2100
    )

    menu.add_child(servo_pwm)
    menu.add_child(pwm_increment)
    menu.add_child(pwm_min)
    menu.add_child(pwm_max)

    return menu
