cat << 'EOF' > README.md
# Parallel Graph Traversal with Web API

## Author
Luis Ruano  
ITSC - Parallel Programming  

---

## Design Approach

### Level-by-Level BFS

Instead of using a queue, the implementation uses:

std::vector<std::vector<std::string>> levels;

Each index represents one depth level.

All nodes in a level are fully expanded before moving to the next level.

This structure is ideal for parallelization.

---

## Parallel Strategy

The parallel version follows the specification:

- A fixed maximum number of threads (MAX_THREADS = 8)
- If nodes < max threads → one thread per node
- If nodes > max threads → nodes are evenly distributed among threads
- Each thread processes a chunk of nodes

### Thread Responsibilities

Each thread:

1. Fetches neighbors via HTTP request
2. Parses JSON response safely
3. Inserts new nodes into:
   - visited set
   - next_level vector

---

## Synchronization

Two mutexes are used:

std::mutex visited_mutex;
std::mutex next_level_mutex;


Without mutex:
- Duplicate insertions into visited
- Race conditions when pushing to next_level
- Memory corruption
- Undefined behavior

Mutex ensures safe concurrent modification of shared structures.

---

## Compilation

Inside static_graphcrawler directory:

make clean
make

This builds:
- level_client
- par_level_client

---

## Running the Program

Example:

./level_client "Tom Hanks" 2
./par_level_client "Tom Hanks" 2

Arguments:
- Node name (quoted if contains spaces)
- Depth (non-negative integer)

---

## Performance Results (Centaurus)

Tested with:
Node = "Tom Hanks"
Depth = 2

Sequential:
Time to crawl: 3.29s

Parallel:
Time to crawl: 0.56s

Speedup ≈ 5.87x

The parallel implementation significantly reduces execution time by overlapping network I/O operations across multiple threads.

---

## Observations

- Parallel version reduces runtime substantially
- Speedup is not linear due to:
  - Network latency
  - Server response time
  - Mutex synchronization overhead
- Order of nodes may differ in parallel execution
- No duplicate nodes appear
- JSON validation prevents crashes due to malformed responses

---

## Files Included

- level_client.cpp (Sequential BFS)
- par_level_client.cpp (Parallel BFS)
- Makefile
- README.md

---

## Conclusion

The parallel level-by-level BFS implementation demonstrates:

- Correct multithreaded design
- Safe synchronization using mutex
- Dynamic workload distribution
- Measurable performance improvement

The solution follows the project specification and maintains correctness while improving performance.

EOF