import arcade
import threading
import queue  # Import the queue module
from lobby_view import LobbyView  # Import your new LobbyView


WINDOW_WIDTH = 1280
WINDOW_HEIGHT = 720
WINDOW_TITLE = "REVERSI"


def main():
    """ Main function """

    print("[Main Thread] Connecting to server...")
    
    # Start game window
    window = arcade.Window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE)
    lobby_view = LobbyView()
    window.show_view(lobby_view)  # Show the LobbyView first
    arcade.run()

    # This code runs AFTER the window is closed
    print("[Main Thread] Game window closed. Cleaning up.")


if __name__ == "__main__":
    main()
