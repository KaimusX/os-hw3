/*

  MyFS: a tiny file-system written for educational purposes

  MyFS is 

  Copyright 2018-21 by

  University of Alaska Anchorage, College of Engineering.

  Copyright 2022-24

  University of Texas at El Paso, Department of Computer Science.

  Contributors: Christoph Lauter 
                ... and
                ...

  and based on 

  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall myfs.c implementation.c `pkg-config fuse --cflags --libs` -o myfs

*/

#include <stddef.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>


/* The filesystem you implement must support all the 13 operations
   stubbed out below. There need not be support for access rights,
   links, symbolic links. There needs to be support for access and
   modification times and information for statfs.

   The filesystem must run in memory, using the memory of size 
   fssize pointed to by fsptr. The memory comes from mmap and 
   is backed with a file if a backup-file is indicated. When
   the filesystem is unmounted, the memory is written back to 
   that backup-file. When the filesystem is mounted again from
   the backup-file, the same memory appears at the newly mapped
   in virtual address. The filesystem datastructures hence must not
   store any pointer directly to the memory pointed to by fsptr; it
   must rather store offsets from the beginning of the memory region.

   When a filesystem is mounted for the first time, the whole memory
   region of size fssize pointed to by fsptr reads as zero-bytes. When
   a backup-file is used and the filesystem is mounted again, certain
   parts of the memory, which have previously been written, may read
   as non-zero bytes. The size of the memory region is at least 2048
   bytes.

   CAUTION:

   * You MUST NOT use any global variables in your program for reasons
   due to the way FUSE is designed.

   You can find ways to store a structure containing all "global" data
   at the start of the memory region representing the filesystem.

   * You MUST NOT store (the value of) pointers into the memory region
   that represents the filesystem. Pointers are virtual memory
   addresses and these addresses are ephemeral. Everything will seem
   okay UNTIL you remount the filesystem again.

   You may store offsets/indices (of type size_t) into the
   filesystem. These offsets/indices are like pointers: instead of
   storing the pointer, you store how far it is away from the start of
   the memory region. You may want to define a type for your offsets
   and to write two functions that can convert from pointers to
   offsets and vice versa.

   * You may use any function out of libc for your filesystem,
   including (but not limited to) malloc, calloc, free, strdup,
   strlen, strncpy, strchr, strrchr, memset, memcpy. However, your
   filesystem MUST NOT depend on memory outside of the filesystem
   memory region. Only this part of the virtual memory address space
   gets saved into the backup-file. As a matter of course, your FUSE
   process, which implements the filesystem, MUST NOT leak memory: be
   careful in particular not to leak tiny amounts of memory that
   accumulate over time. In a working setup, a FUSE process is
   supposed to run for a long time!

   It is possible to check for memory leaks by running the FUSE
   process inside valgrind:

   valgrind --leak-check=full ./myfs --backupfile=test.myfs ~/fuse-mnt/ -f

   However, the analysis of the leak indications displayed by valgrind
   is difficult as libfuse contains some small memory leaks (which do
   not accumulate over time). We cannot (easily) fix these memory
   leaks inside libfuse.

   * Avoid putting debug messages into the code. You may use fprintf
   for debugging purposes but they should all go away in the final
   version of the code. Using gdb is more professional, though.

   * You MUST NOT fail with exit(1) in case of an error. All the
   functions you have to implement have ways to indicated failure
   cases. Use these, mapping your internal errors intelligently onto
   the POSIX error conditions.

   * And of course: your code MUST NOT SEGFAULT!

   It is reasonable to proceed in the following order:

   (1)   Design and implement a mechanism that initializes a filesystem
         whenever the memory space is fresh. That mechanism can be
         implemented in the form of a filesystem handle into which the
         filesystem raw memory pointer and sizes are translated.
         Check that the filesystem does not get reinitialized at mount
         time if you initialized it once and unmounted it but that all
         pieces of information (in the handle) get read back correctly
         from the backup-file. 

   (2)   Design and implement functions to find and allocate free memory
         regions inside the filesystem memory space. There need to be 
         functions to free these regions again, too. Any "global" variable
         goes into the handle structure the mechanism designed at step (1) 
         provides.

   (3)   Carefully design a data structure able to represent all the
         pieces of information that are needed for files and
         (sub-)directories.  You need to store the location of the
         root directory in a "global" variable that, again, goes into the 
         handle designed at step (1).
          
   (4)   Write __myfs_getattr_implem and debug it thoroughly, as best as
         you can with a filesystem that is reduced to one
         function. Writing this function will make you write helper
         functions to traverse paths, following the appropriate
         subdirectories inside the file system. Strive for modularity for
         these filesystem traversal functions.

   (5)   Design and implement __myfs_readdir_implem. You cannot test it
         besides by listing your root directory with ls -la and looking
         at the date of last access/modification of the directory (.). 
         Be sure to understand the signature of that function and use
         caution not to provoke segfaults nor to leak memory.

   (6)   Design and implement __myfs_mknod_implem. You can now touch files 
         with 

         touch foo

         and check that they start to exist (with the appropriate
         access/modification times) with ls -la.

   (7)   Design and implement __myfs_mkdir_implem. Test as above.

   (8)   Design and implement __myfs_truncate_implem. You can now 
         create files filled with zeros:

         truncate -s 1024 foo

   (9)   Design and implement __myfs_statfs_implem. Test by running
         df before and after the truncation of a file to various lengths. 
         The free "disk" space must change accordingly.

   (10)  Design, implement and test __myfs_utimens_implem. You can now 
         touch files at different dates (in the past, in the future).

   (11)  Design and implement __myfs_open_implem. The function can 
         only be tested once __myfs_read_implem and __myfs_write_implem are
         implemented.

   (12)  Design, implement and test __myfs_read_implem and
         __myfs_write_implem. You can now write to files and read the data 
         back:

         echo "Hello world" > foo
         echo "Hallo ihr da" >> foo
         cat foo

         Be sure to test the case when you unmount and remount the
         filesystem: the files must still be there, contain the same
         information and have the same access and/or modification
         times.

   (13)  Design, implement and test __myfs_unlink_implem. You can now
         remove files.

   (14)  Design, implement and test __myfs_unlink_implem. You can now
         remove directories.

   (15)  Design, implement and test __myfs_rename_implem. This function
         is extremely complicated to implement. Be sure to cover all 
         cases that are documented in man 2 rename. The case when the 
         new path exists already is really hard to implement. Be sure to 
         never leave the filessystem in a bad state! Test thoroughly 
         using mv on (filled and empty) directories and files onto 
         inexistant and already existing directories and files.

   (16)  Design, implement and test any function that your instructor
         might have left out from this list. There are 13 functions 
         __myfs_XXX_implem you have to write.

   (17)  Go over all functions again, testing them one-by-one, trying
         to exercise all special conditions (error conditions): set
         breakpoints in gdb and use a sequence of bash commands inside
         your mounted filesystem to trigger these special cases. Be
         sure to cover all funny cases that arise when the filesystem
         is full but files are supposed to get written to or truncated
         to longer length. There must not be any segfault; the user
         space program using your filesystem just has to report an
         error. Also be sure to unmount and remount your filesystem,
         in order to be sure that it contents do not change by
         unmounting and remounting. Try to mount two of your
         filesystems at different places and copy and move (rename!)
         (heavy) files (your favorite movie or song, an image of a cat
         etc.) from one mount-point to the other. None of the two FUSE
         processes must provoke errors. Find ways to test the case
         when files have holes as the process that wrote them seeked
         beyond the end of the file several times. Your filesystem must
         support these operations at least by making the holes explicit 
         zeros (use dd to test this aspect).

   (18)  Run some heavy testing: copy your favorite movie into your
         filesystem and try to watch it out of the filesystem.

*/

/* Helper types and functions */

