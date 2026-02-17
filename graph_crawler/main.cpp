#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <unordered_set>
#include <curl/curl.h>
#include <rapidjson/document.h>

using namespace rapidjson;

// callback function to write response data into a string
size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    std::string* response = static_cast<std::string*>(userp);
    response->append((char*)contents, total_size);
    return total_size;
}

//get neighbors of a node (actor) from the API
std::vector<std::string> get_neighbors(const std::string& node) {

    CURL* curl = curl_easy_init();
    std::vector<std::string> neighbor_list;

    if (!curl) return neighbor_list;

    char* encoded = curl_easy_escape(curl, node.c_str(), node.length());

    std::string url =
        "http://hollywood-graph-crawler.bridgesuncc.org/neighbors/" + std::string(encoded);

    curl_free(encoded);

    std::string response_data;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        return neighbor_list;
    }

    curl_easy_cleanup(curl);

    Document doc;
    doc.Parse(response_data.c_str());

    if (!doc.HasMember("neighbors") || !doc["neighbors"].IsArray())
        return neighbor_list;

    const Value& neighbors = doc["neighbors"];

    for (SizeType i = 0; i < neighbors.Size(); i++) {
        neighbor_list.push_back(neighbors[i].GetString());
    }

    return neighbor_list;
}

//BFS
int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cerr << "Usage: ./crawler \"Actor Name\" depth\n";
        return 1;
    }

    std::string start = argv[1];
    int max_depth = std::stoi(argv[2]);

    curl_global_init(CURL_GLOBAL_DEFAULT);

    std::queue<std::pair<std::string, int>> q;
    std::unordered_set<std::string> visited;

    q.push({start, 0});
    visited.insert(start);

    std::cout << "BFS Traversal:\n";

    while (!q.empty()) {

        auto current = q.front();
        q.pop();

        std::string node = current.first;
        int depth = current.second;

        std::cout << "Depth " << depth << ": " << node << "\n";

        if (depth >= max_depth)
            continue;

        std::vector<std::string> neighbors = get_neighbors(node);

        for (const auto& neighbor : neighbors) {

            if (visited.find(neighbor) == visited.end()) {
                visited.insert(neighbor);
                q.push({neighbor, depth + 1});
            }
        }
    }

    curl_global_cleanup();

    return 0;
}
