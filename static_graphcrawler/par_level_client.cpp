#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <curl/curl.h>
#include <chrono>
#include <algorithm>

#include "rapidjson/document.h"

#define MAX_THREADS 8


std::mutex visited_mutex;
std::mutex next_level_mutex;

const std::string SERVICE_URL =
    "http://hollywood-graph-crawler.bridgesuncc.org/neighbors/";


std::string url_encode(CURL* curl, const std::string& input) {
    char* out = curl_easy_escape(
        curl,
        input.c_str(),
        static_cast<int>(input.size())
    );
    std::string s(out);
    curl_free(out);
    return s;
}

//callback for curl
size_t WriteCallback(void* contents,
                     size_t size,
                     size_t nmemb,
                     std::string* output) {

    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

//fetch neighbors
std::string fetch_neighbors(CURL* curl,
                            const std::string& node) {

    std::string url =
        SERVICE_URL + url_encode(curl, node);

    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                     WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,
                     &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK)
        return "";

    return response;
}

//json parsing
std::vector<std::string>
get_neighbors(const std::string& json_str) {

    std::vector<std::string> neighbors;

    if (json_str.empty())
        return neighbors;

    rapidjson::Document doc;

    if (doc.Parse(json_str.c_str()).HasParseError())
        return neighbors;

    if (!doc.IsObject())
        return neighbors;

    if (!doc.HasMember("neighbors"))
        return neighbors;

    if (!doc["neighbors"].IsArray())
        return neighbors;

    for (const auto& neighbor :
         doc["neighbors"].GetArray()) {

        if (neighbor.IsString())
            neighbors.emplace_back(
                neighbor.GetString());
    }

    return neighbors;
}


void process_chunk(
    CURL* curl,
    const std::vector<std::string>& current_level,
    size_t start,
    size_t end,
    std::unordered_set<std::string>& visited,
    std::vector<std::string>& next_level
) {

    for (size_t i = start; i < end; ++i) {

        const std::string& node =
            current_level[i];

        std::string response =
            fetch_neighbors(curl, node);

        auto neighbors =
            get_neighbors(response);

        for (const auto& neighbor :
             neighbors) {

            bool inserted = false;

            {
                std::lock_guard<std::mutex>
                    lock(visited_mutex);

                if (!visited.count(neighbor)) {
                    visited.insert(neighbor);
                    inserted = true;
                }
            }

            if (inserted) {
                std::lock_guard<std::mutex>
                    lock2(next_level_mutex);

                next_level.push_back(neighbor);
            }
        }
    }
}

//bfs with parallel processing
std::vector<std::vector<std::string>>
parallel_bfs(
    CURL* curl,
    const std::string& start,
    size_t depth
) {

    std::vector<std::vector<std::string>> levels;
    std::unordered_set<std::string> visited;

    levels.push_back({start});
    visited.insert(start);

    for (size_t d = 0; d < depth; ++d) {

        const auto& current_level =
            levels[d];

        std::vector<std::string> next_level;

        size_t nodes =
            current_level.size();

        if (nodes == 0) {
            levels.push_back({});
            continue;
        }

        size_t num_threads =
            std::min(
                static_cast<size_t>(MAX_THREADS),
                nodes
            );

        size_t chunk_size =
            nodes / num_threads;

        size_t remainder =
            nodes % num_threads;

        std::vector<std::thread> threads;

        size_t start_index = 0;

        for (size_t t = 0;
             t < num_threads;
             ++t) {

            size_t end_index =
                start_index +
                chunk_size +
                (t < remainder ? 1 : 0);

            threads.emplace_back(
                process_chunk,
                curl,
                std::cref(current_level),
                start_index,
                end_index,
                std::ref(visited),
                std::ref(next_level)
            );

            start_index = end_index;
        }

        for (auto& th : threads)
            th.join();

        levels.push_back(next_level);
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

    size_t depth =
        static_cast<size_t>(
            std::stoul(argv[2])
        );

    CURL* curl =
        curl_easy_init();

    if (!curl) {
        std::cerr
            << "Failed to initialize CURL\n";
        return -1;
    }

    const auto start_time =
        std::chrono::steady_clock::now();

    auto levels =
        parallel_bfs(
            curl,
            start_node,
            depth
        );

    for (const auto& level :
         levels) {

        for (const auto& node :
             level)
            std::cout << "- "
                      << node
                      << "\n";

        std::cout << level.size()
                  << "\n";
    }

    const auto finish_time =
        std::chrono::steady_clock::now();

    std::chrono::duration<double>
        elapsed =
            finish_time -
            start_time;

    std::cout
        << "Time to crawl: "
        << elapsed.count()
        << "s\n";

    curl_easy_cleanup(curl);

    return 0;
}