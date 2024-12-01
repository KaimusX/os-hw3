#include <stddef.h>
typedef size_t __myfs_off_t;

typedef struct {
  __myfs_off_t first_space;
} list_node_t;

typedef struct {
  size_t remaining;
  __myfs_off_t next_space;
} allocated_node_t;

typedef struct {
  uint32_t magic;
  size_t size;
  __myfs_off_t root_dir;
  __myfs_off_t free_memory;
} myfs_handle_t;

typedef struct {
  size_t size;
  size_t allocated;
  __myfs_off_t data;
  __myfs_off_t next_file_block;
} myfs_file_block_t;

typedef struct {
  size_t total_size;
  __myfs_off_t first_file_block;  
} file_t;

typedef struct {
  size_t max_children;
  __myfs_off_t children;
} directory_t;

typedef struct {
  char name[(size_t)256];
  char is_file;
  struct timespec times[2];
  union {
    file_t file;
    directory_t directory;
  } type;
} node_t;


void *off_to_ptr(void *fsptr, __myfs_off_t offset);
__myfs_off_t ptr_to_off(void *fsptr, void *ptr);
void *get_free_memory_ptr(void *fsptr);
void __free_impl(void *fsptr, void *ptr);
void add_allocation_space(void *fsptr, list_node_t *LL, allocated_node_t *alloc);
