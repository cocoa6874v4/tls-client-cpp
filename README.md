# tls-client C++ Mapper
A small C++ mapper/wrapper for using 'tls-client' from C++.  
  
This project provides a simple `tls_client::Client` class that maps C++ method calls to the JSON request format used by the `tls-client` DLL.

## Features

- Simple C++ interface for `tls-client`
- Supports common HTTP methods:
  - GET
  - POST
  - PUT
  - DELETE
- Uses `nlohmann::json` for request options
- Loads `tls-client-windows-64.dll` dynamically
- Automatically frees response memory returned by the DLL

## Requirements

- Windows
- C++17 or later
- `tls-client-windows-64.dll`  download: https://github.com/bogdanfinn/tls-client/releases/
- `nlohmann/json`

## Setup

Place the following files in your project:

```txt
main.cpp
tls_client.hpp
tls-client-windows-64.dll
