Parallel Graph Crawler using Blocking Queue

Compilation:
make

Execution:
./client "Tom Hanks" <depth>

Example:
./client "Tom Hanks" 4

Description:
This program performs a parallel Breadth-First Search (BFS) traversal on a movie graph.
It uses a blocking queue and multiple worker threads to explore the graph in parallel.

Each thread fetches neighbors of a node using an HTTP API and inserts new nodes into the queue
until the specified depth is reached.

Threads used in the parallel version: 8
