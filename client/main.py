import arcade
import threading
from client.game_view import GameView
from client.lobby_view import LobbyView
from server_handler import server_communication_loop

WINDOW_WIDTH = 1280
WINDOW_HEIGHT = 720
WINDOW_TITLE = "REVERSI"


def main():
    """ Main function """

    # Create a window class. This is what actually shows up on screen
    window = arcade.Window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE)

    # Create and setup the GameView
    game = LobbyView()
    
    # Start the server communication thread
    server_thread = threading.Thread(target=server_communication_loop, daemon=True)
    server_thread.start()

    # Show GameView on screen
    window.show_view(game)

    # Start the arcade game loop
    arcade.run()


if __name__ == "__main__":
    main()
