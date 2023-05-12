# Lua binding

## loop

```lua
loop ev.mkloop()
```

Create a new loop and return.

```lua
loop:co(func [, args])
```

Create a coroutine.

```lua
loop:run([mode])
```

Run loop with mode.

## Timer

```
loop:sleep(ms)
```

Sleep current coroutine for ms milliseconds.

## TCP

```lua
socket loop:tcp()
```

Cteate a tcp socket.

```lua
socket:accept()
```

Accept a client socket.

```lua
socket:connect(addr)
```

Connect to peer address.

use `ev.ip_addr(ip, port)` to generate addr.

```lua
socket:listen(ip, port[, backlog])
```

Listen to ip and port. If backlog not specific, default to 1024.

```lua
socket:send(data)
```

Send data to peer.

```lua
string socket:recv()
```

Receve data from peer.

```lua
addr = socket:sockname()
```

Get local network address.

Use `ev.ip_name()` to get ip and port.

```lua
addr = socket:peername()
```

Get peer network address.

Use `ev.ip_name()` to get ip and port.