/* YOUR HELPER FUNCTIONS GO HERE */

//// STRUCTS HERE

typedef size_t __myfs_off_t;

typedef struct {
  size_t remaining;
  __myfs_off_t next_space;
} allocated_node_t;

typedef struct {
  __myfs_off_t first_space;
} list_node_t;

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
  size_t number_children;
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


//// STRUCTS END

//// 

/* Add a new allocation to the list in ascending order, merging adjacent spaces if possible */
void add_allocation_space(void *fsptr, list_node_t *LL, allocated_node_t *alloc) {
    __myfs_off_t alloc_off = ptr_to_off(fsptr, alloc);

    // Case 1: New space comes before the first space
    if (alloc_off < LL->first_space) {
        allocated_node_t *first = off_to_ptr(fsptr, LL->first_space);
        if ((alloc_off + sizeof(size_t) + alloc->remaining) == LL->first_space) {
            // Merge with the first space
            alloc->remaining += sizeof(size_t) + first->remaining;
            alloc->next_space = first->next_space;
        } else {
            // Just insert as the new first space
            alloc->next_space = LL->first_space;
        }
        LL->first_space = alloc_off;
        return;
    }

    // Case 2: Find the position in the list to insert the new allocation
    allocated_node_t *current = off_to_ptr(fsptr, LL->first_space);
    while (current->next_space != 0 && current->next_space < alloc_off) {
        current = off_to_ptr(fsptr, current->next_space);
    }

    __myfs_off_t current_off = ptr_to_off(fsptr, current);
    __myfs_off_t next_off = current->next_space;

    // Try to merge with the next space
    if (next_off != 0 && (alloc_off + sizeof(size_t) + alloc->remaining) == next_off) {
        allocated_node_t *next = off_to_ptr(fsptr, next_off);
        alloc->remaining += sizeof(size_t) + next->remaining;
        alloc->next_space = next->next_space;
    } else {
        alloc->next_space = next_off;
    }

    // Try to merge with the current space
    if ((current_off + sizeof(size_t) + current->remaining) == alloc_off) {
        current->remaining += sizeof(size_t) + alloc->remaining;
        current->next_space = alloc->next_space;
    } else {
        current->next_space = alloc_off;
    }
}



/* Extend the preferred block with available space, updating pointers and sizes as needed */
void extend_pref_block(void *fsptr, allocated_node_t *before_pref,
                       allocated_node_t *org_pref, __myfs_off_t pref_off,
                       size_t *size) {
    allocated_node_t *pref = off_to_ptr(fsptr, pref_off);

    // Case 1: Prefer block can fully satisfy the size request
    if (pref->remaining >= *size) {
        if (pref->remaining > *size + sizeof(allocated_node_t)) {
            // Create a new free block with the remaining space
            allocated_node_t *new_block = (void *)pref + *size;
            new_block->remaining = pref->remaining - *size;
            new_block->next_space = pref->next_space;

            // Update original preferred block size
            org_pref->remaining += *size;

            // Link new block into the free list
            before_pref->next_space = pref_off + *size;
        } else {
            // Consume all remaining space in the prefer block
            org_pref->remaining += pref->remaining;

            // Update free list to skip the prefer block
            before_pref->next_space = pref->next_space;
        }

        // Entire size request fulfilled
        *size = 0;
        return;
    }

    // Case 2: Prefer block cannot fully satisfy the size request
    // Use all remaining space in the prefer block
    org_pref->remaining += pref->remaining;

    // Update free list to skip the prefer block
    before_pref->next_space = pref->next_space;

    // Adjust remaining size request
    *size -= pref->remaining;
}



/* Allocate memory from the free list, prioritizing a preferred block if available */
void *get_allocation(void *fsptr, list_node_t *LL, allocated_node_t *org_pref, size_t *size) {
    if (LL->first_space == 0) {
        // No free space available
        return NULL;
    }

    // Ensure requested size is at least the size of a block header
    if (*size < sizeof(allocated_node_t)) {
        *size = sizeof(allocated_node_t);
    }

    __myfs_off_t pref_off = 0;
    if ((void *)org_pref != fsptr) {
        pref_off = ptr_to_off(fsptr, org_pref) + sizeof(size_t) + org_pref->remaining;
    }

    allocated_node_t *current = off_to_ptr(fsptr, LL->first_space);
    allocated_node_t *largest = current;
    allocated_node_t *before_current = NULL;
    allocated_node_t *before_largest = NULL;

    size_t largest_size = current->remaining;

    // Traverse the free list to find the preferred block and the largest block
    while (current != NULL) {
        __myfs_off_t current_off = ptr_to_off(fsptr, current);

        if (current_off == pref_off) {
            // Preferred block found
            extend_pref_block(fsptr, before_current, org_pref, pref_off, size);
            if (*size == 0) {
                return NULL;  // Allocation complete from preferred block
            }
        }

        // Track the largest block
        if (current->remaining > largest_size) {
            largest = current;
            largest_size = current->remaining;
            before_largest = before_current;
        }

        // Move to the next block
        before_current = current;
        current = (current->next_space != 0) ? off_to_ptr(fsptr, current->next_space) : NULL;
    }

    // Allocate from the largest block if size is still not 0
    if (*size != 0 && largest != NULL) {
        allocated_node_t *ptr = largest;
        __myfs_off_t largest_off = ptr_to_off(fsptr, largest);

        if (largest->remaining >= *size) {
            // Can satisfy the remaining size request
            if (largest->remaining > *size + sizeof(allocated_node_t)) {
                // Create a new free block from the remainder
                allocated_node_t *new_block = (void *)largest + sizeof(size_t) + *size;
                new_block->remaining = largest->remaining - *size - sizeof(size_t);
                new_block->next_space = largest->next_space;

                if (before_largest == NULL) {
                    LL->first_space = largest_off + sizeof(size_t) + *size;
                } else {
                    before_largest->next_space = largest_off + sizeof(size_t) + *size;
                }
                ptr->remaining = *size;
            } else {
                // Consume entire block
                if (before_largest == NULL) {
                    LL->first_space = largest->next_space;
                } else {
                    before_largest->next_space = largest->next_space;
                }
            }
            *size = 0;  // Allocation complete
        } else {
            // Largest block can't fully satisfy the request
            *size -= largest->remaining;
            if (before_largest == NULL) {
                LL->first_space = largest->next_space;
            } else {
                before_largest->next_space = largest->next_space;
            }
        }

        return (void *)ptr + sizeof(size_t);
    }

    // No suitable block found
    return NULL;
}



/* Allocate memory if size is non-zero, using the preferred pointer if provided */
void *__malloc_impl(void *fsptr, void *pref_ptr, size_t *size) {
    // Return NULL if requested size is zero
    if (*size == 0) {
        return NULL;
    }

    // Default to the base allocation pointer if no preferred pointer is provided
    if (pref_ptr == NULL) {
        pref_ptr = fsptr + sizeof(size_t);
    }

    // Call get_allocation to allocate memory
    return get_allocation(fsptr, get_free_memory_ptr(fsptr), pref_ptr - sizeof(size_t), size);
}



