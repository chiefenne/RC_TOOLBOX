from screens import SplashScreen

def show_splash():
    splash = SplashScreen(
        app_name="RC TOOLS",
        version="v1.0",
        company="(c) MHB Electronics",
        duration=3
    )
    splash.show()
