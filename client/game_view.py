import arcade
import arcade.gui
import time
import queue
import threading
from server_handler import connect_to_server, start_receive_thread, start_heartbeat_thread
from board import Board
from stone import Stone

GAME_WIDTH = 900
WINDOW_WIDTH = 1280
WINDOW_HEIGHT = 720
BOARD_SIZE = 8
# --- Constant for the UI sidebar ---
UI_WIDTH = WINDOW_WIDTH - GAME_WIDTH  # This is 200px
RIGHT_MARGIN = UI_WIDTH // 7.5

PREFIX = "REV"


class GameView(arcade.View):
    """
    Main application class.
    """

    def __init__(self, client_socket, server_queue, current_player: int, username_one: str = "Player 1", username_two: str = "Player 2", lobby_id: int = -1, init_state: str = "", server_ip: str = "", server_port: int = 0, my_username: str = ""):
        """
        Initializer
        """
        # Call the parent class initializer
        super().__init__()

        # --- GUI Manager ---
        self.manager = arcade.gui.UIManager()

        # Variables that will hold sprite lists
        self.active_player = 1
        self.current_player = current_player

        self.lobby_id = lobby_id
        self.username_one = username_one
        self.username_two = username_two

        self.client_socket = client_socket
        self.server_queue = server_queue
        
        # Networking info
        self.server_ip = server_ip
        self.server_port = server_port
        self.my_username = my_username
        self.is_reconnecting = False

        # Labels
        self.player1_score_label = None
        self.player2_score_label = None
        self.status_label = None

        # Modals
        self.pause_box = None
        self.end_box = None
        self.rematch_box = None
        self.reconnect_box = None

        # Set up the player info
        self.board = Board()
        self.init_state = init_state

        # Set the background color
        self.background_color = arcade.color.AMAZON

    def setup_game(self):
        """ Set up the game and initialize the variables. """
        self.board = Board()
        self.prepare_ui()

    def on_show_view(self):
        """ This is run when we switch to this view """
        # Enable the UIManager
        self.manager.enable()
        self.setup_game()
        # Set the background color
        arcade.set_background_color(self.background_color)

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
        
        # --- DRAW ACTIVE PLAYER GREEN DOT ---
        # X Position: Slightly to the left of the sidebar text
        dot_x = WINDOW_WIDTH - UI_WIDTH + 30 
        
        target_y = 0
        
        if self.active_player == 1:
            # Player 1 is at the top. 
            # WINDOW_HEIGHT - Margin - Half Height of Box (~50px offset)
            target_y = WINDOW_HEIGHT - RIGHT_MARGIN - 50 
            
        elif self.active_player == 2:
            # Player 2 is below Player 1.
            # Adjusted offset for the second box
            target_y = WINDOW_HEIGHT - RIGHT_MARGIN - 160

        # Draw the dot if we have a valid target
        if target_y > 0:
            arcade.draw_circle_filled(
                center_x=dot_x, 
                center_y=target_y, 
                radius=8, 
                color=arcade.color.BRIGHT_GREEN
            )
            
            # Optional: Add a white outline
            arcade.draw_circle_outline(
                center_x=dot_x, 
                center_y=target_y, 
                radius=8, 
                color=arcade.color.WHITE, 
                border_width=2
            )
        
    def show_server_error_popup(self):
        """
        Shows an Error modal popup using UIMessageBox 
        """        
        self.reconnect_box = arcade.gui.UIMessageBox(
            width=350,
            height=200,
            message_text="Disconnected! \nReconnecting... (Please Wait)",
            buttons=["Leave Game"]
        )
        
        @self.reconnect_box.event("on_action")
        def on_message_box_close(event):
            self.is_reconnecting = False
            self.server_queue = None
            from lobby_view import LobbyView
            self.window.show_view(LobbyView())
            pass
        
        self.manager.add(self.reconnect_box)
        
        thread = threading.Thread(target=self._reconnect_loop, daemon=True)
        thread.start()
        
    def _reconnect_loop(self):
        """ Attempt to reconnect in the background """
        attempts: int = 0
        
        while self.is_reconnecting:
            if self.client_socket is None:
                attempts += 1
                print("[GameView] Attempting to reconnect to server...")
                new_socket = connect_to_server(self.server_ip, self.server_port)
                if new_socket is not None:
                    try:
                        msg = f"REV CREATE {self.my_username}\n"
                        new_socket.sendall(msg.encode('utf-8'))
                        
                        self.client_socket = new_socket
                        self.server_queue = queue.Queue()
                        
                        recv_thread = threading.Thread(
                            target=start_receive_thread, 
                            args=(self.client_socket, self.server_queue), 
                            daemon=True
                        )
                        recv_thread.start()
                        
                        hb_thread = threading.Thread(
                            target=start_heartbeat_thread,
                            args=(self.client_socket, self.server_queue),
                            daemon=True
                        )
                        hb_thread.start()

                        self.is_reconnecting = False                  
                        if self.reconnect_box:
                            self.manager.remove(self.reconnect_box)
                            self.reconnect_box = None
                            
                        print("[Reconnect] Success! Resuming game.")
                        return
                    except Exception as e:
                        print(f"[Reconnect] Login failed: {e}")
                        if new_socket: new_socket.close()

            for _ in range(5):
                if not self.is_reconnecting: return
                time.sleep(1)
    
    def show_pause_modal(self, player_name: str):
        """
        Shows a Pause modal when opponent disconnects
        """
        if self.pause_box: return  # Already showing

        self.pause_box = arcade.gui.UIMessageBox(
            width=350,
            height=200,
            message_text=f"Player {player_name} was disconnected from the game\nWaiting for reconnection.",
            buttons=["Leave"]
        )
        
        @self.pause_box.event("on_action")
        def on_message_box_close(event):
            from lobby_list_view import LobbyListView
            # Refresh lobby list logic
            self.window.show_view(LobbyListView(5, self.client_socket, self.server_queue))
            
            # Send Exit command
            try:
                message = f"{PREFIX} EXIT {self.lobby_id}\n"
                print(f"[GameView] Leaving the game!")
                self.client_socket.sendall(message.encode())
            except Exception as e:
                pass
            pass

        self.manager.add(self.pause_box)

    def show_error_popup(self, error_message):
        """ Helper for simple errors """
        message_box = arcade.gui.UIMessageBox(
            width=350,
            height=200,
            message_text=error_message,
            buttons=["OK"]
        )
        @message_box.event("on_action")
        def on_message_box_close(event):
            try:
                if self.client_socket:
                    print(f"[GameView] Error popup closed. Exiting Lobby {self.lobby_id}.")
                    self.client_socket.sendall(f"{PREFIX} EXIT {self.lobby_id}\n".encode())
            except Exception as e:
                print(f"[GameView] Failed to send exit command: {e}")

            from lobby_list_view import LobbyListView
            self.window.show_view(LobbyListView(5, self.client_socket, self.server_queue))
        
        self.manager.add(message_box)

    def show_game_over_popup(self, winner_id):
        """Shows who won and lets user leave, quit, or rematch"""
        
        if winner_id == 3:
            result_text = "It's a Draw!"
        else:
            winner_text = "Black (Player 1)" if winner_id == 1 else "White (Player 2)"
            result_text = f"Winner: {winner_text}"
            
        self.end_box = arcade.gui.UIMessageBox(
            width=500,
            height=300,
            message_text=f"GAME OVER\n\n{result_text}",
            buttons=["Rematch", "Menu", "Quit"]
        )
        
        @self.end_box.event("on_action")
        def on_close(event):
            action = event.action
            
            if action == "Quit":
                arcade.exit()
                
            elif action == "Menu":
                try:
                    self.client_socket.sendall(f"{PREFIX} EXIT {self.lobby_id}\n".encode())
                except:
                    pass
                from lobby_list_view import LobbyListView
                self.window.show_view(LobbyListView(5, self.client_socket, self.server_queue))
            
            elif action == "Rematch":
                print("[GameView] Rematch requested.")
                try:
                    self.client_socket.sendall(f"{PREFIX} REMATCH {self.lobby_id}\n".encode())
                except:
                    pass
                if self.end_box:
                    self.manager.remove(self.end_box)
                    self.end_box = None
                
                # 2. Show the waiting box
                self.show_waiting_for_rematch_popup()
                
        self.manager.add(self.end_box)
        
    def show_waiting_for_rematch_popup(self):
        """Shows a modal indicating we are waiting for the opponent"""
        
        self.rematch_box = arcade.gui.UIMessageBox(
            width=350,
            height=200,
            message_text="Waiting for opponent\nto accept rematch...",
            buttons=["Cancel"] # No buttons, they must wait or the opponent triggers the start
        )
        def rematch_box(event):
            try:
                if self.client_socket:
                    self.client_socket.sendall(f"{PREFIX} EXIT {self.lobby_id}\n".encode())
            except Exception as e:
                print(f"[GameView] Failed to send exit command: {e}")

            from lobby_list_view import LobbyListView
            self.window.show_view(LobbyListView(5, self.client_socket, self.server_queue))
        
        self.manager.add(self.rematch_box)


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

                if command == "START":
                    # HANDLE REMATCH START
                    # Close any open Game Over / Pause boxes
                    if getattr(self, "rematch_box", None):
                        self.manager.remove(self.rematch_box)
                        self.rematch_box = None
                    if getattr(self, "end_box", None):
                        self.manager.remove(self.end_box)
                        self.end_box = None
                    if getattr(self, "pause_box", None):
                        self.manager.remove(self.pause_box)
                        self.pause_box = None
                
                    # Soft Reset the Client State
                    self.board = Board()
                    self.active_player = 1
                    
                    if self.status_label:
                        self.status_label.text = "Rematch Started!"
                        self.status_label.style = {"text_color": arcade.color.BRIGHT_GREEN}
                        
                    print("[GameView] Rematch started!")

                elif command == "STATE":
                    # Params: PREFIX STATE <BOARD> <SCORE1> <SCORE2>
                    self.board.set_state(params[2])
                    score1 = params[3]
                    score2 = params[4]
                    if self.player1_score_label:
                        self.player1_score_label.text = str(score1)
                    if self.player2_score_label:
                        self.player2_score_label.text = str(score2)
                    # Update Active Player for the Green Dot
                    self.active_player = int(params[5])
                    # Clear warning text if it was showing a pass
                    if self.status_label and "No moves" in self.status_label.text:
                        self.status_label.text = "Game in progress..."
                        self.status_label.style = {"text_color": arcade.color.YELLOW}
                elif command == "SERVER_DISCONNECT":
                    print("[GameView] Disconnected from server.")
                    self.show_server_error_popup()
                elif command == "DISCONNECT":
                    player_num = int(params[2])
                    player_name: str = self.username_one if player_num == 1 else self.username_two
                    # Check if we are currently looking at the Game Over / Rematch screen
                    if getattr(self, "rematch_box", None) or getattr(self, "end_box", None):
                        if self.rematch_box:
                            self.manager.remove(self.rematch_box)
                            self.rematch_box = None
                        if self.end_box:
                            self.manager.remove(self.end_box)
                            self.end_box = None
                        self.show_error_popup(f"{player_name} left.\nRematch cancelled.")
                    else:
                        print(f"[GameView] Player {player_name} disconnected from the game.")
                        self.show_pause_modal(player_name)
                elif command == "RECONNECT":
                    if self.pause_box is not None:
                        self.manager.remove(self.pause_box)
                        self.pause_box = None
                        print("[GameView] Opponent reconnected. ")
                        if self.status_label:
                            self.status_label.text = "Opponent Reconnected!"
                elif command == "PASS":
                    if self.status_label:
                        self.status_label.text = "No moves available!\nTurn passed to opponent."
                        self.status_label.style = {"text_color": arcade.color.RED}
                    # Manually switch active player locally for instant feedback
                    self.active_player = 2 if self.active_player == 1 else 1
                    print("[GameView] Turn passed.")
                elif command == "END":
                    winner_id = int(params[2])
                    self.active_player = 1
                    self.show_game_over_popup(winner_id)
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

        return player_box, score_value_label

    def prepare_ui(self):
        """ Prepare the UI elements """

        # This main layout will anchor our UI to the top right
        self.ui_anchor_layout = arcade.gui.UIAnchorLayout()

        # Create the main vertical box for the UI sidebar
        ui_box = arcade.gui.UIBoxLayout(
            vertical=True,
            width=UI_WIDTH - RIGHT_MARGIN,
            space_between=RIGHT_MARGIN
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

        # --- Status handler ---
        status_box = arcade.gui.UIBoxLayout(
            vertical=True, 
            padding=(10, 10, 10, 10),
            bg_color=(20, 20, 20, 200)
        )

        self.status_label = arcade.gui.UILabel(
            text="Game Started",
            width=UI_WIDTH - RIGHT_MARGIN - 20,
            font_size=14,
            font_name="Arial",
            text_color=arcade.color.YELLOW,
            align="center",
            multiline=True
        )
        status_box.add(self.status_label)

        ui_box.add(status_box)

        self.ui_anchor_layout.add(
            child=ui_box,
            anchor_x="right",
            anchor_y="top",
            align_x=-RIGHT_MARGIN,  # Center in the sidebar
            align_y=-RIGHT_MARGIN
        )

        self.manager.add(self.ui_anchor_layout)

    def send_move(self, board_x: int, board_y: int):
        """ Send the player's move to the server """
        message = f"{PREFIX} MOVE {board_x} {board_y} {self.lobby_id}\n"
        print(f"[GameView] Sending move to server: {message}")
        self.client_socket.sendall(message.encode())
