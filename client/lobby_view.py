import arcade
import arcade.gui
import queue
import threading
from server_handler import connect_to_server, start_receive_thread, start_heartbeat_thread
from lobby_list_view import LobbyListView

GAME_PREFIX = "REV"
DEFAULT_IP = "127.0.0.1"
DEFAULT_PORT = 10001


class QuietInputText(arcade.gui.UIInputText):
    def deactivate(self):
        if self._active:
            super().deactivate()


class LobbyView(arcade.View):
    def __init__(self):
        super().__init__()
        self.manager = arcade.gui.UIManager()

        self.username = ""
        self.client_socket = None
        self.server_queue = None

        self.background_color = arcade.color.AMAZON
        self.setup_lobby()

    def setup_lobby(self):
        self.manager.clear()

        # Increased space_between to account for taller inputs
        self.v_box = arcade.gui.UIBoxLayout(space_between=15)

        # --- Title ---
        title_label = arcade.gui.UILabel(
            text="Welcome to REVERSI",
            font_size=28,
            font_name="Arial",
            text_color=arcade.color.WHITE,
            bold=True
        )
        self.v_box.add(title_label.with_padding(bottom=20))

        # This keeps the code clean since we have 3 similar inputs
        def add_input_field(label_text, default_val):
            label = arcade.gui.UILabel(
                text=label_text, 
                text_color=arcade.color.WHITE,
                font_size=14,
                bold=True
            )
            self.v_box.add(label)

            # 2. Input (Taller height)
            input_field = QuietInputText(
                width=250, 
                height=32,
                text=default_val, 
                font_size=15,
                text_color=arcade.color.WHITE
            )
            self.v_box.add(input_field)
            return input_field

        # Inputs
        self.username_input = add_input_field("Username:", "Player1")
        self.ip_input = add_input_field("Server IP:", DEFAULT_IP)
        self.port_input = add_input_field("Server Port:", str(DEFAULT_PORT))

        # Start Button
        self.start_button = arcade.gui.UIFlatButton(
            text="Connect & Start", 
            width=250, 
            height=100  # Made button slightly taller to match inputs
        )
        self.v_box.add(self.start_button.with_padding(top=30))

        # --- Layout & Anchor ---
        self.anchor_layout = arcade.gui.UIAnchorLayout()
        self.anchor_layout.add(
            child=self.v_box,
            anchor_x="center",
            anchor_y="center"
        )

        self.manager.add(self.anchor_layout)

        # --- Event Handler ---
        @self.start_button.event("on_click")
        def on_click_start(event):
            self.connect_and_start()

    def show_error_popup(self, error_message):
        """
        Shows an Error modal popup using UIMessageBox
        """
        self.manager.focused_element = None

        message_box = arcade.gui.UIMessageBox(
            width=350,
            height=200,
            message_text=error_message,
            buttons=["OK"]
        )

        @message_box.event("on_action")
        def on_message_box_close(event):
            pass

        self.server_queue = None
        self.manager.add(message_box)

    def connect_and_start(self):
        self.username = self.username_input.text
        target_ip = self.ip_input.text

        is_valid = self.valid_values(target_ip, self.port_input.text, self.username)
        if not is_valid[0]:
            self.show_error_popup(is_valid[1])
            return

        target_port = int(self.port_input.text)

        # 3. Attempt Connection
        client_socket = connect_to_server(target_ip, target_port)

        if client_socket is None:
            print("[Main Thread] FAILED to connect.")
            self.show_error_popup(f"Connection Failed.\n\nCould not reach:\n{target_ip}:{target_port}")
            return

        print("[Main Thread] Successfully connected.")
        self.client_socket = client_socket
        self.server_queue = queue.Queue()

        self.ip_adress = target_ip
        self.port = target_port

        import time
        time.sleep(0.05)

        try:
            print(f"[Main Thread] Sending CREATE for {self.username}...")
            message = f"{GAME_PREFIX} CREATE {self.username}\n"
            self.client_socket.sendall(message.encode('utf-8'))
        except Exception as e:
            print(f"Error sending login: {e}")
            self.client_socket.close()
            return

        receive_thread = threading.Thread(
            target=start_receive_thread,
            args=(client_socket, self.server_queue),
            daemon=True
        )
        receive_thread.start()

        heartbeat_thread = threading.Thread(
            target=start_heartbeat_thread,
            args=(client_socket, self.server_queue,),
            daemon=True
        )
        heartbeat_thread.start()

    def on_show_view(self):
        self.manager.enable()
        arcade.set_background_color(self.background_color)

    def on_hide_view(self):
        self.manager.disable()

    def on_draw(self):
        self.clear()
        self.manager.draw()

    def on_update(self, delta_time):
        # Safety check: Do nothing if queue isn't created yet
        if self.server_queue is None:
            return

        while not self.server_queue.empty():
            try:
                message = self.server_queue.get_nowait()
                if message.startswith(GAME_PREFIX):
                    params = message.split()
                    if len(params) > 1 and params[1] == "LOBBY":
                        lobby_count = int(params[2])
                        self.window.show_view(LobbyListView(lobby_count, self.client_socket, self.server_queue, self.ip_adress, self.port, self.username))
                    elif params[1] == "START":
                        print("[LobbyList] Reconnecting the game...")

                        who_starts = int(params[2])
                        player_name = params[3]
                        opponent_name = params[4]
                        lobby_id = int(params[5])

                        from game_view import GameView
                        game_view = GameView(
                            self.client_socket, 
                            self.server_queue, 
                            who_starts, 
                            player_name, 
                            opponent_name, 
                            lobby_id, 
                            self.ip_adress, 
                            self.port, 
                            self.username
                        )
                        self.window.show_view(game_view)
                        return
                    elif params[1] == "SERVER_DISCONNECT":
                        self.show_error_popup("Disconnected from server.")
                    else:
                        print(f"[WARNING] Unknown command received: {message}")
            except queue.Empty:
                pass

    def valid_values(self, ip, port_str, username) -> [bool, str]:
        try:
            port = int(port_str)
            if port < 1 or port > 65535:
                return [False, "Port must be between 1 and 65535."]
        except ValueError:
            return [False, "Port must be a valid integer."]

        if ip.strip() == "":
            return [False, "IP address cannot be empty."]

        if not all(c.isdigit() or c == '.' for c in ip):
            return [False, "IP address contains invalid characters."]

        if ip.count('.') != 3:
            return [False, "IP address must be IPv4."]

        if username.strip() == "":
            return [False, "Username cannot be empty."]

        if username.count(" ") > 0:
            return [False, "Username cannot contain spaces."]

        return [True, ""]
