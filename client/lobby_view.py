import arcade
import arcade.gui
from lobby_list_view import LobbyListView

GAME_PREFIX = "REV"

class LobbyView(arcade.View):
    """
    Lobby view class.
    Handles the UI for the game lobby.
    """

    def __init__(self, client_socket, server_queue):
        """ Initializer """
        # Call the parent class initializer
        super().__init__()

        # Create a UIManager
        self.manager = arcade.gui.UIManager()  

        # Variable to hold the username
        self.username = ""
        # UI elements
        self.username_input = None
        self.start_button = None
        self.anchor_layout = None

        # Background color
        self.background_color = arcade.color.AMAZON

        # Call setup
        self.setup_lobby()

        self.client_socket = client_socket
        self.server_queue = server_queue

    def setup_lobby(self):
        """ Set up the lobby view. """
        # Create a layout to anchor elements
        self.anchor_layout = arcade.gui.UIAnchorLayout()

        title_label = arcade.gui.UILabel(
            text="Welcome to the Lobby",
            font_size=30,
            font_name="Arial",
            text_color=arcade.color.WHITE
        )

        self.anchor_layout.add(
            child=title_label,
            anchor_x="center",
            anchor_y="center",
            align_y=150
        )

        self.username_input = arcade.gui.UIInputText(
            width=250,
            height=40,
            text="Enter Username",
            font_size=16
        )
        self.anchor_layout.add(
            child=self.username_input, 
            anchor_x="center", 
            anchor_y="center", 
            align_y=0
        )

        self.start_button = arcade.gui.UIFlatButton(
            text="Start Game",
            width=250,
            height=40
        )

        self.anchor_layout.add(
            child=self.start_button,
            anchor_x="center",
            anchor_y="center",
            align_y=-60
        )

        # Add the layout to the UIManager
        self.manager.add(self.anchor_layout)

        # --- Event Handler ---
        # Assign the 'on_click' event for the start button
        @self.start_button.event("on_click")
        def on_click_start(event):
            # Get the username from the input field
            self.username = self.username_input.text
            print(f"Username entered: {self.username}")

            # Here, you would typically switch to the main game view
            # Example (assuming you have a 'GameView' class):
            # game_view = GameView(self.username) # Pass username to the game
            # self.window.show_view(game_view)
            message: str = f"{GAME_PREFIX} CREATE {self.username}"
            self.client_socket.sendall(message.encode('utf-8'))

    def on_show_view(self):
        """ This is run when we switch to this view """
        # Enable the UIManager
        # This is required for GUI elements to respond to events
        self.manager.enable()

        # Set the background color
        arcade.set_background_color(self.background_color)

    def on_hide_view(self):
        """ This is run when we switch away from this view """
        # Disable the UIManager
        # This prevents it from handling events when hidden
        self.manager.disable()

    def on_draw(self):
        """ Render the screen. """
        self.clear()
        self.manager.draw()

    def on_update(self, delta_time):
        """ Update method called periodically. """
        while not self.server_queue.empty():
            message = self.server_queue.get()
            if message.startswith(GAME_PREFIX):
                # Process game-related messages here
                params = message.split()
                command: str = params[1]
                if command == "LOBBY":
                    print("[LobbyView] Received lobby list from server.")
                    lobby_count: int = int(params[2])
                    self.window.show_view(LobbyListView(lobby_count, self.client_socket, self.server_queue))