/* Resize memory allocation, either shrinking or expanding, or freeing if size is 0 */
void *__realloc_impl(void *fsptr, void *orig_ptr, size_t *size) {
    // Free and return NULL if the requested size is 0
    if (*size == 0) {
        __free_impl(fsptr, orig_ptr);
        return NULL;
    }

    list_node_t *LL = get_free_memory_ptr(fsptr);

    // Handle case where orig_ptr points to the base (equivalent to malloc)
    if (orig_ptr == fsptr) {
        return get_allocation(fsptr, LL, fsptr, size);
    }

    // Access the current allocation metadata
    allocated_node_t *alloc = (allocated_node_t *)((char *)orig_ptr - sizeof(size_t));
    allocated_node_t *temp;
    void *new_ptr = NULL;

    if (alloc->remaining >= *size) {
        // Case 1: Shrinking within the current allocation
        if (alloc->remaining < *size + sizeof(allocated_node_t)) {
            new_ptr = orig_ptr; // No new allocation needed
        }
        // Case 2: Shrinking with enough space to split
        else {
            temp = (allocated_node_t *)((char *)orig_ptr + *size);
            temp->remaining = alloc->remaining - *size - sizeof(size_t);
            temp->next_space = 0; // Mark as end of list
            add_allocation_space(fsptr, LL, temp); // Return remaining space to free list

            alloc->remaining = *size; // Update allocation size
            new_ptr = orig_ptr;       // No new allocation needed
        }
    } else {
        // Case 3: Expanding allocation
        new_ptr = get_allocation(fsptr, LL, fsptr, size);
        if (new_ptr == NULL) {
            return NULL; // Unable to allocate more space
        }

        // Copy existing data to new allocation and free old space
        memcpy(new_ptr, orig_ptr, alloc->remaining);
        add_allocation_space(fsptr, LL, alloc);
    }

    return new_ptr;
}



/* Free allocated memory by returning it to the free list */
void __free_impl(void *fsptr, void *ptr) {
    // No operation needed if ptr is NULL
    if (ptr == NULL) {
        return;
    }

    // Return the memory to the free list
    add_allocation_space(fsptr, get_free_memory_ptr(fsptr), 
                         (char *)ptr - sizeof(size_t));
}

///////////// END OF MEMORY ALLOCATION IMPLEMENTATION////////////////


/* Convert an offset to a pointer relative to the base address */
void *off_to_ptr(void *fsptr, __myfs_off_t offset) {
    void *ptr = (char *)fsptr + offset;

    // Ensure the resulting pointer doesn't wrap around
    return (ptr < fsptr) ? NULL : ptr;
}



/* Convert a pointer to an offset relative to the base address */
__myfs_off_t ptr_to_off(void *fsptr, void *ptr) {
    // Ensure the provided pointer is within valid range
    return (fsptr > ptr) ? 0 : (__myfs_off_t)((char *)ptr - (char *)fsptr);
}



/* Update access and modification times for a node */
void update_time(node_t *node, int set_mod) {
    if (!node) {
        return;
    }

    struct timespec ts;

    // Retrieve current time and update node times if successful
    if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
        node->times[0] = ts;
        if (set_mod) {
            node->times[1] = ts;
        }
    }
}



/* Retrieve the pointer to the free memory list in the handler */
void *get_free_memory_ptr(void *fsptr) {
    return &((myfs_handle_t *)fsptr)->free_memory;
}



/* Initialize or mount the file system handler */
void initial_check(void *fsptr, size_t fssize) {
    myfs_handle_t *handle = (myfs_handle_t *)fsptr;

    // Check if the file system is being mounted for the first time
    if (handle->magic != (uint32_t)0xcafebabe) {
        // Set up handler metadata
        handle->magic = (uint32_t)0xcafebabe;
        handle->size = fssize;

        // Set up root directory offset
        handle->root_dir = sizeof(myfs_handle_t);
        node_t *root = off_to_ptr(fsptr, handle->root_dir);

        // Initialize root directory node
        memset(root->name, '\0', (size_t)255 + 1); // Clear the name field
        memcpy(root->name, "/", strlen("/"));       // Set root name to "/"
        update_time(root, 1);                       // Update timestamps
        root->is_file = 0;                          // Root is a directory

        // Initialize root directory structure
        directory_t *dir = &root->type.directory;
        dir->number_children = 1;                  // Reserve first slot for ".."

        // Allocate space for root directory's children
        size_t *children_size = off_to_ptr(fsptr, handle->root_dir + sizeof(node_t));
        *children_size = 4 * sizeof(__myfs_off_t); // Allocate space for 4 children
        dir->children = ptr_to_off(fsptr, (void *)children_size + sizeof(size_t));
        __myfs_off_t *children_ptr = off_to_ptr(fsptr, dir->children);

        // Set root's parent to 0 (no parent)
        *children_ptr = 0;

        // Initialize free memory management
        handle->free_memory = dir->children + *children_size;
        list_node_t *LL = get_free_memory_ptr(fsptr);
        allocated_node_t *free_block = off_to_ptr(fsptr, LL->first_space);

        // Set up the free block
        free_block->remaining = fssize - handle->free_memory - sizeof(size_t);
        memset((void *)free_block + sizeof(size_t), 0, free_block->remaining);
    }
}



/* Extracts the last token from a given path and returns it as a new string. */
char *get_last_token(const char *path, unsigned long *token_len) {
    if (path == NULL || token_len == NULL) {
        return NULL; // Validate inputs
    }

    unsigned long len = strlen(path);
    if (len == 0) {
        *token_len = 0;
        return NULL; // Handle empty path
    }

    unsigned long index = len;

    // Find the last '/' in the path
    while (index > 0) {
        if (path[index - 1] == '/') {
            break;
        }
        index--;
    }

    // Calculate the length of the last token
    *token_len = len - index;

    // Allocate memory for the token (including null terminator)
    char *token = (char *)malloc((*token_len + 1) * sizeof(char));
    if (token == NULL) {
        return NULL; // Handle allocation failure
    }

    // Copy the last token into the newly allocated memory
    strcpy(token, &path[index]);
    token[*token_len] = '\0'; // Ensure null termination

    return token;
}



/* Splits a path into tokens based on a delimiter, skipping the last skip_n_tokens. */
char **tokenize(const char delimiter, const char *path, int skip_n_tokens) {
    if (path == NULL || skip_n_tokens < 0) {
        return NULL; // Validate inputs
    }

    // Count the number of tokens in the path
    int n_tokens = 0;
    for (const char *c = path; *c != '\0'; c++) {
        if (*c == delimiter) {
            n_tokens++;
        }
    }

    // Ensure at least one token exists (root directory) and adjust for skipped tokens
    n_tokens -= skip_n_tokens;
    if (n_tokens <= 0) {
        return NULL; // Nothing to tokenize
    }

    // Allocate space for tokens array (+1 for null terminator)
    char **tokens = (char **)malloc((n_tokens + 1) * sizeof(char *));
    if (tokens == NULL) {
        return NULL; // Allocation failed
    }

    const char *start = path + 1; // Skip the first character (assumed root '/')
    const char *end = start;

    // Populate tokens array
    for (int i = 0; i < n_tokens; i++) {
        while (*end != delimiter && *end != '\0') {
            end++;
        }

        // Allocate space for the current token
        int token_len = end - start;
        tokens[i] = (char *)malloc((token_len + 1) * sizeof(char));
        if (tokens[i] == NULL) {
            // Free previously allocated tokens on failure
            for (int j = 0; j < i; j++) {
                free(tokens[j]);
            }
            free(tokens);
            return NULL;
        }

        // Copy the token and null-terminate it
        memcpy(tokens[i], start, token_len);
        tokens[i][token_len] = '\0';

        start = (*end == delimiter) ? end + 1 : end; // Advance to the next token
        end = start;
    }

    // Null-terminate the array of tokens
    tokens[n_tokens] = NULL;

    return tokens;
}



/* Frees the memory allocated for an array of tokens. */
void free_tokens(char **tokens) {
    if (tokens == NULL) {
        return; // Ensure tokens is not NULL before attempting to free memory
    }

    // Free each token string
    for (char **p = tokens; *p != NULL; p++) {
        free(*p);
    }

    // Finally, free the tokens array itself
    free(tokens);
}



/* Retrieves a node from the directory by the given child name.
 * Returns the node corresponding to the child or NULL if not found.
 */
