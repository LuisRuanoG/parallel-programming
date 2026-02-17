#include <iostream>
#include <string>
#include <curl/curl.h>

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    std::string* response = static_cast<std::string*>(userp);
    response->append((char*)contents, total_size);
    return total_size;
}

int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cerr << "Usage: ./crawler \"Actor Name\" depth\n";
        return 1;
    }

    std::string actor = argv[1];

    for (char& c : actor) {
        if (c == ' ')
            c = '_';
    }

    std::string url =
        "http://hollywood-graph-crawler.bridgesuncc.org/neighbors/" + actor;

    std::cout << "Requesting: " << url << std::endl;

    CURL* curl;
    CURLcode res;
    std::string response_data;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (!curl) {
        std::cerr << "Failed to initialize curl\n";
        return 1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::cerr << "curl failed: " << curl_easy_strerror(res) << "\n";
    } else {
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        std::cout << "HTTP code: " << http_code << "\n\n";
        std::cout << response_data << std::endl;
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return 0;
}
