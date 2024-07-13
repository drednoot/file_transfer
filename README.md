# Client Server File Transfer
C++ client-server application that allows client-server file transfer

# What is this app
  
## features
 * Client is able to send data to server for saving
 * Server files are updated on users in real time
 * Client is able to download files from the server

## technology
 - Qt 5 with Network

# Building

## Requirements
- qt5-qmake
- qt5
- make
- gcc
- stdc++ libs (usually comes bundled with g++)

## build steps
```
cd src
make
```
This will build both client and server  
They will be available in `build/server/server` and `build/client/client` respectively

To build client and server separately you can run
```
make build_server
```
and
```
make build_client
```
respectively

to run server and client you can run
```
make run_server
```
to run server, and
```
make run_client
```
to run a client.  
Or you can just run an executable found in build directory
