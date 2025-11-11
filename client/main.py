import arcade
import threading
from game_view import GameView
from lobby_view import LobbyView
from server_handler import server_communication_loop

WINDOW_WIDTH = 1280
WINDOW_HEIGHT = 720
WINDOW_TITLE = "REVERSI"


def main():
    """ Main function """

    # Create a window class. This is what actually shows up on screen
    window = arcade.Window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, False, False)
    
    # Start the server communication thread
    server_thread = threading.Thread(target=server_communication_loop, daemon=True)
    server_thread.start()

    # Show GameView on screen
    view: LobbyView = LobbyView()
    window.show_view(view)

    # Start the arcade game loop
    arcade.run()


if __name__ == "__main__":
    main()
