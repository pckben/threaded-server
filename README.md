Multi-threaded TCP server, which listens on a given TCP port, then forks into a separate thread to handle each connected client.

Each server thread is intended to be CPU intensive, thus a thread pool is needed to limit the number of active threads, and thus the number of connected clients.
The actual application for this library is a multi-threaded speech recognition server.




1. Server creates N threads and sleep
2. Server listen on the given port p
3. Client connect to port
