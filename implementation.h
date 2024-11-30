#include <stddef.h>
typedef size_t __myfs_off_t;

typedef struct {
  __myfs_off_t first_space;
} list_node_t;

typedef struct {
  size_t remaining;
  __myfs_off_t next_space;
} allocated_node_t;



/* START memory allocation helper functions declarations */

void add_allocation_space(void *fsptr, list_node_t *LL, allocated_node_t *alloc);
void *__malloc_impl(void *fsptr, void *pref_ptr, size_t *size);
void *__realloc_impl(void *fsptr, void *orig_ptr, size_t *size);
void __free_impl(void *fsptr, void *ptr);

// END memory allocation functions

/* START fuse helper functions */

void *off_to_ptr(void *reference, __myfs_off_t offset);
__myfs_off_t ptr_to_off(void *reference, void *ptr);
void update_time(node_t *node, int new_node);
void *get_free_memory_ptr(void *fsptr);
void initilaize_check(void *fsptr, size_t fssize);
char *get_last_token(const char *path, unsigned long *token_len);
char **tokenize(const char token, const char *path, int skip_n_tokens);
void free_tokens(char **tokens);
node_t *get_node(void *fsptr, directory_t *dict, const char *child);
node_t *path_solver(void *fsptr, const char *path, int skip_n_tokens);
node_t *make_inode(void *fsptr, const char *path, int *errnoptr, int isfile);
void free_file_info(void *fsptr, file_t *file);
void remove_node(void *fsptr, directory_t *dict, node_t *node);
void remove_data(void *fsptr, myfs_file_block_t *block, size_t size);
int add_data(void *fsptr, file_t *file, size_t size, int *errnoptr);
