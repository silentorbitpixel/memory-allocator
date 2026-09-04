Custom Memory Allocator
Elizabeth Astor

A custom implementation of dynamic memory management in C, replicating malloc and free using a sorted, coalescing free list.

Overview
Implements getmem and freemem, backed by a singly linked list of free blocks kept in ascending address order. This allows adjacent free blocks to be merged automatically, reducing fragmentation. When the free list can't satisfy a request, the allocator pulls a large chunk from the system (malloc) and splits it up for future use.

Features
getmem(size) — returns a 16-byte-aligned block of at least size bytes
freemem(ptr) — frees a block and merges it with adjacent free blocks
Block splitting and coalescing to reduce fragmentation
check_heap() — assertion-based validation of free list integrity
print_heap() / get_mem_stats() — debugging and benchmarking utilities
Files
memory.c — implementation
mem.h — public interface
mem_internal.h — internal declarations
Makefile — build and test