node_t *get_node(void *fsptr, directory_t *dict, const char *child) {
    // Ensure the directory is valid and has children
    if (dict == NULL || child == NULL) {
        return NULL;  // Return NULL if directory or child is NULL
    }

    size_t n_children = dict->number_children;
    __myfs_off_t *children = off_to_ptr(fsptr, dict->children);
    node_t *node = NULL;

    // Check if we need to go to the parent directory (special case for "..")
    if (strcmp(child, "..") == 0) {
        return ((node_t *)off_to_ptr(fsptr, children[0])); // Return parent node
    }

    // Start iterating from the second child because the first one is ".." (parent)
    for (size_t i = 1; i < n_children; i++) {
        node = ((node_t *)off_to_ptr(fsptr, children[i]));
        
        // Check if this node's name matches the requested child
        if (strcmp(node->name, child) == 0) {
            return node;  // Found matching node, return it
        }
    }

    // If no matching node was found, return NULL
    return NULL;
}



/* Resolves a path by breaking it into tokens and traversing through the directory structure.
 * Returns the node corresponding to the last path element or NULL if the path is invalid.
 */
node_t *path_solver(void *fsptr, const char *path, int skip_n_tokens) {
    // Ensure the path starts with '/'
    if (*path != '/') {
        return NULL;  // Invalid path if it does not start with root '/'
    }

    // Retrieve the root directory node
    node_t *node = off_to_ptr(fsptr, ((myfs_handle_t *)fsptr)->root_dir);

    // If the path is just root '/', return the root directory node
    if (path[1] == '\0') {
        return node;
    }

    // Tokenize the path by splitting on '/'
    char **tokens = tokenize('/', path, skip_n_tokens);

    // Iterate through each token in the path
    for (char **token = tokens; *token; token++) {
        // If the current node is a file, it cannot have children, return NULL
        if (node->is_file) {
            free_tokens(tokens);  // Clean up tokens before returning
            return NULL;
        }

        // If the token is ".", stay in the current directory (no movement)
        if (strcmp(*token, ".") != 0) {
            // Retrieve the child node corresponding to the current token
            node = get_node(fsptr, &node->type.directory, *token);
            // Check if the node was found, return NULL if not
            if (node == NULL) {
                free_tokens(tokens);  // Clean up tokens before returning
                return NULL;
            }
        }
    }

    // Clean up token array after use
    free_tokens(tokens);

    // Return the resolved node after processing all tokens
    return node;
}



/* Creates an inode (node) in the filesystem at the specified path. 
 * The function ensures the parent directory exists, the name is valid, 
 * and there is enough space for the new inode. The inode can either be 
 * a directory or a file, depending on the `isfile` flag.
 */
node_t *make_inode(void *fsptr, const char *path, int *errnoptr, int isfile) {
    // Retrieve the parent node by solving the path without the last token (filename)
    node_t *parent_node = path_solver(fsptr, path, 1);
    
    // Check if the parent node exists and is not a file
    if (parent_node == NULL) {
        *errnoptr = ENOENT;  // Parent directory does not exist
        return NULL;
    }
    if (parent_node->is_file) {
        *errnoptr = ENOTDIR;  // Parent is a file, not a directory
        return NULL;
    }

    // Get the directory structure from the parent node
    directory_t *dict = &parent_node->type.directory;

    // Extract the filename from the path
    unsigned long len;
    char *new_node_name = get_last_token(path, &len);

    // Check if a node with the same name already exists in the parent directory
    if (get_node(fsptr, dict, new_node_name) != NULL) {
        *errnoptr = EEXIST;  // File or directory with the same name exists
        return NULL;
    }

    // Validate the filename length
    if (len == 0) {
        *errnoptr = ENOENT;  // Filename is empty
        return NULL;
    }
    if (len > (size_t)255) {
        *errnoptr = ENAMETOOLONG;  // Filename is too long
        return NULL;
    }

    // Access the parent directory's children list
    __myfs_off_t *children = off_to_ptr(fsptr, dict->children);
    allocated_node_t *block = (((void *)children) - sizeof(size_t));

    // Check if there is enough space to add a new child node
    size_t max_children = block->remaining / sizeof(__myfs_off_t);
    size_t ask_size;

    // If no space, request more space for children
    if (max_children == dict->number_children) {
        ask_size = block->remaining * 2;
        void *new_children = __realloc_impl(fsptr, children, &ask_size);

        if (ask_size != 0) {
            *errnoptr = ENOSPC;  // Not enough space to expand the children list
            return NULL;
        }

        // Update the children offset
        dict->children = ptr_to_off(fsptr, new_children);
        children = ((__myfs_off_t *)new_children);
    }

    // Allocate memory for the new node
    ask_size = sizeof(node_t);
    node_t *new_node = (node_t *)__malloc_impl(fsptr, NULL, &ask_size);

    if ((ask_size != 0) || (new_node == NULL)) {
        __free_impl(fsptr, new_node);
        *errnoptr = ENOSPC;  // Not enough space to create the node
        return NULL;
    }

    // Initialize the new node's name and set its timestamps
    memset(new_node->name, '\0', (size_t)255 + 1);  // Initialize name to null
    memcpy(new_node->name, new_node_name, len);  // Copy the filename into the node
    update_time(new_node, 1);

    // Add the new node to the parent's children list
    children[dict->number_children] = ptr_to_off(fsptr, new_node);
    dict->number_children++;
    update_time(parent_node, 1);

    // If creating a file
    if (isfile) {
        new_node->is_file = 1;
        file_t *file = &new_node->type.file;
        file->total_size = 0;
        file->first_file_block = 0;  // Initialize with no blocks
    } else {  // If creating a directory
        new_node->is_file = 0;
        dict = &new_node->type.directory;
        dict->number_children = 1;  // Use the first child for ".."

        // Allocate space for 4 children in the new directory
        ask_size = 4 * sizeof(__myfs_off_t);
        __myfs_off_t *ptr = ((__myfs_off_t *)__malloc_impl(fsptr, NULL, &ask_size));

        if ((ask_size != 0) || (ptr == NULL)) {
            __free_impl(fsptr, ptr);
            *errnoptr = ENOSPC;  // Not enough space to allocate children
            return NULL;
        }

        // Set up the directory's children list
        dict->children = ptr_to_off(fsptr, ptr);
        *ptr = ptr_to_off(fsptr, parent_node);  // Set ".." to point to the parent
    }

    // Return the newly created inode
    return new_node;
}



/* Frees the memory allocated for the file's blocks, including all its data.
 * It iterates through each file block, frees the block data, and then
 * the block itself until the last block is reached.
 */
void free_file_info(void *fsptr, file_t *file) {
    myfs_file_block_t *block = off_to_ptr(fsptr, file->first_file_block);
    myfs_file_block_t *next;

    // Iterate over all blocks in the file until we reach the end
    // The last block's pointer will point to fsptr, signaling the end of the list
    while (((void *)block) != fsptr) {
        // Free the data of the current block
        __free_impl(fsptr, off_to_ptr(fsptr, block->data));

        // Save the pointer to the next block for the next iteration
        next = off_to_ptr(fsptr, block->next_file_block);

        // Free the current block itself
        __free_impl(fsptr, block);

        // Move to the next block
        block = next;
    }
}



/* Removes a node from a directory and re-adjusts the children list. It also
 * attempts to free memory by reducing the size of the children array if possible.
 */
