<p align="center">
  <img src="https://github.com/Lartu/coraldb/blob/main/coraldblogo.png">
  <br><br>
  <img src="https://img.shields.io/badge/dev._version-v1.0.0-blue.svg">
  <img src="https://img.shields.io/badge/license-BSD_2.0-purple">
  <img src="https://img.shields.io/badge/corals-lots-yellow">
  <img src="https://img.shields.io/badge/prod._ready-not yet-red">
</p>

# CoralDB
An extremely simple in-memory key-value database for \*nix systems, **CoralDB** has been designed with simplicity in mind. It works as a remote dictionary that can perform search, insertion, deletion and retrieval of string-string key-value pairs. Being an in-memory system, operations are handled extremely fast. For long term storage, cyclical *checkpoint* saves are executed.

# Building and installation
* To **build** CoralDB, clone this repository and run `make` in it.
* To **install** CoralDB, run `make install`.
* To **uninstall** CoralDB, simply run `make uninstall`. You will also manually have to delete any checkpoint files you may have created while using CoralDB.
