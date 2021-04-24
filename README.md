<p align="center">
  <img src="https://github.com/Lartu/coraldb/blob/main/coraldblogo.png">
  <br><br>
  <img src="https://img.shields.io/badge/dev._version-v1.1.1-blue.svg">
  <img src="https://img.shields.io/badge/license-BSD_2.0-purple">
  <img src="https://img.shields.io/badge/corals-lots-yellow">
  <img src="https://img.shields.io/badge/prod._ready-probably-green">
</p>

# CoralDB
An extremely simple in-memory key-value database for distributed applications and microservices, **CoralDB** has been designed with simplicity in mind. It works as a remote dictionary that can perform search, insertion, deletion and retrieval of string-string key-value pairs. Being an in-memory system, operations are handled extremely fast. For long term storage, cyclical *checkpoint* saves are executed.

# Building and installation
* To **build** CoralDB, clone this repository and run `make` in it. C++11 is required.
* To **install** CoralDB, run `make install`.
* To **uninstall** CoralDB, simply run `make uninstall`. You will also manually have to delete any checkpoint files you may have created while using CoralDB.

# Usage
CoralDB can be used both as a local or a remote database. In both cases, running `coraldb` starts the CoralDB server 
on the default CoralDB port (26725) using the file `coraldb.db` on your home directory as its checkpoint file. Check `coraldb --help` for
information on how to launch CoralDB with a custom configuration.

Programs can connect to CoralDB to access the database system using its port and IP address (`localhost` for local systems). To communicate with
CoralDB, 6 commands can be used, sent via a normal TCP socket stream. Sent messages **must** be terminated using the `\r\n` character sequence.
Also, messages may not contain the `\r\n` character sequence, even within quotes.
Messages sent by CoralDB always end with said character sequence, too.

A client must establish a new connection to CoralDB every time it wants to store a different value.
Likewise, CoralDB closes the connection after a response has been sent to the client.

## Command Index
Command | Description
:---: | :---:
[SET](#SET) | Inserts or updates a value into the database.
[GET](#GET) | Fetches a value from the database.
[PROBE](#PROBE) | Checks if a key exists the database.
[DROP](#DROP) | Removes a key-value pair from the database.
[PING](#PING) | Checks if the database is online.
[CHECKPOINT](#CHECKPOINT) | Forces a database checkpoint save.
[SETKEY](#SETKEY) | Sets a database password.
[KEY](#KEY) | Authenticates with the database password.

### SET

**Usage**: ```SET key "value"```

**SET** stores `"value"` in `key`. Keys may not have whitespace in them. Values must be enclosed in double-quotes (`"`). Double-quotes inside a value may be escaped using the `\"` character sequence.
* Responses:
  * `OK.` for successful operations.
  * `ERROR.` for unsuccessful operations.

### GET

**Usage**: ```GET key```

**GET** gets the value stored in the key `key`. If no value had been assigned to said key, the operation fails.
* Responses:
  * `"value"` for successful operations, where `value` is the value retrieved. Note that the value is enclosed in double-quotes.
  * `ERROR.` for unsuccessful operations.


### PROBE

**Usage**: ```PROBE key```

**PROBE** succeeds if the `key` exists within the database, otherwise it fails.
* Responses:
  * `FOUND.` for successful operations.
  * `NOT-FOUND.` for unsuccessful operations.

### DROP

**Usage**: ```DROP key```

**DROP** deletes the key `key` from the database. The operation succeeds even if the key didn't previously exist.
* Responses:
  * `OK.` for successful operations.
  * `ERROR.` for unsuccessful operations.

### PING

**Usage**: ```PING```

**PING** can be used to check if CoralDB is alive.
* Responses:
  * `OK.` for successful operations.

### CHECKPOINT

**Usage**: ```CHECKPOINT```

**CHECKPOINT** can be used to trigger a checkpoint save.
* Responses:
  * `OK.` once the checkpoint save has finished.

### SETKEY

**Usage**: ```SETKEY password```

**SETKEY** can be used to protect the database behind a password. Once a password is set, unauthenticated commands will
be rejected with the `WRONG-KEY.` error message.
* Responses:
  * `OK.` once the password has been set.

### KEY

**Usage**: ```KEY password <command>```

**KEY** can be used to authenticate your request with a password. It's mandatory for databases that have previously
set a password. To use the KEY command, prepend it to another command. For example, if you want to run `GET mydata`
on a database protected behind the `123456` password, run `KEY 123456 GET mydata`.

# Example Session

In this example, `\r\n` at the end of lines are omitted.

```
Client:  SET color1 "blue"
CoralDB: OK.
Client:  SET color2 "red"
CoralDB: OK.
Client:  SET color1 "other red, better than blue"
CoralDB: OK.
Client:  GET color2
CoralDB: "red"
Client:  DROP color2
CoralDB: OK.
Client:  GET color2
CoralDB: ERROR.
```

# I need help! I want to contribute!

If you need help on using CoralDB, please open an issue here and I will try to address it as soon as possible.

Contributions are also more than welcome! Feel free to submit pull requests!

# License

CoralDB is released under the [BSD 2-Clause License](https://github.com/Lartu/coraldb/blob/main/LICENSE).
