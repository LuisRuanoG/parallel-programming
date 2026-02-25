#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <unordered_set>
#include <cstdio>
#include <cstdlib>
#include <curl/curl.h>
#include <stdexcept>
#include "rapidjson/error/error.h"
#include "rapidjson/reader.h"

struct ParseException : std::runtime_error, rapidjson::ParseResult {
    ParseException(rapidjson::ParseErrorCode code, const char* msg, size_t offset)
        : std::runtime_error(msg),
          rapidjson::ParseResult(code, offset) {}
};

#define RAPIDJSON_PARSE_ERROR_NORETURN(code, offset) \
    throw ParseException(code, #code, offset)

#include <rapidjson/document.h>
#include <chrono>

bool debug = false;

const std::string SERVICE_URL =
    "http://hollywood-graph-crawler.bridgesuncc.org/neighbors/";

// URL encode
std::string url_encode(CURL* curl, const std::string& input) {
    char* out = curl_easy_escape(curl, input.c_str(),
                                 static_cast<int>(input.size()));
    std::string s(out);
    curl_free(out);
    return s;
}

// Write callback
size_t WriteCallback(void* contents, size_t size,
                     size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// Fetch neighbors
std::string fetch_neighbors(CURL* curl, const std::string& node) {

    std::string url = SERVICE_URL + url_encode(curl, node);
    std::string response;

    if (debug)
        std::cout << "Sending request to: " << url << std::endl;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "User-Agent: C++-Client/1.0");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::cerr << "CURL error: "
                  << curl_easy_strerror(res) << std::endl;
    } else if (debug) {
        std::cout << "CURL request successful!" << std::endl;
    }

    curl_slist_free_all(headers);

    if (debug)
        std::cout << "Response received: "
                  << response << std::endl;

    return (res == CURLE_OK) ? response : "{}";
}

// Parse JSON
std::vector<std::string>
get_neighbors(const std::string& json_str) {

    std::vector<std::string> neighbors;

    try {
        rapidjson::Document doc;
        doc.Parse(json_str.c_str());

        if (doc.HasMember("neighbors") &&
            doc["neighbors"].IsArray()) {

            for (const auto& neighbor :
                 doc["neighbors"].GetArray()) {

                neighbors.emplace_back(
                    neighbor.GetString());
            }
        }

    } catch (const ParseException& e) {
        std::cerr << "Error parsing JSON: "
                  << json_str << std::endl;
        throw;
    }

    return neighbors;
}

// BFS
std::vector<std::vector<std::string>>
bfs(CURL* curl,
    const std::string& start,
    size_t depth) {

    std::vector<std::vector<std::string>> levels;
    std::unordered_set<std::string> visited;

    levels.push_back({start});
    visited.insert(start);

    for (size_t d = 0; d < depth; ++d) {

        if (debug)
            std::cout << "Starting level: "
                      << d << "\n";

        levels.emplace_back();

        for (const std::string& s : levels[d]) {

            try {

                if (debug)
                    std::cout << "Expanding: "
                              << s << "\n";

                for (const auto& neighbor :
                     get_neighbors(
                         fetch_neighbors(curl, s))) {

                    if (!visited.count(neighbor)) {
                        visited.insert(neighbor);
                        levels[d + 1]
                            .push_back(neighbor);
                    }
                }

            } catch (const ParseException&) {
                std::cerr
                    << "Error fetching neighbors of: "
                    << s << std::endl;
                throw;
            }
        }
    }

    return levels;
}

int main(int argc, char* argv[]) {

    if (argc != 3) {
        std::cerr << "Usage: "
                  << argv[0]
                  << " <node_name> <depth>\n";
        return 1;
    }

    std::string start_node = argv[1];

    size_t depth;
    try {
        depth = static_cast<size_t>(
            std::stoul(argv[2]));
    } catch (const std::exception&) {
        std::cerr
            << "Error: Depth must be a "
            << "non-negative integer.\n";
        return 1;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr
            << "Failed to initialize CURL\n";
        return -1;
    }

    const auto start_time =
        std::chrono::steady_clock::now();

    for (const auto& level :
         bfs(curl, start_node, depth)) {

        for (const auto& node : level)
            std::cout << "- "
                      << node << "\n";

        std::cout << level.size()
                  << "\n";
    }

    const auto finish_time =
        std::chrono::steady_clock::now();

    const std::chrono::duration<double>
        elapsed_seconds =
            finish_time - start_time;

    std::cout << "Time to crawl: "
              << elapsed_seconds.count()
              << "s\n";

    curl_easy_cleanup(curl);

    return 0;
}