void remove_node(void *fsptr, directory_t *dict, node_t *node) {
    size_t n_children = dict->number_children;
    __myfs_off_t *children = off_to_ptr(fsptr, dict->children);
    size_t index;
    __myfs_off_t node_off = ptr_to_off(fsptr, node);

    // Find the index of the node to be removed in the children list
    for (index = 1; index < n_children; index++) {
        if (children[index] == node_off) {
            break;
        }
    }

    // Free the node itself
    __free_impl(fsptr, node);

    // Shift the remaining nodes one position to the left to cover the removed node
    for (; index < n_children - 1; index++) {
        children[index] = children[index + 1];
    }

    // Set the last slot to zero and update the number of children
    children[index] = (__myfs_off_t)0;
    dict->number_children--;

    // Try to reduce the size of the children array by half while ensuring there are
    // at least 4 child slots available and space for an allocated_node_t object
    size_t new_n_children = (*((size_t *)children) - 1) / sizeof(__myfs_off_t);
    new_n_children <<= 1;  // Divide the available slots by 2

    // Check if the new number of children is sufficient, while also ensuring there is
    // enough space for at least 4 children and an allocated_node_t structure
    if ((new_n_children >= dict->number_children) &&
        (new_n_children * sizeof(__myfs_off_t) >= sizeof(allocated_node_t)) &&
        (new_n_children >= 4)) {

        // Proceed to create an AllocateFrom structure and add it to the list of free blocks
        allocated_node_t *temp = (allocated_node_t *)&children[new_n_children];
        temp->remaining = new_n_children * sizeof(__myfs_off_t) - sizeof(size_t);
        temp->next_space = 0;
        __free_impl(fsptr, temp);

        // Update the size of the current directory's children array
        size_t *new_size = (size_t *)(children - 1);
        *new_size -= (temp->remaining - sizeof(size_t));
    }
}



/* Removes data from a file starting at a specific block, freeing memory
 * as necessary. The function iterates through blocks and deallocates data
 * until the specified size is removed.
 */
void remove_data(void *fsptr, myfs_file_block_t *block, size_t size) {
    // Return if no block is provided to remove data from
    if (block == NULL) {
        return;
    }

    size_t idx = 0;

    // Iterate through the blocks until the required size is removed
    while (size != 0) {
        // If we need to cut data within this block
        if (size > block->allocated) {
            // Decrease size by the amount allocated in the current block
            size -= block->allocated;
            // Move to the next block
            block = off_to_ptr(fsptr, block->next_file_block);
        } else {
            // Cut the data within the current block
            idx = size;
            size = 0;
        }
    }

    // If there is data to free within the current block
    if ((idx + sizeof(allocated_node_t)) < block->allocated) {
        // Set the header for the block data after cutting
        size_t *temp = (size_t *)(&((char *)off_to_ptr(fsptr, block->data))[idx]);
        *temp = block->allocated - idx - sizeof(size_t);

        // Free the data portion of the block after the header
        __free_impl(fsptr, ((void *)temp) + sizeof(size_t));
    }

    // Free all data and blocks after the current block
    block = off_to_ptr(fsptr, block->next_file_block);
    myfs_file_block_t *temp;

    // Iterate through the remaining blocks and free them
    while (block != fsptr) {
        // Free the data block
        __free_impl(fsptr, off_to_ptr(fsptr, block->data));

        // Get the next block before freeing the current one
        temp = off_to_ptr(fsptr, block->next_file_block);
        // Free the current block
        __free_impl(fsptr, block);

        // Update the block pointer to the next one
        block = temp;
    }
}




/* Adds data to a file by allocating blocks as necessary. The function
 * extends the last block if possible, appends new blocks, and fills
 * them with zeroes if there is not enough space.
 */
int add_data(void *fsptr, file_t *file, size_t size, int *errnoptr) {
    myfs_file_block_t *block = off_to_ptr(fsptr, file->first_file_block);

    // Auxiliary variables for managing blocks and sizes
    myfs_file_block_t *prev_temp_block = NULL;
    myfs_file_block_t *temp_block;
    size_t new_data_block_size;
    size_t ask_size;
    size_t append_n_bytes;
    size_t initial_file_size = file->total_size;
    void *data_block;

    // If the file is empty, create the first block manually
    if (((void *)block) == fsptr) {
        ask_size = sizeof(myfs_file_block_t);
        block = __malloc_impl(fsptr, NULL, &ask_size);
        if (ask_size != 0) {
            *errnoptr = ENOSPC;
            return -1; // No space left to allocate the first block
        }

        // Set the first file block and allocate data space
        file->first_file_block = ptr_to_off(fsptr, block);
        ask_size = size;
        data_block = __malloc_impl(fsptr, NULL, &ask_size);

        // Initialize block with size and data allocation
        block->size = size - ask_size;
        block->allocated = block->size;
        block->data = ptr_to_off(fsptr, data_block);
        block->next_file_block = 0;

        size -= block->size;
    }
    // If the file is not empty, extend the last block by appending zeroes
    else {
        // Traverse blocks until the last one
        while (block->next_file_block != 0) {
            size -= block->allocated;
            block = off_to_ptr(fsptr, block->next_file_block);
        }

        // Calculate how many zeroes to append to the last block
        append_n_bytes = (block->size - block->allocated) > size
                             ? size
                             : (block->size - block->allocated);
        data_block = &((char *)off_to_ptr(fsptr, block->data))[block->allocated];

        // Append zeroes to the last block
        memset(data_block, 0, append_n_bytes);
        block->allocated += append_n_bytes;
        size -= append_n_bytes;
    }

    // If no more data is left to add, return early
    if (size == 0) {
        return 0;
    }

    // Otherwise, allocate new blocks until the required size is met
    size_t prev_size = block->allocated;
    ask_size = size;
    data_block = ((char *)off_to_ptr(fsptr, block->data));
    void *new_data_block = __malloc_impl(fsptr, data_block, &ask_size);
    block->size = *(((size_t *)data_block) - 1);  // Update the block's total size

    // Check if allocation failed or if the requested size exceeds available space
    if (new_data_block == NULL) {
        if (ask_size != 0) {
            *errnoptr = ENOSPC;
            return -1; // No space available for new block
        } else {
            // If allocation size is zero, extend the existing block with zeroes
            append_n_bytes = (block->size - block->allocated >= size
                                ? size
                                : block->size - block->allocated);
            memset(&((char *)off_to_ptr(fsptr, block->data))[prev_size], 0,
                   append_n_bytes);
            block->allocated += append_n_bytes;
            size = 0;
        }
    } else {
        // Allocate and initialize new blocks until all data is added
        append_n_bytes = block->size - block->allocated;
        memset(&((char *)off_to_ptr(fsptr, block->data))[prev_size], 0,
               append_n_bytes);
        block->allocated += append_n_bytes;
        size -= append_n_bytes;

        temp_block = block;

        // Allocate and chain new blocks until all data is added or fail
        while (1) {
            new_data_block_size = *(((size_t *)new_data_block) - 1);

            // Link the previous block to the current block
            if (prev_temp_block != NULL) {
                prev_temp_block->next_file_block = ptr_to_off(fsptr, temp_block);
            }

            // Initialize the current block
            temp_block->size = new_data_block_size;
            temp_block->allocated = ask_size == 0 ? size : new_data_block_size;
            temp_block->data = ptr_to_off(fsptr, new_data_block);
            temp_block->next_file_block = 0;

            memset(new_data_block, 0, temp_block->allocated);
            size -= temp_block->allocated;

            prev_temp_block = temp_block;

            // If all data is added, exit the loop
            if (size == 0) {
                break;
            }

            // Allocate the next block of data
            ask_size = size;
            new_data_block = __malloc_impl(fsptr, NULL, &ask_size);
            size_t temp_size = sizeof(myfs_file_block_t);
            temp_block = __malloc_impl(fsptr, NULL, &temp_size);

            // Ensure successful allocation, otherwise rollback and return error
            if ((new_data_block == NULL) || (temp_block == NULL) || (temp_size != 0)) {
                remove_data(fsptr, off_to_ptr(fsptr, file->first_file_block),
                            initial_file_size);
                *errnoptr = ENOSPC;
                return -1; // Failed to allocate enough space for the file
            }
        }
    }

    return 0; // Data successfully added to the file
}





