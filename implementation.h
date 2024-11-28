#include <stddef.h>
typedef size_t __myfs_off_t;

void *off_to_ptr(void *fsptr, __myfs_off_t offset);
__myfs_off_t ptr_to_off(void *fsptr, void *ptr);
void *get_free_memory_ptr(void *fsptr);

