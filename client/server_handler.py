import socket

SERVER_ADRESS = "127.0.0.1"
PORT = 9999


def connect_to_server():
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((SERVER_ADRESS, PORT))
        return sock
    except socket.error as e:
        print(f"Connection error: {e}")
        return None

  
def server_communication_loop():
    """
    This function will run in a separate thread.
    It handles connecting to the server and the
    infinite loop for sending/receiving messages.
    """
    print("[Server Thread] Started. Trying to connect...")
    
    # Try to connect to the server
    try:
        # We assume connect_to_server() returns a connected socket
        client_socket = connect_to_server()
        if client_socket is None:
            print("[Server Thread] Failed to connect. connect_to_server() returned None.")
            return
            
    except ConnectionRefusedError:
        print("[Server Thread] Connection refused. Is the server running?")
        return
    except Exception as e:
        print(f"[Server Thread] Error connecting: {e}")
        return

    print("[Server Thread] Successfully connected to server!")
    print(">>> You can now type messages in this terminal and press Enter. <<<")

    try:
        # This is the infinite loop you wanted
        while True:
            # Wait for the user to type a message in the console
            message_to_send = input("> ")

            if not message_to_send:
                continue

            # Send the message to the server
            client_socket.sendall(message_to_send.encode('utf-8'))

            # Wait for a response from the server
            data = client_socket.recv(1024)
            if not data:
                # Server disconnected
                print("[Server Thread] Server closed the connection.")
                break
            
            # Print the server's response
            print(f"Server response: {data.decode('utf-8')}")

    except (ConnectionResetError, BrokenPipeError):
        print("[Server Thread] Connection to server was lost.")
    except Exception as e:
        print(f"[Server Thread] An error occurred: {e}")
    finally:
        # Clean up and close the socket when the loop ends
        client_socket.close()
        print("[Server Thread] Disconnected.")