/* End of helper functions */

/* Implements an emulation of the stat system call on the filesystem 
   of size fssize pointed to by fsptr. 
   
   If path can be followed and describes a file or directory 
   that exists and is accessable, the access information is 
   put into stbuf. 

   On success, 0 is returned. On failure, -1 is returned and 
   the appropriate error code is put into *errnoptr.

   man 2 stat documents all possible error codes and gives more detail
   on what fields of stbuf need to be filled in. Essentially, only the
   following fields need to be supported:

   st_uid      the value passed in argument
   st_gid      the value passed in argument
   st_mode     (as fixed values S_IFDIR | 0755 for directories,
                                S_IFREG | 0755 for files)
   st_nlink    (as many as there are subdirectories (not files) for directories
                (including . and ..),
                1 for files)
   st_size     (supported only for files, where it is the real file size)
   st_atim
   st_mtim

*/
int __myfs_getattr_implem(void *fsptr, size_t fssize, int *errnoptr,
                          uid_t uid, gid_t gid,
                          const char *path, struct stat *stbuf) {
  // Validate filesystem pointer and size
  initial_check(fsptr, fssize); 

  // Validate path
  if (path == NULL || strlen(path) == 0) {
    *errnoptr = ENOENT; // Path does not exist
    return -1;
  }

  // Locate the file or directory node
  node_t *node = path_solver(fsptr, path, 0);
  if (node == NULL) {
    *errnoptr = ENOENT; // File or directory not found
    return -1;
  }

  // Set UID and GID
  stbuf->st_uid = uid;
  stbuf->st_gid = gid;

  // Populate fields based on node type
  if (node->is_file) {
    // File-specific attributes
    stbuf->st_mode = S_IFREG;
    stbuf->st_nlink = ((nlink_t)1);            // Files have 1 link
    stbuf->st_size = ((off_t)node->type.file.total_size); // File size
  } else {
    // Directory-specific attributes
    stbuf->st_mode = S_IFDIR;

    // Count the number of subdirectories, including `.` and `..`
    directory_t *dir = &node->type.directory;
    stbuf->st_nlink = 2; // Start with `.` and `..`
    if (dir->children != 0) {
      __myfs_off_t *children_offsets = off_to_ptr(fsptr, dir->children);
      size_t num_children = dir->number_children;

      for (size_t i = 0; i < num_children; i++) {
        node_t *child = off_to_ptr(fsptr, children_offsets[i]);
        if (!child->is_file) {
          stbuf->st_nlink++; // Increment for each subdirectory
        }
      }
    }
  }

  // Set access and modification times
  stbuf->st_atim = node->times[0]; // Last access time
  stbuf->st_mtim = node->times[1]; // Last modification time

  return 0; // Success
}

/* Implements an emulation of the readdir system call on the filesystem 
   of size fssize pointed to by fsptr. 

   If path can be followed and describes a directory that exists and
   is accessable, the names of the subdirectories and files 
   contained in that directory are output into *namesptr. The . and ..
   directories must not be included in that listing.

   If it needs to output file and subdirectory names, the function
   starts by allocating (with calloc) an array of pointers to
   characters of the right size (n entries for n names). Sets
   *namesptr to that pointer. It then goes over all entries
   in that array and allocates, for each of them an array of
   characters of the right size (to hold the i-th name, together 
   with the appropriate '\0' terminator). It puts the pointer
   into that i-th array entry and fills the allocated array
   of characters with the appropriate name. The calling function
   will call free on each of the entries of *namesptr and 
   on *namesptr.

   The function returns the number of names that have been 
   put into namesptr. 

   If no name needs to be reported because the directory does
   not contain any file or subdirectory besides . and .., 0 is 
   returned and no allocation takes place.

   On failure, -1 is returned and the *errnoptr is set to 
   the appropriate error code. 

   The error codes are documented in man 2 readdir.

   In the case memory allocation with malloc/calloc fails, failure is
   indicated by returning -1 and setting *errnoptr to EINVAL.

*/
int __myfs_readdir_implem(void *fsptr, size_t fssize, int *errnoptr,
                          const char *path, char ***namesptr) {
  // Validate filesystem pointer and size
  initial_check(fsptr, fssize);

  // Validate path
  if (path == NULL || strlen(path) == 0) {
    *errnoptr = ENOENT; // Path does not exist
    return -1;
  }

  // Locate the directory node
  node_t *node = path_solver(fsptr, path, 0);
  if (node == NULL) {
    *errnoptr = ENOENT; // Directory not found
    return -1;
  }

  // Check if the path refers to a directory
  if (node->is_file) {
    *errnoptr = ENOTDIR; // Not a directory
    return -1;
  }

  // Get the directory structure
  directory_t *dir = &node->type.directory;

  // Check that directory have more than ".", ".." nodes inside
  if (dir->number_children == 1) {
    return 0;
  }

  // Get the children array
  __myfs_off_t *children_offsets = off_to_ptr(fsptr, dir->children);
  size_t num_children = dir->number_children;

  // Filter out `.` and `..` entries and count remaining valid entries
  size_t valid_entries = 0;
  for (size_t i = 0; i < num_children; i++) {
    node_t *child = off_to_ptr(fsptr, children_offsets[i]);
    if (strcmp(child->name, ".") != 0 && strcmp(child->name, "..") != 0) {
      valid_entries++;
    }
  }

  // If no valid entries, return 0
  if (valid_entries == 0) {
    *namesptr = NULL;
    return 0;
  }

  // Allocate memory for the array of names
  char **names = calloc(valid_entries, sizeof(char *));
  if (names == NULL) {
    *errnoptr = EINVAL; // Memory allocation failure
    return -1;
  }

  // Populate the names array with valid entries
  size_t index = 0;
  for (size_t i = 0; i < num_children; i++) {
    node_t *child = off_to_ptr(fsptr, children_offsets[i]);
    if (strcmp(child->name, ".") != 0 && strcmp(child->name, "..") != 0) {
      // Allocate memory for the name and copy it
      size_t name_length = strlen(child->name) + 1;
      names[index] = calloc(name_length, sizeof(char));
      if (names[index] == NULL) {
        // Free previously allocated memory and return error
        for (size_t j = 0; j < index; j++) {
          free(names[j]);
        }
        free(names);
        *errnoptr = EINVAL;
        return -1;
      }
      strncpy(names[index], child->name, name_length);
      index++;
    }
  }

  // Set the output pointer and return the number of valid entries
  *namesptr = names;
  return valid_entries;
}

/* Implements an emulation of the mknod system call for regular files
   on the filesystem of size fssize pointed to by fsptr.

   This function is called only for the creation of regular files.

   If a file gets created, it is of size zero and has default
   ownership and mode bits.

   The call creates the file indicated by path.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 mknod.

*/
int __myfs_mknod_implem(void *fsptr, size_t fssize, int *errnoptr,
                        const char *path) {
  initial_check(fsptr, fssize);

  // Make a directory, 1 because it is a file
  node_t *node = make_inode(fsptr, path, errnoptr, 1);

  // Check if the node was successfully created, if it wasn't the errnoptr was
  // already set so we just return failure with -1
  if (node == NULL) {
    return -1;
  }
  
  return 0; // Success
}

