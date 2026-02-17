#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <rapidjson/document.h>

using namespace rapidjson;

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

    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();

    if (!curl) {
        std::cerr << "Failed to initialize curl\n";
        return 1;
    }

    // URL-encode the actor name
    char* encoded = curl_easy_escape(curl, actor.c_str(), actor.length());

    std::string url =
        "http://hollywood-graph-crawler.bridgesuncc.org/neighbors/" + std::string(encoded);

    curl_free(encoded);

    std::string response_data;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::cerr << "curl failed: " << curl_easy_strerror(res) << "\n";
        return 1;
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();




    Document doc;
    doc.Parse(response_data.c_str());

    if (!doc.HasMember("neighbors") || !doc["neighbors"].IsArray()) {
        std::cerr << "Invalid JSON or no neighbors found.\n";
        return 1;
    }

    const Value& neighbors = doc["neighbors"];

    std::vector<std::string> neighbor_list;

    for (SizeType i = 0; i < neighbors.Size(); i++) {
        neighbor_list.push_back(neighbors[i].GetString());
    }

    std::cout << "Neighbors of " << actor << ":\n";

    for (const auto& n : neighbor_list) {
        std::cout << " - " << n << "\n";
    }

    return 0;
}
