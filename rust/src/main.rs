use std::net::{TcpListener, TcpStream};
use std::thread;
use std::sync::{Arc, Mutex};
use std::io::{Read, Write};
use std::time::Duration;

const PORT: u16 = 8124;
const MAX_CLIENTS: u32 = 2;

struct PropulsionServer {
    client_fd: TcpStream,
    pending_cmd: Arc<Mutex<Option<u64>>>,
}

impl PropulsionServer {
    fn new(client_fd: TcpStream) -> Self {
        PropulsionServer {
            client_fd,
            pending_cmd: Arc::new(Mutex::new(None)),
        }
    }

    fn check_fire(&mut self) {
        loop {
            let mut pending_cmd = self.pending_cmd.lock().unwrap();
            if let Some(cmd) = *pending_cmd {
                let now = std::time::SystemTime::now()
                    .duration_since(std::time::SystemTime::UNIX_EPOCH)
                    .unwrap()
                    .as_secs();
                if cmd <= now {
                    // Send firing message through socket
                    let msg = "firing now!";
                    self.client_fd.write_all(msg.as_bytes()).unwrap();
                    *pending_cmd = None;
                }
            }
            thread::sleep(Duration::from_millis(100));
        }
    }

    fn listen_for_commands(&mut self) {
        loop {
            let mut buffer = [0; 1024];
            let bytes_received = self.client_fd.read(&mut buffer).unwrap();
            if bytes_received == 0 {
                break;
            }
            let msg = std::str::from_utf8(&buffer[..bytes_received]).unwrap();
            let relative_time: u64 = msg.parse().unwrap();
            *self.pending_cmd.lock().unwrap() = Some(relative_time + std::time::SystemTime::now()
                .duration_since(std::time::SystemTime::UNIX_EPOCH)
                .unwrap()
                .as_secs());
        }
    }
}

fn main() {
    let listener = TcpListener::bind(format!("127.0.0.1:{}", PORT)).unwrap();
    println!("Server listening on port {}", PORT);

    for stream in listener.incoming() {
        match stream {
            Ok(mut stream) => {
                println!("Client connected");
                let propulsion_server = PropulsionServer::new(stream.try_clone().unwrap());
                let pending_cmd = propulsion_server.pending_cmd.clone();

                thread::spawn(move || {
                    propulsion_server.check_fire();
                });

                thread::spawn(move || {
                    propulsion_server.listen_for_commands();
                });
            }
            Err(e) => {
                println!("Error: {}", e);
            }
        }
    }
}