/* Implements an emulation of the unlink system call for regular files
   on the filesystem of size fssize pointed to by fsptr.

   This function is called only for the deletion of regular files.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 unlink.

*/
int __myfs_unlink_implem(void *fsptr, size_t fssize, int *errnoptr,
                        const char *path) {
  // Validate the filesystem pointer and size
  initial_check(fsptr, fssize);

  // Validate the path
  if (path == NULL || strlen(path) == 0) {
    *errnoptr = EINVAL; // Invalid path
    return -1;
  }

  // Locate the node for the file to delete
  node_t *node = path_solver(fsptr, path, 0);
  if (node == NULL) {
    *errnoptr = ENOENT; // File does not exist
    return -1;
  }

  // Check if the node is a regular file
  if (!node->is_file) {
    *errnoptr = ENOTDIR; // Cannot unlink a directory
    return -1;
  }

  // Get directory from node
  directory_t *dict = &node->type.directory;

  // Get last token which have the filename
  unsigned long len;
  char *filename = get_last_token(path, &len);

  // Check that the parent does not contain a node with the same name as the one we are about to create
  node_t *file_node = get_node(fsptr, dict, filename);
  if (file_node == NULL) {
    *errnoptr = ENOENT;
    return -1;
  }

  // Check that file_node is actually a file
  if (!file_node->is_file) {
    *errnoptr = EISDIR; // Path given lead to a directory not a file
    return -1;
  }

  // Free file information
  file_t *file = &file_node->type.file;
  if (file->total_size != 0) {
    free_file_info(fsptr, file);
  }

  // Remove file_node from parent directory
  remove_node(fsptr, dict, file_node);

  return 0; // Success
}

/* Implements an emulation of the rmdir system call on the filesystem 
   of size fssize pointed to by fsptr. 

   The call deletes the directory indicated by path.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The function call must fail when the directory indicated by path is
   not empty (if there are files or subdirectories other than . and ..).

   The error codes are documented in man 2 rmdir.

*/
int __myfs_rmdir_implem(void *fsptr, size_t fssize, int *errnoptr,
                        const char *path) {
  // Validate the filesystem pointer and size
  initial_check(fsptr, fssize);

  // Validate the path
  if (path == NULL || strlen(path) == 0) {
    *errnoptr = EINVAL; // Invalid path
    return -1;
  }

  // Locate the directory node
  node_t *dir_node = path_solver(fsptr, path, 0);
  if (dir_node == NULL) {
    *errnoptr = ENOENT; // Directory does not exist
    return -1;
  }

  // Check if the node is a directory
  if (!dir_node->is_file) {
    *errnoptr = ENOTDIR; // Path is not a directory
    return -1;
  }

  // Ensure the directory is empty (excluding "." and "..")
  directory_t *dir = &dir_node->type.directory;
  if (dir->number_children > 0) {
    *errnoptr = ENOTEMPTY; // Directory is not empty
    return -1;
  }

  // Get parent directory
  __myfs_off_t *children = off_to_ptr(fsptr, dir->children);
  node_t *parent_node = off_to_ptr(fsptr, *children);

  // Free children of node and the node itself
  __free_impl(fsptr, children);
  remove_node(fsptr, &parent_node->type.directory, dir_node);

  return 0; // Success
}

/* Implements an emulation of the mkdir system call on the filesystem 
   of size fssize pointed to by fsptr. 

   The call creates the directory indicated by path.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 mkdir.

*/
int __myfs_mkdir_implem(void *fsptr, size_t fssize, int *errnoptr,
                        const char *path) {
  initilaize_check(fsptr, fssize);

  // Make a directory, 0 because it is not a file
  node_t *node = make_inode(fsptr, path, errnoptr, 0);

  if (node == NULL) {
    return -1;
  }

  return 0;
}

/* Implements an emulation of the rename system call on the filesystem 
   of size fssize pointed to by fsptr. 

   The call moves the file or directory indicated by from to to.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   Caution: the function does more than what is hinted to by its name.
   In cases the from and to paths differ, the file is moved out of 
   the from path and added to the to path.

   The error codes are documented in man 2 rename.

*/
int __myfs_rename_implem(void *fsptr, size_t fssize, int *errnoptr,
                         const char *from, const char *to) {
  // Check if filesystem is valid
  initilaize_check(fsptr, fssize);

  // DO WORK

  return -1; // Success
}

/* Implements an emulation of the truncate system call on the filesystem 
   of size fssize pointed to by fsptr. 

   The call changes the size of the file indicated by path to offset
   bytes.

   When the file becomes smaller due to the call, the extending bytes are
   removed. When it becomes larger, zeros are appended.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 truncate.

*/
int __myfs_truncate_implem(void *fsptr, size_t fssize, int *errnoptr,
                           const char *path, off_t offset) {
    // Validate input parameters
    initial_check(fsptr, fssize);
    if (offset < 0) {
        *errnoptr = EFAULT;
        return -1;
    }

    // Locate the file by path
    node_t *node = path_solver(fsptr, path, 0);
    if (node == NULL) {
        *errnoptr = ENOENT; // File not found
        return -1;
    }

    // Ensure the node is a file
    if (!node->is_file) {
        *errnoptr = EISDIR; // Path refers to a directory
        return -1;
    }

    // Access the file structure
    file_t *file = &node->type.file;

    // File size stays the same 
    if (file->total_size == ((size_t)offset)){
        update_time(node,0);
        return 0; 
    } 
    // Handle shrinking the file
    else if ((size_t)offset < file->total_size) {
        myfs_file_block_t *first_block = off_to_ptr(fsptr, file->first_file_block);

        // Update access and modifiy times
        update_time(node, 1);

        // Remove data after the offset
        remove_data(fsptr, first_block, ((size_t)offset));

        // Update the total size
        file->total_size = ((size_t)offset);

    }
    // Handle extending the file 
    else {
        // Handle extending the file with zeros
        if (add_data(fsptr, file, ((size_t)offset), errnoptr) == -1) {
            // Error occurred while allocating space
            return -1;
        }
    }

    // Update the total size
    file->total_size = ((size_t)offset);

    // If we reach here, the truncate was successful
    return 0;
}

/* Implements an emulation of the open system call on the filesystem 
   of size fssize pointed to by fsptr, without actually performing the opening
   of the file (no file descriptor is returned).

   The call just checks if the file (or directory) indicated by path
   can be accessed, i.e. if the path can be followed to an existing
   object for which the access rights are granted.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The two only interesting error codes are 

   * EFAULT: the filesystem is in a bad state, we can't do anything

   * ENOENT: the file that we are supposed to open doesn't exist (or a
             subpath).

   It is possible to restrict ourselves to only these two error
   conditions. It is also possible to implement more detailed error
   condition answers.

   The error codes are documented in man 2 open.

*/
int __myfs_open_implem(void *fsptr, size_t fssize, int *errnoptr,
                       const char *path) {
  // Ensure the filesystem is initialized and valid
  initial_check(fsptr, fssize);

  // Validate the input path
  if (path == NULL || strlen(path) == 0) {
    *errnoptr = ENOENT; // No such file or directory
    return -1;
  }

  // Resolve the path to locate the corresponding node
  node_t *node = path_solver(fsptr, path, 0);

  // If the path is invalid or doesn't exist
  if (node == NULL) {
    *errnoptr = ENOENT; // No such file or directory
    return -1;
  }

  // If we reach here, the file or directory exists and is accessible
  return 0;
}

