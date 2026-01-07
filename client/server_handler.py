import socket
import time

SERVER_ADRESS = "127.0.0.1"
PORT = 10001
HEARTBEAT_INTERVAL = 2.0  # Send heartbeat every 2 seconds
TIMEOUT_LIMIT = 6.0       # Disconnect if no response for 6 seconds


class GameSocket:
    """
    A wrapper around the standard socket to allow adding custom attributes
    like 'last_response' for the heartbeat logic.
    """
    def __init__(self, real_socket):
        self.sock = real_socket
        self.last_response = time.time()

    def recv(self, bufsize):
        return self.sock.recv(bufsize)

    def sendall(self, data):
        return self.sock.sendall(data)

    def close(self):
        return self.sock.close()


def connect_to_server(server_address=SERVER_ADRESS, port=PORT):
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((server_address, port))

        # Wrap the socket in our custom class so we can store the timestamp
        return GameSocket(sock)

    except socket.error as e:
        print(f"Connection error: {e}")
        return None


def start_receive_thread(client_socket, server_queue):
    """
    Handles receiving data. Uses a buffer to fix split packets.
    Updates the 'last_response' timestamp on every valid message.
    """
    buffer = ""
    bad_message_count = 0
    MAX_BAD_MESSAGES = 5

    try:
        while True:
            data = client_socket.recv(1024)
            if not data:
                print("[Server Thread] Server closed the connection.")
                break

            client_socket.last_response = time.time()

            buffer += data.decode('utf-8')

            while "\n" in buffer:
                message, buffer = buffer.split("\n", 1)
                message = message.strip()

                if message:
                    if not message.startswith("REV"):
                        bad_message_count += 1
                        if bad_message_count >= MAX_BAD_MESSAGES:
                            raise ConnectionAbortedError("Too many invalid messages.")
                    else:
                        bad_message_count = 0

                        if "HEARTPOP" in message:
                            continue

                        server_queue.put(message)

    except (ConnectionResetError, BrokenPipeError, ConnectionAbortedError):
        print("[Server Thread] Connection lost.")
    except Exception as e:
        print(f"[Server Thread] Error: {e}")
    finally:
        if client_socket:
            client_socket.close()
        server_queue.put("REV SERVER_DISCONNECT")


def start_heartbeat_thread(client_socket, server_queue):
    """
    Sends heartbeats AND checks for timeouts.
    """
    print(f"[Heartbeat] Started. Ping: {HEARTBEAT_INTERVAL}s, Timeout: {TIMEOUT_LIMIT}s")

    if not hasattr(client_socket, 'last_response'):
        client_socket.last_response = time.time()

    try:
        while True:
            time.sleep(HEARTBEAT_INTERVAL)
            message = "REV HEARTBEAT\n"
            client_socket.sendall(message.encode('utf-8'))

            time_since_last_response = time.time() - client_socket.last_response
            if time_since_last_response > TIMEOUT_LIMIT:
                print(f"[Heartbeat] TIMEOUT! No response for {time_since_last_response:.1f}s.")
                # Closing the socket will trigger the exception in start_receive_thread
                client_socket.close()
                return

    except (ConnectionResetError, BrokenPipeError, OSError):
        return
    except Exception as e:
        print(f"[Heartbeat] Error: {e}")
