#include "tls_client.hpp"

#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

int main() {
    try {
        tls_client::Client client;

        client.client_identifier =
            "chrome_103"; // Client list:
                          // https://github.com/bogdanfinn/tls-client/blob/master/profiles/profiles.go
        client.request_url = "https://reqbin.com/echo";

        nlohmann::json headers = {{"requestBody", ""},
                                  {"followRedirects", true},
                                  {"insecureSkipVerify", false},
                                  {"timeoutSeconds", 30}};

        auto raw = client.get(headers.dump()); // You can use get, post, put, del

        std::cout << raw << "\n";
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
