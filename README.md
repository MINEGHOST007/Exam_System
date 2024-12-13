# MCQ-Based Exam System

## Project Structure

The project includes the following files:

- Source code files (`*.c`, `*.h`)
- `makefile` for building the project

## How to Build and Run

To build the project, run:

```bash
make
```

This will compile the project and generate the `server` and `client` executables in the `./build` directory.

### Running the Server

Start the server by executing:

```bash
./build/server
```

### Running the Client

For remote connections, before running the client, update the client's code to connect to the server's IP address. Replace `INADDR_ANY` with the server's IP address, which you can find using:

```bash
ip addr show
```

Alternatively, for local connections, you can directly run the client without modifying the code.

Start the client by executing:

```bash
./build/client
```

We have implemented semaphores to manage concurrency, aiming to avoid deadlocks and preferring reliability over response time.

## Contributing

Contributions are welcome! Please follow these steps:

1. Fork the repository.
2. Create a new branch.
3. Commit your changes.
4. Open a pull request.
