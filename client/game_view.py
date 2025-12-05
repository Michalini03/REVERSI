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
RIGHT_MARGIN = UI_WIDTH // 10

PREFIX = "REV"


class GameView(arcade.View):
    """
    Main application class.
    """

    def __init__(self, client_socket, server_queue, current_player: int, username_one: str = "Player 1", username_two: str = "Player 2", lobby_id: int = -1, init_state: str = ""):
        """
        Initializer
        """
        # Call the parent class initializer
        super().__init__()

        # --- GUI Manager ---
        self.manager = arcade.gui.UIManager()

        # Variables that will hold sprite lists
        self.current_player = current_player

        self.lobby_id = lobby_id
        self.username_one = username_one
        self.username_two = username_two

        self.client_socket = client_socket
        self.server_queue = server_queue

        # --- Store UI labels to update them later ---
        self.player1_score_label = None
        self.player2_score_label = None

        # Set up the player info
        self.board = Board()
        self.init_state = init_state

        # Set the background color
        self.background_color = arcade.color.AMAZON

    def setup_game(self):
        """ Set up the game and initialize the variables. """
        self.board = Board()
        self.prepare_ui()

    # --- ADDED: on_show_view ---
    def on_show_view(self):
        """ This is run when we switch to this view """
        # Enable the UIManager
        self.manager.enable()
        self.setup_game()
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

        if self.board:
            self.board.draw()
        self.manager.draw()
        
    def show_server_error_popup(self):
        """
        Shows an Error modal popup using UIMessageBox 
        """
        self.manager.focused_element = None 
        
        message_box = arcade.gui.UIMessageBox(
            width=350,
            height=200,
            message_text="Disconnected from server.",
            buttons=["OK"]
        )
        
        @message_box.event("on_action")
        def on_message_box_close(event):
            self.server_queue = None
            from lobby_view import LobbyView
            self.window.show_view(LobbyView())
            pass
        
        self.manager.add(message_box)
    
    def show_pause_modal(self, player_name: str):
        """
        Shows an Error modal popup using UIMessageBox 
        """

        pause_box = arcade.gui.UIMessageBox(
            width=350,
            height=200,
            message_text=f"Player {player_name} was disconnected from the game\nWaiting for reconnection.",
            buttons=["Leave"]
        )
        
        @pause_box.event("on_action")
        def on_message_box_close(event):
            from lobby_list_view import LobbyListView
            self.window.show_view(LobbyListView(5, self.client_socket, self.server_queue))
            pass
        
        self.manager.add(pause_box)


    def on_update(self, delta_time):
        """ Movement and game logic """
        
        if self.client_socket is None or self.server_queue is None:
            return
        
        while not self.server_queue.empty():
            message = self.server_queue.get()
            
            if message is None or message.strip() == "":
                    continue
            print(f"[GameView] Message from server: {message}")
            # --- PROCESS SERVER MESSAGES HERE ---
            
            params = message.split()
            if params[0] == PREFIX:
                command = params[1]
                if command == "STATE":
                    self.board.set_state(params[2])
                    score1 = params[3]
                    score2 = params[4] 
                    if self.player1_score_label:
                        self.player1_score_label.text = str(score1)
                    if self.player2_score_label:
                        self.player2_score_label.text = str(score2)                        
                elif command == "SERVER_DISCONNECT":
                    print("[GameView] Disconnected from server.")
                    self.show_server_error_popup()
                elif command == "DISCONNECT":
                    player_name = params[2]
                    self.show_pause_modal()
                elif command == "RECONENCT":
                    return
                else:
                    print("[GameView] Unexpected command from server.")

    def on_mouse_press(self, x, y, button, key_modifiers):
        """
        Called when the user presses a mouse button.
        """
        
        # If the click is on the UI, let the UIManager handle it and stop
        if self.manager.on_mouse_press(x, y, button, key_modifiers):
            return
            
        # Check if the click is outside the game board
        if x > GAME_WIDTH:
            return

        if button == arcade.MOUSE_BUTTON_LEFT:
            indexX = int(x // (GAME_WIDTH / BOARD_SIZE))
            indexY = 7 - int(y // (WINDOW_HEIGHT / BOARD_SIZE))
            print(f"[GameView] Mouse clicked at board position: ({indexX}, {indexY})")
            self.send_move(indexX, indexY)

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
            width=UI_WIDTH - RIGHT_MARGIN, # 180
            space_between = RIGHT_MARGIN
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
            align_x=-RIGHT_MARGIN, # Center in the sidebar
            align_y=-RIGHT_MARGIN  # 20px margin from the top
        )

        # Add the anchor layout to the manager
        self.manager.add(self.ui_anchor_layout)
        
    def send_move(self, board_x: int, board_y: int):
        """ Send the player's move to the server """
        message = f"{PREFIX} MOVE {board_x} {board_y} {self.lobby_id}\n"
        print(f"[GameView] Sending move to server: {message}")
        self.client_socket.sendall(message.encode())