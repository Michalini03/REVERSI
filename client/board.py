from arcade.shape_list import ShapeElementList, create_rectangle_outline
from stone import Stone
import arcade

MAP_SIZE = 8
WINDOW_WIDTH = 1280
WINDOW_HEIGHT = 720

class Board(ShapeElementList):
    """Map Class"""

    def __init__(self):
        super().__init__()
        self.grid = [[0 for _ in range(MAP_SIZE)] for _ in range(MAP_SIZE)]
        self.create_walls()
        self.setup_initial_stones()

    def create_walls(self):
        """Create wall shapes based on the grid"""
        wall_color = arcade.color.BLACK
        wall_width = WINDOW_WIDTH / MAP_SIZE
        wall_height = WINDOW_HEIGHT / MAP_SIZE

        for row in range(MAP_SIZE):
            for col in range(MAP_SIZE):
                x = col * wall_width + wall_width / 2
                y = row * wall_height + wall_height / 2
                wall = create_rectangle_outline(x, y, wall_width, wall_height, wall_color, 4)
                self.append(wall)

    def setup_initial_stones(self):
        """Set up the initial four stones in the center of the board"""
        center = MAP_SIZE // 2
        positions = [
            (center - 1, center - 1, arcade.color.WHITE),
            (center, center, arcade.color.WHITE),
            (center - 1, center, arcade.color.BLACK),
            (center, center - 1, arcade.color.BLACK),
        ]

        for row, col, color in positions:
            posX = (col * (WINDOW_WIDTH / MAP_SIZE)) + (WINDOW_WIDTH / MAP_SIZE) / 2
            posY = (row * (WINDOW_HEIGHT / MAP_SIZE)) + (WINDOW_HEIGHT / MAP_SIZE) / 2
            self.grid[row][col] = Stone(color, posX, posY)

    def draw(self):
        """Draw the board"""
        super().draw()
        for row in range(MAP_SIZE):
            for col in range(MAP_SIZE):
                if isinstance(self.grid[row][col], Stone):
                    self.grid[row][col].draw()

    def check_rules(self, indexX, indexY, color):
        """
        Check if placing a stone at (indexX, indexY) is valid in Reversi.

        Args:
            indexX (int): Column index (0-7)
            indexY (int): Row index (0-7)
            color (int): 1 for black, 2 for white

        Returns:
            bool: True if move is valid, False otherwise
        """
        if self.grid[indexY][indexX] != 0:
            return False  # Cell is not empty

        opponent = 2 if color == 1 else 1
        valid_move = False

        # Directions: N, NE, E, SE, S, SW, W, NW
        directions = [(-1, -1), (-1, 0), (-1, 1),
                      (0, -1),           (0, 1),
                      (1, -1),  (1, 0),  (1, 1)]

        print(f"Checking move at ({indexX}, {indexY}) for player {color}")

        final_dx, final_dy, size = [], [], 0
        possible_moves: dict = {}

        for dx, dy in directions:

            size = 0
            x, y = indexX + dx, indexY + dy
            found_opponent = False

            while 0 <= x < 8 and 0 <= y < 8:
                if self.grid[y][x] == 0:
                    break
                elif self.grid[y][x].id == opponent:
                    found_opponent = True
                    size += 1
                    x += dx
                    y += dy
                elif self.grid[y][x].id == color:
                    if found_opponent:
                        possible_moves[(dx, dy)] = size
                        valid_move = True
                    break

        print(f"Move valid: {valid_move} in direction ({final_dx}, {final_dy}) flipping {size} stones")
        return valid_move, possible_moves

    def flip_stones(self, indexX, indexY, moves, color):
        """Flip opponent stones in the direction (dx, dy)"""
        for (dx, dy), size in moves.items():
            x, y = indexX + dx, indexY + dy

            for _ in range(size):
                print(f"Flipping stone at ({x}, {y}) to color {color}")
                self.grid[y][x].id = color
                self.grid[y][x].color = arcade.color.BLACK if color == 1 else arcade.color.WHITE
                x += dx
                y += dy
