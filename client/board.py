from arcade.shape_list import ShapeElementList, create_rectangle_outline, create_ellipse_filled
from stone import Stone
import arcade

BOARD_SIZE = 8
GAME_WIDTH = 900
WINDOW_HEIGHT = 720

class Board(ShapeElementList):
    """Map Class"""

    def __init__(self):
        super().__init__()
        self.grid = [[0 for _ in range(BOARD_SIZE)] for _ in range(BOARD_SIZE)]
        self.create_walls()
        self.setup_initial_stones()

    def create_walls(self):
        """Create wall shapes based on the grid"""
        wall_color = arcade.color.BLACK
        wall_width = GAME_WIDTH / BOARD_SIZE
        wall_height = WINDOW_HEIGHT / BOARD_SIZE

        for row in range(BOARD_SIZE):
            for col in range(BOARD_SIZE):
                x = col * wall_width + wall_width / 2
                y = row * wall_height + wall_height / 2
                wall = create_rectangle_outline(x, y, wall_width, wall_height, wall_color, 4)
                self.append(wall)


    def draw(self):
        """Draw the board"""
        super().draw()
        for row in range(BOARD_SIZE):
            for col in range(BOARD_SIZE):
                if isinstance(self.grid[row][col], Stone):
                    self.grid[row][col].draw()
                elif self.grid[row][col] == 3:
                    centerX = (row * (GAME_WIDTH / BOARD_SIZE)) + (GAME_WIDTH / BOARD_SIZE) / 2
                    centerY = (col * (WINDOW_HEIGHT / BOARD_SIZE)) + (WINDOW_HEIGHT / BOARD_SIZE) / 2
                    width = (GAME_WIDTH / BOARD_SIZE) * 0.6
                    height = (WINDOW_HEIGHT / BOARD_SIZE) * 0.6
                    create_ellipse_filled(centerX, centerY, width, height, arcade.color.ASH_GREY).draw()

    def set_state(self, state_string: str):
        """Set the board state from a string representation"""
        tokens = state_string.split()
        for row in range(BOARD_SIZE):
            for col in range(BOARD_SIZE):
                index = row * BOARD_SIZE + col
                token = int(tokens[index])
                posX = (col * (GAME_WIDTH / BOARD_SIZE)) + (GAME_WIDTH / BOARD_SIZE) / 2
                posY = (row * (WINDOW_HEIGHT / BOARD_SIZE)) + (WINDOW_HEIGHT / BOARD_SIZE) / 2

                if token == '0':
                    self.grid[row][col] = 0
                elif token == '1':
                    self.grid[row][col] = Stone(arcade.color.BLACK, posX, posY)
                elif token == '2':
                    self.grid[row][col] = Stone(arcade.color.WHITE, posX, posY)
                elif token == '3':
                    self.grid[row][col] = 3
