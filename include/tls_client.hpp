#pragma once

#include <nlohmann/json.hpp>

#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace tls_client {

class Client {
  private:
    using RequestFunc = char* (*)(char*);
    using FreeMemoryFunc = void (*)(char*);

#ifdef _WIN32
    using LibraryHandle = HMODULE;
#else
    using LibraryHandle = void*;
#endif

    LibraryHandle _library = nullptr;
    RequestFunc _request = nullptr;
    FreeMemoryFunc _freeMemory = nullptr;

    static LibraryHandle loadLibrary(const std::string& path) {
#ifdef _WIN32
        auto handle = LoadLibraryA(path.c_str());

        if (!handle) {
            throw std::runtime_error("LoadLibrary failed: " + std::to_string(GetLastError()));
        }

        return handle;
#else
        auto handle = dlopen(path.c_str(), RTLD_NOW);

        if (!handle) {
            const char* error = dlerror();

            throw std::runtime_error(std::string("dlopen failed: ") +
                                     (error ? error : "unknown error"));
        }

        return handle;
#endif
    }

    template <typename T> static T loadSymbol(LibraryHandle library, const char* name) {
#ifdef _WIN32
        auto symbol = GetProcAddress(library, name);

        if (!symbol) {
            throw std::runtime_error(std::string("GetProcAddress failed for ") + name);
        }

        return reinterpret_cast<T>(symbol);
#else
        dlerror();

        auto symbol = dlsym(library, name);

        const char* error = dlerror();

        if (error) {
            throw std::runtime_error(std::string("dlsym failed for ") + name + ": " + error);
        }

        return reinterpret_cast<T>(symbol);
#endif
    }

    static void closeLibrary(LibraryHandle library) {
        if (!library) {
            return;
        }

#ifdef _WIN32
        FreeLibrary(library);
#else
        dlclose(library);
#endif
    }

    static std::string defaultLibraryPath() {
#ifdef _WIN32
        return "tls-client-windows-64.dll";
#elif defined(__linux__)
        return "./tls-client.so";
#else
#error Unsupported operating system
#endif
    }

  public:
    explicit Client(const std::string& libraryPath = defaultLibraryPath()) {
        try {
            _library = loadLibrary(libraryPath);

            _request = loadSymbol<RequestFunc>(_library, "request");

            _freeMemory = loadSymbol<FreeMemoryFunc>(_library, "freeMemory");
        } catch (...) {
            closeLibrary(_library);
            _library = nullptr;
            throw;
        }
    }

    ~Client() {
        closeLibrary(_library);
    }

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    std::string client_identifier;
    std::string request_url;

    std::string send(const std::string& method, const std::string& payload) {
        if (!_request || !_freeMemory) {
            throw std::runtime_error("tls-client library is not loaded");
        }

        nlohmann::json session;

        session["client_identifier"] = client_identifier;
        session["requestUrl"] = request_url;
        session["requestMethod"] = method;

        nlohmann::json extra =
            payload.empty() ? nlohmann::json::object() : nlohmann::json::parse(payload);

        session.update(extra);

        std::string body = session.dump();

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
