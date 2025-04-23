## Approaches and Instructions

### C++
The `c++` folder contains a straightforward TCP server implemented using the Posix socket API. The server listens for incoming connections, and when a connection is established, it creates two threads: one for checking if a fire command has been scheduled and another for listening for new commands. I have included my own `testClient.py` to show what I used to verify functionality. 


No third-party libraries are required. I utilized only the standard library, though the compiler needs to be able to compile to C++17 standard, so for gcc, you need a minimum version of gcc7

### C++/ZMQ
For fun, I also implemented the same program utilizing the Zeromq open-source library in `zmq_c++`. Similar to the Posix implementation, except made a lot nicer by hiding the boilerplate socket setup behind an API. ZMQ also enforces certain communication paradigms depending on the type of socket defined. One potential drawback to this is that both ends of communication need to utilize ZMQ, so testing with a simple python script not set up for that won't work. This is why I have also included the stdlib version above. I have provided my own python script as well if interested.

It is worth noting that ZMQ is open-source, and has supported bindings for many languages, including Rust and Python

The [ZeroMQ](https://zeromq.org/download/) library is required. I am running WSL/Debian, so I installed the libzmq3-dev headers, with the libzmq5 library for linkage. If using my `testClient.py` for this program, zmq needs to be installed in the python venv. Otherwise it's just standard library.

### Build Instructions:

The simplest way is to have `make` installed. I have provided the Makefile, so while in a terminal in the directory of the Makefile you can run:
- `make propulsion_server`

or

- `make propulsion_server_zmq`