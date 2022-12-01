CASH (cryptographic access shell portocol)
==========================================

Secure communication protocol with AES encryption for running source shell on the server.
This protocol has two password.

Why two password?
-----------------

- The password value will not be changed in the socket
- Exchange data is more secure
- If one of the passwords is detected, the communication will not be established properly

How will it work?
-----------------
This protocol works with two passwords as follows:

- Server password
- Client password

The server password for data encryption is on the server side, and the values sent to the client can be recovered with this password.

The password of client is also for the same work on the client side.

To communicate between the server and the client, the value of the client password on both sides and the server password on both sides must be the same.

What happens when connecting?
-----------------------------

After establishing the connection, the initial value of START_C_MSG is encrypted from the client side and sent to the server. If the sent value is equal to the START_C_MSG value, the connection will be established, otherwise the connection will be terminated. The server encrypts a random number and sends it to the client. The client will receive it and decrypt it and re-encrypt it and send it to the server. If its value corresponds to the created number, the connection will be established, otherwise the connection will be disconnected

> The START_C_MSG value can be variable, but this value must be the same between the client and the server, otherwise the connection will not be established. The default value of START_C_MSG is \x21\xFF. Don't change your preferences.

After authentication, the values sent on both sides are encrypted with the same values and the communication continues like this

> **WARNING** This protocol is for executing commands on the server side and cannot work properly like the SSH protocol!

---------------------------
***DEVELOPED BY MJSCRIPT (Mahmood Jamshidian)***
