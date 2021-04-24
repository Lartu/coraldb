# CoralDB library for LDPL

## Usage

Include the library with `include "coraldb.ldpl"`. This library depends on (and
includes) the [LDPL Socket Library](https://github.com/xvxx/ldpl-socket).

## Commands

```
CORALDB SET ADDRESS <text> PORT <number>
```

Sets the CoralDB address and port for future connections.

```
CORALDB AUTH PASSWORD <text>
```

Sets the CoralDB authentication password for future connections.

```
CORALDB SET KEY <text> VALUE <text> RESULT <text-map>
```

Sets a Key-Value pair with the key and value provided. The result of the command
is written into the provided text-map. `<text-map>:"result"` will contain `"ok"` if the
command ran successfully, `"error"` if it failed and `"wrong pass"` if the
database authentication failed. 

```
CORALDB GET KEY <text> RESULT <text-map>
```

Sets a Key-Value pair with the key and value provided. The result of the command
is written into the provided text-map. `<text-map>:"result"` will contain `"ok"` if the
command ran successfully, `"error"` if it failed and `"wrong pass"` if the
database authentication failed. If the command ran successfully, `<text-map>:"value"`
will contain the requested value.

```
CORALDB PROBE KEY <text> RESULT <text-map>
```

Probes if the key provided already exists in the database. The result of the command
is written into the provided text-map. `<text-map>:"result"` will contain `"ok"` if the
command ran successfully, `"error"` if it failed and `"wrong pass"` if the
database authentication failed. If the command ran successfully, `<text-map>:"value"`
will contain `"found"` if the key was found or `"not found"` otherwise.

```
CORALDB DROP KEY <text> RESULT <text-map>
```

Deletes a key-value pair with the provided key from the database. The result of the command
is written into the provided text-map. `<text-map>:"result"` will contain `"ok"` if the
command ran successfully, `"error"` if it failed and `"wrong pass"` if the
database authentication failed. The command doesn't fail if the key does not
exist.

```
CORALDB PING RESULT <text-map>
```

Pings the database. The result of the command
is written into the provided text-map. `<text-map>:"result"` will contain `"ok"` if the
command ran successfully, `"error"` if it failed and `"wrong pass"` if the
database authentication failed.

```
CORALDB CHECKPOINT RESULT <text-map>
```

Forces a database checkpoint. The result of the command
is written into the provided text-map. `<text-map>:"result"` will contain `"ok"` if the
command ran successfully, `"error"` if it failed and `"wrong pass"` if the
database authentication failed.

```
CORALDB SET PASSWORD <text> RESULT <text-map>
```

Sets the database authentication password (database-side). All future requests will
have to be authenticated with this password, otherise they will fail with an authentication error.
The result of the command
is written into the provided text-map. `<text-map>:"result"` will contain `"ok"` if the
command ran successfully, `"error"` if it failed and `"wrong pass"` if the
database authentication failed.

```
CORALDB DELETE PASSWORD RESULT <text-map>
```

Removes the database authentication password (database-side). The result of the command
is written into the provided text-map. `<text-map>:"result"` will contain `"ok"` if the
command ran successfully, `"error"` if it failed and `"wrong pass"` if the
database authentication failed.