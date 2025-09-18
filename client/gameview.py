import arcade
from player import Player

SPRITE_SCALING = 0.5


WINDOW_WIDTH = 1280

WINDOW_HEIGHT = 720

WINDOW_TITLE = "Move Sprite with Keyboard Example"


MOVEMENT_SPEED = 3

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

        self.player_list = None


        # Set up the player info

        self.player_sprite = None


        # Set the background color

        self.background_color = arcade.color.AMAZON


    def setup(self):

        """ Set up the game and initialize the variables. """


        # Sprite lists

        self.player_list = arcade.SpriteList()


        # Set up the player

        self.player_sprite = Player(

            ":resources:images/animated_characters/female_person/femalePerson_idle.png",

            scale=SPRITE_SCALING,

        )

        self.player_two_sprite = Player(

            ":resources:images/animated_characters/female_person/femalePerson_idle.png",

            scale=SPRITE_SCALING,

        )

        self.player_sprite.center_x = 50
        self.player_sprite.center_y = 50

        self.player_two_sprite.center_x = 30
        self.player_two_sprite.center_y = 30

        self.player_list.append(self.player_sprite)
        self.player_list.append(self.player_two_sprite)


    def on_draw(self):

        """
        Render the screen.
      """


        # This command has to happen before we start drawing

        self.clear()


        # Draw all the sprites.

        self.player_list.draw()


    def on_update(self, delta_time):

        """ Movement and game logic """


        # Move the player

        self.player_list.update(delta_time)


    def on_key_press(self, key, modifiers):

        """Called whenever a key is pressed. """


        # If the player presses a key, update the speed

        if key == arcade.key.UP:

            self.player_sprite.change_y = MOVEMENT_SPEED

        elif key == arcade.key.DOWN:

            self.player_sprite.change_y = -MOVEMENT_SPEED

        elif key == arcade.key.LEFT:

            self.player_sprite.change_x = -MOVEMENT_SPEED

        elif key == arcade.key.RIGHT:

            self.player_sprite.change_x = MOVEMENT_SPEED


    def on_key_release(self, key, modifiers):

        """Called when the user releases a key. """


        # If a player releases a key, zero out the speed.

        # This doesn't work well if multiple keys are pressed.

        # Use 'better move by keyboard' example if you need to

        # handle this.

        if key == arcade.key.UP or key == arcade.key.DOWN:

            self.player_sprite.change_y = 0

        elif key == arcade.key.LEFT or key == arcade.key.RIGHT:

            self.player_sprite.change_x = 0