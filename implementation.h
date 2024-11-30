#include <stddef.h>
typedef size_t __myfs_off_t;

typedef struct {
  __myfs_off_t first_space;
} list_node_t;

typedef struct {
  size_t remaining;
  __myfs_off_t next_space;
} allocated_node_t;

void *off_to_ptr(void *fsptr, __myfs_off_t offset);
__myfs_off_t ptr_to_off(void *fsptr, void *ptr);
void *get_free_memory_ptr(void *fsptr);
void __free_impl(void *fsptr, void *ptr);
void add_allocation_space(void *fsptr, list_node_t *LL, allocated_node_t *alloc);
