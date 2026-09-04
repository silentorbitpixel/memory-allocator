/* memory.c
   CSE 374, Memory Homework, Spring 2026
   Elizabeth Astor
   Implements getmem and freemem memory management functions,
   along with helper functions to manage a sorted free block list.
*/

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include "mem.h"
#include "mem_internal.h"

#define NODESIZE sizeof(freeNode)
#define MINCHUNK 16
#define BIGCHUNK 16000

static freeNode* freeBlockList;
static uintptr_t totalMalloc;

void* getmem(uintptr_t size) {
  assert(size > 0);
  if (size % MINCHUNK != 0) {
    size = size + MINCHUNK - (size % MINCHUNK);
  }
  uintptr_t block = get_block(size);
  if (block == 0) {
    return NULL;
  }
  return ((void*)(block + NODESIZE));
}

void freemem(void* p) {
  if (p == NULL) return;
  check_heap();
  uintptr_t node = (uintptr_t)p - NODESIZE;
  return_block(node);
  check_heap();
}

uintptr_t get_block(uintptr_t size) {
  check_heap();
  freeNode* current = freeBlockList;
  freeNode* prev = NULL;
  while (current != NULL) {
    if (current->size >= size) {
      if (current->size >= size + NODESIZE + MINCHUNK) {
        split_node(current, size);
      }
      if (prev == NULL) {
        freeBlockList = current->next;
      } else {
        prev->next = current->next;
      }
      current->next = NULL;
      check_heap();
      return (uintptr_t)current;
    }
    prev = current;
    current = current->next;
  }
  freeNode* fresh = new_block(size);
  if (fresh == NULL) return 0;
  current = freeBlockList;
  prev = NULL;
  while (current != NULL) {
    if (current->size >= size) {
      if (current->size >= size + NODESIZE + MINCHUNK) {
        split_node(current, size);
      }
      if (prev == NULL) {
        freeBlockList = current->next;
      } else {
        prev->next = current->next;
      }
      current->next = NULL;
      check_heap();
      return (uintptr_t)current;
    }
    prev = current;
    current = current->next;
  }
  return 0;
}

freeNode* new_block(int size) {
  uintptr_t allocSize = BIGCHUNK;
  if ((uintptr_t)size + NODESIZE > allocSize) {
    allocSize = (uintptr_t)size + NODESIZE;
    if (allocSize % MINCHUNK != 0) {
      allocSize = allocSize + MINCHUNK - (allocSize % MINCHUNK);
    }
  }
  void* raw = malloc(allocSize);
  if (raw == NULL) return NULL;
  totalMalloc += allocSize;
  freeNode* node = (freeNode*)raw;
  node->size = allocSize - NODESIZE;
  node->next = NULL;
  return_block((uintptr_t)node);
  return node;
}

void split_node(freeNode* n, uintptr_t size) {
  uintptr_t secondAddr = (uintptr_t)n + NODESIZE + size;
  freeNode* second = (freeNode*)secondAddr;
  second->size = n->size - size - NODESIZE;
  second->next = n->next;
  n->size = size;
  n->next = second;
}

void return_block(uintptr_t node) {
  freeNode* newNode = (freeNode*)node;
  if (freeBlockList == NULL || node < (uintptr_t)freeBlockList) {
    newNode->next = freeBlockList;
    freeBlockList = newNode;
  } else {
    freeNode* current = freeBlockList;
    while (current->next != NULL &&
           (uintptr_t)(current->next) < node) {
      current = current->next;
    }
    newNode->next = current->next;
    current->next = newNode;
  }
  if (newNode->next != NULL && adjacent(newNode)) {
    newNode->size = newNode->size + NODESIZE + newNode->next->size;
    newNode->next = newNode->next->next;
  }
  if (freeBlockList != newNode) {
    freeNode* prev = freeBlockList;
    while (prev->next != NULL && prev->next != newNode) {
      prev = prev->next;
    }
    if (prev->next == newNode && adjacent(prev)) {
      prev->size = prev->size + NODESIZE + newNode->size;
      prev->next = newNode->next;
    }
  }
}

int adjacent(freeNode* node) {
  if (node->next == NULL) return 0;
  uintptr_t endOfNode = (uintptr_t)node + NODESIZE + node->size;
  return (endOfNode == (uintptr_t)(node->next));
}

void check_heap() {
  if (!freeBlockList) return;
  freeNode* currentNode = freeBlockList;
  uintptr_t minsize = currentNode->size;
  while (currentNode != NULL) {
    if (currentNode->size < minsize) {
      minsize = currentNode->size;
    }
    if (currentNode->next != NULL) {
      assert((uintptr_t)currentNode < (uintptr_t)(currentNode->next));
      assert((uintptr_t)currentNode + currentNode->size + NODESIZE
              < (uintptr_t)(currentNode->next));
    }
    currentNode = currentNode->next;
  }
  if (minsize == 0) print_heap(stdout);
  assert(minsize >= MINCHUNK);
}

void get_mem_stats(uintptr_t* total_size, uintptr_t* total_free,
                   uintptr_t* n_free_blocks) {
  *total_size = totalMalloc;
  *total_free = 0;
  *n_free_blocks = 0;
  freeNode* currentNode = freeBlockList;
  while (currentNode) {
    *n_free_blocks = *n_free_blocks + 1;
    *total_free = *total_free + (currentNode->size + NODESIZE);
    currentNode = currentNode->next;
  }
}

void print_heap(FILE *f) {
  printf("Printing the heap\n");
  freeNode* currentNode = freeBlockList;
  while (currentNode != NULL) {
    fprintf(f, "%" PRIuPTR, (uintptr_t)currentNode);
    fprintf(f, ", size: %" PRIuPTR, currentNode->size);
    fprintf(f, ", next: %" PRIuPTR, (uintptr_t)currentNode->next);
    fprintf(f, "\n");
    currentNode = currentNode->next;
  }
}
