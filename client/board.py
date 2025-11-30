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
        
        cell_width = GAME_WIDTH / BOARD_SIZE
        cell_height = WINDOW_HEIGHT / BOARD_SIZE

        for row in range(BOARD_SIZE):
            for col in range(BOARD_SIZE):
                # Draw Stones (Positions calculated in set_state)
                if isinstance(self.grid[row][col], Stone):
                    self.grid[row][col].draw()
                
                # Draw Hints (Calculated here)
                elif self.grid[row][col] == 3:
                    # FIX: CenterX uses COL, CenterY uses ROW
                    # FIX: We must invert ROW because Arcade draws Y-Up, C++ is Y-Down
                    inverted_row = (BOARD_SIZE - 1) - row
                    
                    centerX = (col * cell_width) + (cell_width / 2)
                    centerY = (inverted_row * cell_height) + (cell_height / 2)
                    
                    width = cell_width * 0.6
                    height = cell_height * 0.6
                    create_ellipse_filled(centerX, centerY, width, height, arcade.color.ASH_GREY).draw()

    def set_state(self, state_string: str):
        """Set the board state from a string representation"""
        
        cell_width = GAME_WIDTH / BOARD_SIZE
        cell_height = WINDOW_HEIGHT / BOARD_SIZE

        # Iterate 0 to 63
        for i in range(len(state_string)):
            
            # 1. C++ Logic Coords (Top-Left Origin)
            row = i // BOARD_SIZE  # Row 0 is Top
            col = i % BOARD_SIZE   # Col 0 is Left
            
            token = int(state_string[i])

            # 2. Arcade Screen Coords (Bottom-Left Origin)
            # X matches Col directly
            posX = (col * cell_width) + (cell_width / 2)
            
            # Y must be inverted: Row 0 (Top) -> Y=7 (Top of screen)
            inverted_row = (BOARD_SIZE - 1) - row
            posY = (inverted_row * cell_height) + (cell_height / 2)

            # 3. Store in Grid
            if token == 0:
                self.grid[row][col] = 0
            elif token == 1:
                self.grid[row][col] = Stone(arcade.color.BLACK, posX, posY)
            elif token == 2:
                self.grid[row][col] = Stone(arcade.color.WHITE, posX, posY)
            elif token == 3:
                self.grid[row][col] = 3