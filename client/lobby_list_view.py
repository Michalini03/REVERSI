import arcade
import arcade.gui
from game_view import GameView  # Import the GameView
import queue                  # Import the queue

# Protocol prefix for game-related messages
GAME_PREFIX = "REV"

class LobbyListView(arcade.View):
    """
    Lobby List view class.
    Handles the UI for showing the list of lobbies.
    """

    def __init__(self, lobby_count, client_socket, server_queue):
        """ Initializer """
        super().__init__()

        # Network and state
        self.client_socket = client_socket
        self.server_queue = server_queue
        self.lobby_count = lobby_count

        self.title_label = None
        self.manager = None
        self.v_box = None
        self.lobby_buttons = []

        self.lobby_id = -1

        self.background_color = arcade.color.AMAZON

    def setup_lobby_list(self):
        """ Set up the lobby list view. """
        self.title_label = arcade.Text(
            "Select a Lobby",
            self.window.width / 2,
            self.window.height - 70,
            color=arcade.color.WHITE,
            font_size=30, 
            anchor_x="center"
        )

        self.lobby_buttons = [] 
        button_center_x = self.window.width / 2

        start_y = self.window.height - 150
        button_spacing = 60

        # Create the lobby buttons
        self.lobby_buttons = [] 
        for i in range(self.lobby_count):
            lobby_id = i
            button = arcade.gui.UIFlatButton(text=f"Join Lobby {lobby_id}", width=250)


            button.center_x = button_center_x
            button.center_y = start_y - (i * button_spacing)

            self.lobby_buttons.append(button)

            button.on_click = lambda event, lobby_id=lobby_id: self.join_lobby(lobby_id)
            self.manager.add(button)


    def on_show_view(self):
        """ This is run when we switch to this view """
        # --- FIX: Create UIManager and run setup here ---
        self.manager = arcade.gui.UIManager(self.window)
        self.setup_lobby_list()
        self.manager.enable()
        # --- End Fix ---
        
        # Set the background color
        arcade.set_background_color(self.background_color)


    def on_hide_view(self):
        """ This is run when we switch away from this view """
        self.manager.disable()


    def on_draw(self):
        """ Render the screen. """
        self.clear()

        # Draw a title
        if self.title_label:
            self.title_label.draw()

        self.manager.draw()


    def on_update(self, delta_time):
        """ Check the server queue for messages """
        if self.client_socket is None or self.server_queue is None:
            return
        
        try:
            while not self.server_queue.empty():
                message: str = self.server_queue.get()
                
                if message is None or message.strip() == "":
                    continue
                
                print(f"[LobbyListView] Message from server: {message}")
                params = message.split()
                if params[0] == GAME_PREFIX:
                    command = params[1]
                    if command == "CONNECT":
                        player_id = int(params[2])
                        if player_id == 3:
                            self.show_error_popup("Lobby is full. Cannot join.")
                            return

                        print(f"[LobbyListView] Connected to lobby as player {player_id}")
                        self.show_waiting_circle()

                    if command == "START":
                        print("[LobbyListView] Game is starting...")

                        who_starts = int(params[2])
                        player_name = params[3]
                        opponent_name = params[4]
                        lobby_id = params[5]

                        game_view = GameView(self.client_socket, self.server_queue, who_starts, player_name, opponent_name, lobby_id)
                        self.window.show_view(game_view)
                        return
                    if command == "SERVER_DISCONNECT":
                        print("[LobbyListView] Disconnected from server.")
                        self.show_server_error_popup()
                else:
                    print("[LobbyListView] Unknown message prefix")
        except Exception as e:
            print(f"[LobbyListView] Error processing queue: {e}")
            
    def show_error_popup(self, error_message):
        """
        Shows an Error modal popup using UIMessageBox 
        """
        
        message_box = arcade.gui.UIMessageBox(
            width=350,
            height=200,
            message_text=error_message,
            buttons=["OK"]
        )
        
        @message_box.event("on_action")
        def on_message_box_close(event):
            message = f"{GAME_PREFIX} EXIT {self.lobby_id}\n"
            self.lobby_id = -1
            try:
                self.client_socket.sendall(message.encode('utf-8'))
            except Exception as e:
                print(f"[LobbyListView] Error sending join lobby message: {e}")
        
        self.manager.add(message_box)
    
     
    def show_waiting_circle(self):
        message_box = arcade.gui.UIMessageBox(
            width=300,
            height=175,
            message_text="Waiting for opponent...",
            buttons=["Exit"]

        )
        
        @message_box.event("on_action")
        def on_waiting_box_action(event):
            print("[LobbyListView] Exiting waiting modal.")
            self.client_socket.sendall(f"{GAME_PREFIX} EXIT {self.lobby_id}\n".encode('utf-8'))
        self.manager.add(message_box) 
        
    def show_server_error_popup(self):
        """
        Shows an Error modal popup using UIMessageBox 
        """
        
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

    def join_lobby(self, lobby_id):
        """
        Called when a 'Join Lobby' button is clicked.
        Sends the join request to the server.
        """
        self.lobby_id = lobby_id
        print(f"[LobbyListView] Attempting to join lobby {lobby_id}...")
        message = f"{GAME_PREFIX} JOIN {lobby_id}\n"
        try:
            self.client_socket.sendall(message.encode('utf-8'))
        except Exception as e:
            print(f"[LobbyListView] Error sending join lobby message: {e}")