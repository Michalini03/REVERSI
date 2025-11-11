import arcade
import arcade.gui
from game_view import GameView

class LobbyView(arcade.View):
    """
    Lobby view class.
    Handles the UI for the game lobby.
    """

    def __init__(self):
        """ Initializer """
        # Call the parent class initializer
        super().__init__()

        # --- GUI ---
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

    def setup_lobby(self):
        """ Set up the lobby view. """
        
        # Create a layout to anchor elements
        self.anchor_layout = arcade.gui.UIAnchorLayout()

        # --- Title ---
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

        # --- Text Input ---
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

        # --- Start Button ---
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
            self.window.show_view(GameView(self.username, "Skibidi"))

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
        # This command has to happen before we start drawing
        self.clear()
        
        # Draw all the GUI elements
        self.manager.draw()