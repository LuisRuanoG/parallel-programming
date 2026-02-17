#include <iostream>
#include <string>
#include <curl/curl.h>

std::string response_data;

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    response_data.append((char*)contents, total_size);
    return total_size;
}

int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cerr << "Usage: ./crawler \"Actor Name\" depth\n";
        return 1;
    }

    std::string actor = argv[1];

    for (char& c : actor) {
        if (c == ' ') c = '_';
    }

    std::string url = "http://hollywood-graph-crawler.bridgesuncc.org/neighbors/" + actor;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();

    if (!curl) {
        std::cerr << "Failed to initialize curl\n";
        return 1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::cerr << "curl failed: " << curl_easy_strerror(res) << "\n";
    } else {
        std::cout << response_data << "\n";
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return 0;
}
