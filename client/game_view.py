import arcade
import arcade.gui
from board import Board
from stone import Stone

GAME_WIDTH = 900
WINDOW_WIDTH = 1280
WINDOW_HEIGHT = 720
BOARD_SIZE = 8
# --- New constant for the UI sidebar ---
UI_WIDTH = WINDOW_WIDTH - GAME_WIDTH  # This is 200px


class GameView(arcade.View):
    """
    Main application class.
    """

    def __init__(self, username_one: str = "Player 1", username_two: str = "Player 2"):
        """
        Initializer
        """
        # Call the parent class initializer
        super().__init__()

        # --- GUI Manager ---
        self.manager = arcade.gui.UIManager()

        # Variables that will hold sprite lists
        self.current_player = 1
        
        # --- FIX: Save both usernames ---
        self.username_one = username_one
        self.username_two = username_two

        # --- Store UI labels to update them later ---
        self.player1_score_label = None
        self.player2_score_label = None

        # Set up the player info
        self.board = None

        # Set the background color
        self.background_color = arcade.color.AMAZON
        self.setup_game()

    def setup_game(self):
        """ Set up the game and initialize the variables. """
        # Sprite lists
        self.board = Board()
        self.board.check_possible_moves(self.current_player)
        
        # --- Call the UI setup ---
        self.prepare_ui()

    # --- ADDED: on_show_view ---
    def on_show_view(self):
        """ This is run when we switch to this view """
        # Enable the UIManager
        self.manager.enable()
        # Set the background color
        arcade.set_background_color(self.background_color)

    # --- ADDED: on_hide_view ---
    def on_hide_view(self):
        """ This is run when we switch away from this view """
        # Disable the UIManager
        self.manager.disable()

    def on_draw(self):
        """
        Render the screen.
        """
        # This command has to happen before we start drawing
        self.clear()

        # Draw all the sprites.
        self.board.draw()

        # --- ADDED: Draw the GUI ---
        # This draws all the UI elements (labels, buttons, etc)
        self.manager.draw()

    def on_update(self, delta_time):
        """ Movement and game logic """
        
        if self.player1_score_label:
            score1 = self.board.get_score(1) # Assuming you add this method
            self.player1_score_label.text = str(score1)
    
        if self.player2_score_label:
            score2 = self.board.get_score(2)
            self.player2_score_label.text = str(score2)
        
        return

    def on_mouse_press(self, x, y, button, key_modifiers):
        """
        Called when the user presses a mouse button.
        """
        
        # --- ADDED: Check if the click was on the UI ---
        # If the click is on the UI, let the UIManager handle it and stop
        if self.manager.on_mouse_press(x, y, button, key_modifiers):
            return
            
        # --- ADDED: Check if the click is outside the game board ---
        if x > GAME_WIDTH:
            print("Clicked on UI area, not game board.")
            return

        if button == arcade.MOUSE_BUTTON_LEFT:
            indexX = int(x // (GAME_WIDTH / BOARD_SIZE))
            indexY = int(y // (WINDOW_HEIGHT / BOARD_SIZE))

            is_valid, moves = self.board.check_rules(indexX, indexY, self.current_player)
            if not is_valid:
                return

            print(f"Click coordinates: ({x}, {y})")
            centerX = (indexX * (GAME_WIDTH / BOARD_SIZE)) + (GAME_WIDTH / BOARD_SIZE) / 2
            centerY = (indexY * (WINDOW_HEIGHT / BOARD_SIZE)) + (WINDOW_HEIGHT / BOARD_SIZE) / 2

            color = arcade.color.BLACK if self.current_player == 1 else arcade.color.WHITE
            self.board.grid[indexY][indexX] = Stone(color, centerX, centerY)
            self.board.flip_stones(indexX, indexY, moves, self.current_player)
            self.current_player = 2 if self.current_player == 1 else 1
            can_continue = self.board.check_possible_moves(self.current_player)
            while can_continue == False:
                # self.board.flip_stones(indexX, indexY, moves, self.current_player) # <-- This looks like a bug, removing it.
                print(f"Player {self.current_player} has no moves, skipping turn.")
                # Switch player again
                self.current_player = 2 if self.current_player == 1 else 1
                can_continue = self.board.check_possible_moves(self.current_player)
                
                # TODO: Check for game end (neither player can move)

    def _create_player_ui(self, username: str, player_color: tuple) -> (arcade.gui.UIBoxLayout, arcade.gui.UILabel):
        """
        Helper function to create a UI block for a player.
        Returns a (UIBoxLayout, score_label_widget) tuple.
        """
        # --- MODIFIED ---
        # Create a vertical box for this player
        # Added padding and bg_color directly here
        player_box = arcade.gui.UIBoxLayout(
            vertical=True, 
            space_between=5,
            padding=(10, 10, 10, 10),
            bg_color=(40, 40, 40, 150) # Dark, semi-transparent
        )

        # Username Label (with player's stone color)
        username_label = arcade.gui.UILabel(
            text=username,
            font_size=18,
            font_name="Arial",
            text_color=player_color if player_color != arcade.color.WHITE else arcade.color.LIGHT_GRAY,
            bold=True,
            align="center"
        )
        player_box.add(username_label)

        # "Score:" text
        score_text_label = arcade.gui.UILabel(
            text="Score:",
            font_size=14,
            font_name="Arial",
            text_color=arcade.color.WHITE,
            align="center"
        )
        player_box.add(score_text_label)

        # Score "0"
        score_value_label = arcade.gui.UILabel(
            text="0",
            font_size=22,
            font_name="Arial",
            text_color=arcade.color.WHITE,
            bold=True,
            align="center"
        )
        player_box.add(score_value_label)

        # --- REMOVED ---
        # The UIBoxGroup wrapper was incorrect.
        # player_ui_group = arcade.gui.UIBoxGroup(
        #     child=player_box,
        #     padding=(10, 10, 10, 10),
        #     bg_color=(40, 40, 40, 150) # Dark, semi-transparent
        # )

        # Return the group and the score label (so it can be updated)
        # --- MODIFIED: Return player_box directly ---
        return player_box, score_value_label

    # --- FINISHED: prepare_ui ---
    def prepare_ui(self):
        """ Prepare the UI elements """
        
        # This main layout will anchor our UI to the top right
        self.ui_anchor_layout = arcade.gui.UIAnchorLayout()

        # Create the main vertical box for the UI sidebar
        # Use UI_WIDTH with a small margin (e.g., 180)
        ui_box = arcade.gui.UIBoxLayout(
            vertical=True,
            width=UI_WIDTH - 20, # 180
            space_between=20
        )

        # --- Player 1 UI (Black) ---
        player1_group, self.player1_score_label = self._create_player_ui(
            username=f"{self.username_one} (Black)",
            player_color=arcade.color.BLACK
        )
        ui_box.add(player1_group)
        
        # --- Player 2 UI (White) ---
        player2_group, self.player2_score_label = self._create_player_ui(
            username=f"{self.username_two} (White)",
            player_color=arcade.color.WHITE
        )
        ui_box.add(player2_group)

        # Anchor the main UI box to the top right
        self.ui_anchor_layout.add(
            child=ui_box,
            anchor_x="right",
            anchor_y="top",
            align_x=-20, # Center in the sidebar
            align_y=-20  # 20px margin from the top
        )

        # Add the anchor layout to the manager
        self.manager.add(self.ui_anchor_layout)