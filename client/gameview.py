import arcade
from board import Board
from stone import Stone

WINDOW_WIDTH = 1280
WINDOW_HEIGHT = 720
BOARD_SIZE = 8


class GameView(arcade.View):

    """
    Main application class.
    """

    def __init__(self):
        """
        Initializer
        """

        # Call the parent class initializer

        super().__init__()

        # Variables that will hold sprite lists
        self.current_player = 1

        # Set up the player info

        # Set the background color
        self.background_color = arcade.color.AMAZON

    def setup(self):
        """ Set up the game and initialize the variables. """
        # Sprite lists
        self.board = Board()

    def on_draw(self):
        """
        Render the screen.
        """

        # This command has to happen before we start drawing
        self.clear()

        # Draw all the sprites.
        self.board.draw()

    def on_update(self, delta_time):
        """ Movement and game logic """
        return

    def on_key_press(self, key, modifiers):
        if key == arcade.key.ESCAPE:
            arcade.close_window()
        return

    def on_key_release(self, key, modifiers):
        return

    def on_mouse_press(self, x, y, button, key_modifiers):

        """
        Called when the user presses a mouse button.
        """

        if button == arcade.MOUSE_BUTTON_LEFT:

            indexX = int(x // (WINDOW_WIDTH / BOARD_SIZE))
            indexY = int(y // (WINDOW_HEIGHT / BOARD_SIZE))

            is_valid, moves = self.board.check_rules(indexX, indexY, self.current_player)
            if not is_valid:
                return

            print(f"Click coordinates: ({x}, {y})")
            centerX = (indexX * (WINDOW_WIDTH / BOARD_SIZE)) + (WINDOW_WIDTH / BOARD_SIZE) / 2
            centerY = (indexY * (WINDOW_HEIGHT / BOARD_SIZE)) + (WINDOW_HEIGHT / BOARD_SIZE) / 2

            color = arcade.color.BLACK if self.current_player == 1 else arcade.color.WHITE
            self.board.grid[indexY][indexX] = Stone(color, centerX, centerY)
            self.board.flip_stones(indexX, indexY, moves, self.current_player)
            self.current_player = 2 if self.current_player == 1 else 1
