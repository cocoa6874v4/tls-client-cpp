#pragma once
#include <Windows.h>
#include <cstddef>
#include <errhandlingapi.h>
#include <iostream>
#include <libloaderapi.h>
#include <minwindef.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace tls_client {
class Client {
  private:
    using RequestFunc = char* (*)(char*);
    using FreeMemoryFunc = void (*)(char*);
    HMODULE hModule() {
        HMODULE a = LoadLibraryA("tls-client-windows-64.dll");

        if (a == NULL) {
            std::cout << "LoadLibrary failed: " << GetLastError() << "\n";
            return NULL;
        }

        return a;
    };

    RequestFunc request(HMODULE a) {
        RequestFunc request = (RequestFunc)GetProcAddress(a, "request");

        if (!request) {
            std::cout << "GetProcAddress failed: " << GetLastError() << "\n";
        }
        return request;
    };

    FreeMemoryFunc freeMemory(HMODULE a) {
        FreeMemoryFunc freeMemory = (FreeMemoryFunc)GetProcAddress(a, "freeMemory");

        if (!freeMemory) {
            std::cout << "freeMemory GetProcAddress failed: " << GetLastError() << "\n";
        }
        return freeMemory;
    };

    HMODULE _hModule;
    RequestFunc _request;
    FreeMemoryFunc _freeMemory;

  public:
    Client() {
        _hModule = hModule();

        if (_hModule == NULL) {
            _request = nullptr;
            _freeMemory = nullptr;
            return;
        }

        _request = request(_hModule);
        _freeMemory = freeMemory(_hModule);

        if (!_request || !_freeMemory) {
            FreeLibrary(_hModule);
            _hModule = NULL;
        }
    }
    ~Client() {
        if (_hModule != NULL) {
            FreeLibrary(_hModule);
        }
    }

    std::string client_identifier;
    std::string request_url;

    std::string send(const std::string& method, const std::string& payload) {
        if (!_request || !_freeMemory) {
            throw std::runtime_error("tls-client library is not loaded");
        }

        nlohmann::json Session;

        Session["client_identifier"] = client_identifier;
        Session["requestUrl"] = request_url;
        Session["requestMethod"] = method;

        nlohmann::json extra =
            payload.empty() ? nlohmann::json::object() : nlohmann::json::parse(payload);

        Session.update(extra);

        std::string body = Session.dump();
        char* response = _request(const_cast<char*>(body.c_str()));

        if (!response) {
            throw std::runtime_error("request returned null");
        }

        std::string result(response);
        _freeMemory(response);

        return result;
    }

    std::string get(const std::string& payload = "{}") {
        return send("GET", payload);
    }

    std::string post(const std::string& payload = "{}") {
        return send("POST", payload);
    }

    std::string put(const std::string& payload = "{}") {
        return send("PUT", payload);
    }

    std::string del(const std::string& payload = "{}") {
        return send("DELETE", payload);
    }
};
} // namespace tls_client
