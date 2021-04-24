from coraldb import CoralDBConnector, CoralDBResult

# Set the CoralDB connection information
connector = CoralDBConnector("localhost", 26725)

# Set a key
connector.set("mykey", "myvalue")
connector.set("hw", "Hello World!!!")
connector.set("kthxbye", "Hasta la vista!")

# Retrieve key
result = connector.get("hw")
if result[0] == CoralDBResult.OK:
    print("Got value:", result[1])

# Set database password
connector.setkey("123456")

# This command will fail because we are not authenticated
result = connector.get("kthxbye")
if result[0] == CoralDBResult.OK:
    print("Got value:", result[1])
elif result[0] == CoralDBResult.WRONGKEY:
    print("Oh no, we are not authenticated!")
elif result[0] == CoralDBResult.ERROR:
    print("Something went wrong.")

# Set authentication password for future commands
connector.key("123456")

# Try again, now authenticated
result = connector.get("kthxbye")
if result[0] == CoralDBResult.OK:
    print("Got value:", result[1])
elif result[0] == CoralDBResult.WRONGKEY:
    print("Oh no, we are not authenticated!")
elif result[0] == CoralDBResult.ERROR:
    print("Something went wrong.")

# Remove database password
connector.delkey()

# Don't use authentication password
connector.key("")

# Now we can run commands unauthenticated again
result = connector.ping()
if result == CoralDBResult.OK:
    print("Pong!")