import arcade
from gameview import GameView

SPRITE_SCALING = 0.5


WINDOW_WIDTH = 1280

WINDOW_HEIGHT = 720

WINDOW_TITLE = "MAZE RUNNER"


MOVEMENT_SPEED = 3

def main():

    """ Main function """

    # Create a window class. This is what actually shows up on screen

    window = arcade.Window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE)


    # Create and setup the GameView

    game = GameView()

    game.setup()


    # Show GameView on screen

    window.show_view(game)


    # Start the arcade game loop
    arcade.run()


if __name__ == "__main__":
    main()
