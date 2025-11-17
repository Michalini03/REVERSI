import arcade
import threading
import queue  # Import the queue module
from lobby_view import LobbyView  # Import your new LobbyView
from server_handler import connect_to_server, start_receive_thread

WINDOW_WIDTH = 1280
WINDOW_HEIGHT = 720
WINDOW_TITLE = "REVERSI"


def main():
    """ Main function """

    print("[Main Thread] Connecting to server...")
    client_socket = connect_to_server()
    if client_socket is None:
        print("[Main Thread] FAILED to connect. Exiting.")
        return

    print("[Main Thread] Successfully connected.")

    server_queue = queue.Queue()

    receive_thread = threading.Thread(
        target=start_receive_thread,
        args=(client_socket, server_queue),
        daemon=True  # daemon=True means thread stops when main app stops
    )
    receive_thread.start()

    # Start game window
    window = arcade.Window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE)
    lobby_view = LobbyView(client_socket, server_queue)
    window.show_view(lobby_view)  # Show the LobbyView first
    arcade.run()

    # This code runs AFTER the window is closed
    print("[Main Thread] Game window closed. Cleaning up.")
    client_socket.close()


if __name__ == "__main__":
    main()
