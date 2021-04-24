import socket
from typing import Tuple, Optional
from enum import Enum

class CoralDBResult(Enum):
    OK = 0
    ERROR = 1
    WRONGKEY = 2


class CoralDBConnector:
    def __init__(self, address: str, port: int):
        '''Initializes the CoralDBConnector object.
        '''
        self.address: str = address
        self.port: int = port
        self.authkey: Optional[str] = None

    def __get_return_value(self, result: str) -> CoralDBResult:
        if result == "OK.":
            return CoralDBResult.OK
        elif result == "ERROR.":
            return CoralDBResult.ERROR
        elif result == "WRONG-KEY.":
            return CoralDBResult.WRONGKEY
        else:
            return CoralDBResult.OK

    
    def set(self, key: str, value: str) -> CoralDBResult:
        '''Sets the specified key-value pair.
        '''
        key = key.replace('"', '\\\"').replace('\n', '\\n')
        value = value.replace('"', '\\\"').replace('\n', '\\n')
        result = self.run_connection(f"SET {key} \"{value}\"").strip()
        return self.__get_return_value(result)
    
    def get(self, key: str) -> Tuple[CoralDBResult, str]:
        '''Gets the specified key. Returns a tuple with the result of the
        command in the first position and the eventual value in the second.
        '''
        key = key.replace('"', '\\\"').replace('\n', '\\n')
        result = self.run_connection(f"GET {key}").strip()
        value = ""
        if self.__get_return_value(result) == CoralDBResult.OK:
            value = result.strip()[1:-1].replace('\\\"', '"').replace('\\n', '\n')
        return [self.__get_return_value(result), value]
    
    def probe(self, key: str) -> Tuple[CoralDBResult, bool]:
        '''Probes the specified key. Returns a tuple with the result of the
        command in the first position and the eventual probe result in the second.
        '''
        key = key.replace('"', '\\\"').replace('\n', '\\n')
        result = self.run_connection(f"PROBE {key}").strip()
        return [self.__get_return_value(result), result == "FOUND."]
    
    def drop(self, key: str) -> CoralDBResult:
        '''Drops the specified key.
        '''
        key = key.replace('"', '\\\"').replace('\n', '\\n')
        result = self.run_connection(f"DROP {key}").strip()
        return self.__get_return_value(result)
    
    def ping(self) -> bool:
        '''Pings the database.
        '''
        result = self.run_connection("PING").strip()
        return self.__get_return_value(result)
    
    def checkpoint(self) -> bool:
        '''Forces a database checkpoint.
        '''
        result = self.run_connection("CHECKPOINT").strip()
        return self.__get_return_value(result)
    
    def setkey(self, key: str) -> bool:
        '''Sets a database password.
        '''
        result = self.run_connection(f"SETKEY {key}").strip()
        return self.__get_return_value(result)
    
    def key(self, key: str) -> None:
        '''Sets the password to use when interfacing with the database.
        '''
        self.authkey = key
        if self.authkey == "":
            self.authkey = None
    
    def delkey(self) -> None:
        '''Removes the database password.
        '''
        result = self.run_connection("DELKEY").strip()
        return self.__get_return_value(result)

    def run_connection(self, command: str) -> str:
        try:
            total_data = ""
            if self.authkey is not None:
                command = f"KEY {self.authkey} {command}"
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((self.address, self.port))
                s.sendall(bytes(command + "\r\n", "utf-8"))
                while True:
                    data = s.recv(1024)
                    if not data:
                        break
                    total_data += data.decode("utf-8")
                    if "\n" in total_data:
                        break
            if "\n" in total_data:
                total_data = total_data.split("\n")[0]
            return total_data
        except ConnectionRefusedError:
            raise Exception("CoralDB cannot be reached.")