/* Implements an emulation of the read system call on the filesystem 
   of size fssize pointed to by fsptr.

   The call copies up to size bytes from the file indicated by 
   path into the buffer, starting to read at offset. See the man page
   for read for the details when offset is beyond the end of the file etc.
   
   On success, the appropriate number of bytes read into the buffer is
   returned. The value zero is returned on an end-of-file condition.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 read.

*/
int __myfs_read_implem(void *fsptr, size_t fssize, int *errnoptr,
                       const char *path, char *buf, size_t size, off_t offset) {
  // Validate filesystem pointer and size
  initial_check(fsptr, fssize);

  if (offset < 0) {
    *errnoptr = EFAULT;
    return -1;
  }

  // Validate path
  if (path == NULL || strlen(path) == 0) {
    *errnoptr = ENOENT; // Invalid or non-existent path
    return -1;
  }

  // Validate buffer
  if (buf == NULL || size == 0) {
    *errnoptr = EFAULT; // Invalid argument
    return -1;
  }

  // Locate the file node
  node_t *node = path_solver(fsptr, path, 0);
  if (node == NULL) {
    *errnoptr = ENOENT; // File not found
    return -1;
  }

  // Ensure the node represents a file
  if (!node->is_file) {
    *errnoptr = EISDIR; // Path refers to a directory
    return -1;
  }

  file_t *file = &node->type.file;

  // If offset is beyond the file size
  if ((size_t)offset > file->total_size) {
    *errnoptr = EFAULT;
    return -1;
  }

  if (file->total_size == 0) {
    return 0;
  }

  // Determine the number of bytes to read, ensuring not to exceed file size
  size_t bytes_to_read = (offset + size > file->total_size) ? file->total_size - (size_t)offset : size;

  // Find the file block corresponding to the offset
  myfs_file_block_t *block = off_to_ptr(fsptr, file->first_file_block);
  size_t block_offset = 0;
  size_t read_offset = (size_t)offset;

  // Traverse to the block containing the starting offset
  while (read_offset >= block->size) {
    read_offset -= block->size;
    block = off_to_ptr(fsptr, block->next_file_block);
    if (block == NULL) {
      *errnoptr = EFAULT; // Corrupted filesystem
      return -1;
    }
  }

  // Read data into the buffer
  size_t bytes_read = 0;
  while (bytes_to_read > 0 && block != NULL) {
    // Determine the amount to read from the current block
    size_t read_from_block = (read_offset + bytes_to_read > block->size) ? block->size - read_offset : bytes_to_read;

    // Copy data from the block to the buffer
    char *block_data = (char *)off_to_ptr(fsptr, block->data);
    memcpy(buf + bytes_read, block_data + read_offset, read_from_block);

    // Update counters and pointers
    bytes_read += read_from_block;
    bytes_to_read -= read_from_block;
    read_offset = 0; // Subsequent blocks are read from the start
    block = off_to_ptr(fsptr, block->next_file_block);
  }

  // Update the last access time of the file
  update_time(node, 0); 

  return bytes_read;
}

/* Implements an emulation of the write system call on the filesystem 
   of size fssize pointed to by fsptr.

   The call copies up to size bytes to the file indicated by 
   path into the buffer, starting to write at offset. See the man page
   for write for the details when offset is beyond the end of the file etc.
   
   On success, the appropriate number of bytes written into the file is
   returned. The value zero is returned on an end-of-file condition.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 write.

*/
int __myfs_write_implem(void *fsptr, size_t fssize, int *errnoptr,
                        const char *path, const char *buf, size_t size, off_t offset) {
  // Validate filesystem pointer and size
  initial_check(fsptr, fssize); 

  if (offset < 0) {
    *errnoptr = EFAULT;
    return -1;
  }

  // Validate path
  if (path == NULL || strlen(path) == 0) {
    *errnoptr = ENOENT; // Invalid or non-existent path
    return -1;
  }

  // Validate buffer
  if (buf == NULL || size == 0) {
    *errnoptr = EINVAL; // Invalid argument
    return -1;
  }

  // Locate the file node
  node_t *node = path_solver(fsptr, path, 0);
  if (node == NULL) {
    *errnoptr = ENOENT; // File not found
    return -1;
  }

  // Ensure the node represents a file
  if (!node->is_file) {
    *errnoptr = EISDIR; // Path refers to a directory
    return -1;
  }

  // Check that the file have more bytes than the size_left so we don't have to iterate it
  file_t *file = &node->type.file;
  if (file->total_size < ((size_t)offset)) {
    *errnoptr = EFBIG;
    return -1;
  }

  // Calculate the total size after the write operation
  size_t new_size = (size_t)offset + size;

  // Extend the file if necessary
  if (new_size > file->total_size) {
    if (add_data(fsptr, file, new_size, errnoptr) != 0) {
      return -1; // Failed to allocate new data blocks
    }
    file->total_size = new_size; // Update the file size
  }

  // Locate the block where the write begins
  myfs_file_block_t *block = off_to_ptr(fsptr, file->first_file_block);
  size_t write_offset = (size_t)offset;
  while (write_offset >= block->size) {
    write_offset -= block->size;
    block = off_to_ptr(fsptr, block->next_file_block);
    if (block == NULL) {
      *errnoptr = EFAULT; // Corrupted filesystem
      return -1;
    }
  }

  // Write the data to the file blocks
  size_t bytes_written = 0;
  while (size > 0 && block != NULL) {
    // Determine how much to write to the current block
    size_t write_to_block = (write_offset + size > block->size) ? block->size - write_offset : size;

    // Write data from the buffer to the block
    char *block_data = (char *)off_to_ptr(fsptr, block->data);
    memcpy(block_data + write_offset, buf + bytes_written, write_to_block);

    // Update counters and pointers
    bytes_written += write_to_block;
    size -= write_to_block;
    write_offset = 0; // Subsequent blocks are written from the start
    block = off_to_ptr(fsptr, block->next_file_block);
  }

  // Update the modification and access times of the file
  update_time(node, 1); 

  return bytes_written;

}

/* Implements an emulation of the utimensat system call on the filesystem 
   of size fssize pointed to by fsptr.

   The call changes the access and modification times of the file
   or directory indicated by path to the values in ts.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 utimensat.

*/
int __myfs_utimens_implem(void *fsptr, size_t fssize, int *errnoptr,
                          const char *path, const struct timespec ts[2]) {
  // Ensure the filesystem is initialized and valid
  initial_check(fsptr, fssize);

  // Validate the path
  if (path == NULL || strlen(path) == 0) {
    *errnoptr = ENOENT; // Invalid path or file not found
    return -1;
  }

  // Find the node corresponding to the given path
  node_t *node = path_solver(fsptr, path, 0);
  if (node == NULL) {
    *errnoptr = ENOENT; // File or directory does not exist
    return -1;
  }
  
  node->times[0] = ts[0]; // Last access time
  node->times[1] = ts[1]; // Last modification time

  return 0; // Success
}

/* Implements an emulation of the statfs system call on the filesystem 
   of size fssize pointed to by fsptr.

   The call gets information of the filesystem usage and puts in 
   into stbuf.

   On success, 0 is returned.

   On failure, -1 is returned and *errnoptr is set appropriately.

   The error codes are documented in man 2 statfs.

   Essentially, only the following fields of struct statvfs need to be
   supported:

   f_bsize   fill with what you call a block (typically 1024 bytes)
   f_blocks  fill with the total number of blocks in the filesystem
   f_bfree   fill with the free number of blocks in the filesystem
   f_bavail  fill with same value as f_bfree
   f_namemax fill with your maximum file/directory name, if your
             filesystem has such a maximum

*/
int __myfs_statfs_implem(void *fsptr, size_t fssize, int *errnoptr,
                         struct statvfs* stbuf) {
  // Ensure the filesystem is initialized and valid
  initial_check(fsptr, fssize);

  // Ensure the stbuf pointer is valid
  if (stbuf == NULL) {
    *errnoptr = EINVAL; // Invalid argument
    return -1;
  }

  // Cast the handler to extract metadata from the filesystem
  myfs_handle_t *handler = (myfs_handle_t *)fsptr;

  // Total number of blocks in the filesystem
  stbuf->f_blocks = fssize / 1024;

  // Number of free blocks available
  allocated_node_t *free_space = off_to_ptr(fsptr, handler->free_memory);
  size_t free_memory_size = free_space->remaining;
  stbuf->f_bfree = free_memory_size / 1024;

  // Number of available blocks (same as f_bfree in this case)
  stbuf->f_bavail = stbuf->f_bfree;

  // Filesystem block size
  stbuf->f_bsize = 1024;

  // Maximum file name length
  stbuf->f_namemax = 255;

  return 0;
}

