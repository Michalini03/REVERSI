import socket

SERVER_ADRESS = "127.0.0.1"
PORT = 9999


def connect_to_server(server_address=SERVER_ADRESS, port=PORT):
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((server_address, port))
        return sock
    except socket.error as e:
        print(f"Connection error: {e}")
        return None


def start_receive_thread(client_socket, server_queue):
    """
    This function will run in a separate thread.
    It handles connecting to the server and the
    infinite loop for sending/receiving messages.
    """

    try:
        # This is the infinite loop you wanted
        while True:

            # Wait for a response from the server
            data = client_socket.recv(1024)
            if not data:
                # Server disconnected
                print("[Server Thread] Server closed the connection.")
                break

            # Print the server's response
            full_message: str = data.decode('utf-8')
            messages = full_message.split("\n")
            for message in messages:
                if message is not None or message.strip() != "":
                    server_queue.put(message)

    except (ConnectionResetError, BrokenPipeError):
        print("[Server Thread] Connection to server was lost.")
    except Exception as e:
        print(f"[Server Thread] An error occurred: {e}")
    finally:
        # Clean up and close the socket when the loop ends
        if client_socket:
            client_socket.close()
        server_queue.put("REV SERVER_DISCONNECT")
        print("[Server Thread] Disconnected.")


def send_message(client_socket, message):
    """ Send a message to the server. """
    try:
        client_socket.sendall(message.encode('utf-8'))
    except Exception as e:
        print(f"[Send Message] Error sending message: {e}")