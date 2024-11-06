/* -MYFS is FUSE translated by lauter. We only care about
  line on how to compile. (`) are backquotes!

  -Once everything is compiled, mount filesystem in linux filesystem
 	How? We have a tree of directories that is the file system
	- You replace the root node with your root node.

	Mounted with ./myfs --backupfile=test.myfs ~/fuse-mnt/ -f
	- What is the backup file?
	- In the homework, we base filesystem to run completely in memory
	- It is already done in myfs by lauter, dont worry about allocating mem
	- Problem is memory goes away when off.
	- To fix this, we must use "backup file".
	- Our fs cannot be used as first fs because it needs another fs to
	store the backup file.

	Use GDB so you can see where something is segfaulting
	

 */


 /* Implementation.c
	all functions get a void ptr. 
	- Starting at that pointer, you can use fssize bytes to store your
	file system.
	
	Each function has an integer pointer (errnoptr).
	On success, all functions return 0.
	
	somethign wrong?
	fail and 
	ex.
	if (handle == NULL)
	*errnoptr = EFAULT; <--- These are #defined / #included already.
	if (node == NULL)
	*errnoptr = ENOENT;

	The manpages indicate which errors can happen on the system call.
	- Choose the right errors.
	On error reutrn -1 and set eptr to one of the man errors

	This is a fiile system in the kernel mode. We cannot just exit(1)
	Every error must be handled!
	
	/
	/
	/
	const char *path is a string of the name of a certain file
	in the file system.

	read{}
	__myfs_handle_t handle
	__myfs_inode_t *node
	size_t off, cpy;
	__myfs_off_t curr
	void *mc_start
	int res = -1;

	We need to use a trick to check if the filesystem is initalized
	- If it is initalized, keep it like this
	- If not, do it.

	On the first occurence to a call to any of these functions, do a call
	checking if the file system was intialized or not. 

	Get the fsptr of some esize, 
	If it is not initalized, everything in that memory chunk is 0.
	If it is initalized, it holds content of where it was last time we 
	stored it.

	We can have a specific 32 bit integer inside the four byte. That value
	is how we can tell if OUR data is there.
	This is how jpeg files work, the first four bytes have a specific
	sequence of bytes that tell us it is jpeg.
	
	So, in the begininning we check if the memory has our magic number, and
	other initialization work, then start executing.

	/
	/
	/
	When you implement a file system, we are implementing a data structure.
	- Create nodes, delete nodes, get more memory.
	In C, we use malloc,calloc, free. 
	- Do we need malloc here? NO, because everything we store needs to go
	intot the fsptr memory space. 
	So, start thinking about our own implementation of "allocate memory,
	reallocate memory, free memory" in the big chunk.

	in summary, implement some functions to do it.
	Think of it like this.
	At start, everything is free, Put magic number, free block ptr, 
	update free pointer to show where the free memory is.
	Again, update free pointer further down the fsize.
	If someone deletes a file, we create a linked list of free space.

	/
	/
	/
	Another difficulty we must overcome.
	We mount the file system, we get an fsptr at some address.
	- We write our magic, we indicate free blocks, etc.
	Fssize is something e.g. 0xdead
	We unmount it.
	We mount it back again. We get the same fsize, the same memroy
	contents as we left them.
	But, we get a different fsptr.
	What does this mean?
	- Inside the file system, there cannot be pointers because,
	addresses will not be consistent between mounts.
	How do we solve this?
	- Store the offset to the beginning of the fsptr.
	- By doing this, the offset is always the same relative to fsptr.
	
	/
	/
	/
	Lauters implementation of relative pointers.
	typedef size_t __myfs_off_t // Thesre are our (pointers)
	static inline void *__myfs_offset_to_ptr(void *fsptr, myfsofft off){
		if (off == ((myofft) 0)) return NULL;
		return fsptr+off;
	}
	Translates addresses to offsets.
	static inline myoft myfsptrtooffset(void *fsptr, void *ptr){
		if (ptr==NULL return myfsofft 0
		if (ptr <=fsptr return mtfsofft 0
		return (myfsofft) ptr - fsptr);

		Inside the file system, everthign stores only offsets.

		uint32_t magic
		myfsofft free_memory
		myfsoff_t root_dir
		size_t size; //// check if size hasn't changed.
	}
	
	When mounting use the whole path.
	
 */
