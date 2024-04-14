# Project 2 - IOTA: Server for Remote Calculator

## Task

Your task is to implement a server for the IPK Calculator Protocol [1].
The server should be able to communicate with any client using IPKCP.

The server is started using:
`ipkcpd -h <host> -p <port> -m <mode>`
where `host` is the IPv4 address the server will listen on, `port` the port it will listen on, and `mode` either `tcp` or `udp` (e.g., `ipkcpd -h 127.0.0.1 -p 2023 -m udp`).

The server will use the appropriate protocol variant depending on the selected mode (binary for UDP, textual for TCP).
It should be able to handle more than one client at the same time.
The mechanism used to handle concurrent connections is up to the implementation, but it should be clearly described as part of the documentation.

The server should react to the _Interrupt_ signal (`C-c`) by gracefully exiting, cleanly terminating all active connections as appropriate for the selected mode.

## Submission

The submission should follow all the requirements described in the general README.
The project should build a single executable called `ipkcpd` in the project root using the default `make` target.
This executable need not be self-contained, and can be implemented as a wrapper/launcher, but it should conform to the CLI interface described above.

## References

[1] [IPK Calculator Protocol](https://git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master/Project%201/Protocol.md)
