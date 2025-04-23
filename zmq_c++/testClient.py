import subprocess
import threading
import time
import zmq

HOST = "127.0.0.1"  # Server address (assuming it's running locally)
PORT = 8124         # Must match the C++ server port

def receive_messages(client_socket):
    shutdown = False
    while not shutdown:
        try:
            response = client_socket.recv_string()
            # print(len(response), "bytes received")
            if not response:
                print("Server disconnected")
                break  # If server disconnects
            print("Received from server:", response)
        except KeyboardInterrupt:
            shutdown = True
        except Exception as e:
            print(f"Error receiving message: {e}")
            break

def send_messages(client_socket):
    shutdown = False
    while not shutdown:
        try:
            time.sleep(1)
            user_input = input("Enter an relative time to send: ").strip()
            if not user_input:
                continue
            if user_input.strip() == "exit":
                client_socket.send_string("-1")
                break
            client_socket.send_string(user_input)
        except KeyboardInterrupt:
            shutdown = True
        except Exception as e:
            print(f"Error sending message: {e}")
            break

def main():
    try:
        # Run the C++ server
        server_process = subprocess.Popen(["./propulsion_server_zmq"])

        # ZMQ socket setup
        time.sleep(1)
        context = zmq.Context()
        client_socket = context.socket(zmq.DEALER)
        client_socket.connect(f"tcp://{HOST}:{PORT}")
        print(f"Connected to server at {HOST}:{PORT}")

        listener_thread = threading.Thread(target=receive_messages, args=(client_socket,), daemon=True)
        listener_thread.start()

        sender_thread = threading.Thread(target=send_messages, args=(client_socket,))
        sender_thread.start()

    except KeyboardInterrupt:
        print("Shutting down...")
    except Exception as e:
        print(f"Failed to connect: {e}")
    
    finally:
        sender_thread.join()
        server_process.send_signal(subprocess.signal.SIGINT)
        client_socket.close()
        server_process.wait()

if __name__ == "__main__":
    main()
