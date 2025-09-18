import arcade
from arcade.shape_list import create_ellipse_filled

WINDOW_WIDTH = 1280

WINDOW_HEIGHT = 720
BOARD_SIZE = 8


class Stone():
    """Stone Class"""

    def __init__(self, color, position_x, position_y):
        self.center_x = position_x
        self.center_y = position_y
        self.color = color
        self.id = 1 if color == arcade.color.BLACK else 2
        self.width = (WINDOW_WIDTH / BOARD_SIZE) * 0.8
        self.height = (WINDOW_HEIGHT / BOARD_SIZE) * 0.8

    def draw(self):
        """Draw the stone"""
        create_ellipse_filled(self.center_x, self.center_y, self.width, self.height, self.color).draw()
