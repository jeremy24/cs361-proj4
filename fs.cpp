//COSC 361 Spring 2017
//FUSE Project Template
//Group Name
//Group Member 1 Jeremy Poff
//Group Member 2 Caleb Faban

#ifndef __cplusplus
#error "You must compile this using C++"
#endif
#include <fuse.h>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fs.h>


// our includes
#include <fstream>
#include <iostream>
#include <map>
#include <inttypes.h>
#include <vector>

using namespace std;

//Use debugf() and NOT printf() for your messages.
//Uncomment #define DEBUG in block.h if you want messages to show

//Here is a list of error codes you can return for
//the fs_xxx() functions
//
//EPERM          1      /* Operation not permitted */
//ENOENT         2      /* No such file or directory */
//ESRCH          3      /* No such process */
//EINTR          4      /* Interrupted system call */
//EIO            5      /* I/O error */
//ENXIO          6      /* No such device or address */
//ENOMEM        12      /* Out of memory */
//EACCES        13      /* Permission denied */
//EFAULT        14      /* Bad address */
//EBUSY         16      /* Device or resource busy */
//EEXIST        17      /* File exists */
//ENOTDIR       20      /* Not a directory */
//EISDIR        21      /* Is a directory */
//EINVAL        22      /* Invalid argument */
//ENFILE        23      /* File table overflow */
//EMFILE        24      /* Too many open files */
//EFBIG         27      /* File too large */
//ENOSPC        28      /* No space left on device */
//ESPIPE        29      /* Illegal seek */
//EROFS         30      /* Read-only file system */
//EMLINK        31      /* Too many links */
//EPIPE         32      /* Broken pipe */
//ENOTEMPTY     36      /* Directory not empty */
//ENAMETOOLONG  40      /* The name given is too long */

//Use debugf and NOT printf() to make your
//debug outputs. Do not modify this function.
#if defined(DEBUG)
int debugf(const char *fmt, ...)
{
	int bytes = 0;
	va_list args;
	va_start(args, fmt);
	bytes = vfprintf(stderr, fmt, args);
	va_end(args);
	return bytes;
}
#else
int debugf(const char *fmt, ...)
{
	return 0;
}
#endif

// our typedefs
typedef unsigned int uint;

typedef map< string, NODE * > NODEMAP;
typedef map< int64_t, BLOCK* > BLOCKMAP; 

typedef pair<string, NODE*> nodepair;

// a map of file paths to NODE* 
NODEMAP _nodes;

// a map or file paths to map<int, BLOCK*>
// the int is the block number
// this should make reordering blocks for fs_trunc easier
BLOCKMAP _blocks;

// a global reference to the header
BLOCK_HEADER * _header;

unsigned int _next_block_id = -1;

static void print_node(const NODE * local_node){

	return;

	debugf("file: %s\n\tid: %lu\n\tlink_id: %lu\n\t",
			local_node->name, local_node->id, local_node->link_id);
    debugf("mode: %lu\n\tctime: %lu\n\tatime: %lu\n\t",
		local_node->mode, local_node->ctime, local_node-> atime);
	debugf("mtime: %lu\n\tuid: %u\n\tgid: %u\n\tsize: %lu\n",
		local_node->mtime, local_node->uid, local_node->gid,local_node->size);

	const uint num = local_node -> size > 0 ? local_node -> size / _header->block_size + 1 : 0;
	uint i = 0;
	debugf("\tnum blocks: %d\n", num);
	debugf("\tblocks: ");
	while (i < num)
	{
		debugf("%d, ", (local_node -> blocks)[i++]);
	}
	debugf("\n");
}

static void print_header( const BLOCK_HEADER * b )
{
	return;
	debugf("Header:\n");
	debugf("\tMagic: %s\n", b -> magic);
	debugf("\tBlock size: %lu\n", b -> block_size);
	debugf("\tNodes: %lu\n", b -> nodes);
	debugf("\tBlocks: %lu\n", b -> blocks);
}

static void print_all_nodes(void)
{
	NODEMAP::iterator iv = _nodes.begin();
	return;	
	debugf("\nPrinting all nodes in map\n");
	while ( iv != _nodes.end() )
	{
		print_node(iv->second);
		++iv;
	}
}



//////////////////////////////////////////////////////////////////
//
// START HERE W/ fs_drive()
//
//////////////////////////////////////////////////////////////////
//Read the hard drive file specified by dname
//into memory. You may have to use globals to store
//the nodes and / or blocks.
//Return 0 if you read the hard drive successfully (good MAGIC, etc).
//If anything fails, return the proper error code (-EWHATEVER)
//Right now this returns -EIO, so you'll get an Input/Output error
//if you try to run this program without programming fs_drive.
//////////////////////////////////////////////////////////////////
int fs_drive(const char *dname)
{
	debugf("fs_drive: %s\n", dname);
	
	BLOCK_HEADER * header;
	unsigned int i = 0;
	uint64_t j = 0;
    
	// setup
	header = (BLOCK_HEADER*) malloc(sizeof(BLOCK_HEADER));
	ifstream infile(dname, fstream::binary);

	// read in the header
	infile.read((char*)header, sizeof(BLOCK_HEADER));

	// check the magic
	const string magic = header -> magic;
	if ( magic != "COSC_361" )
	{
		free(header);
		return -ENOENT;
	}

	// set the global header reference
	_header = header;

	debugf("header magic: %s\n", header -> magic);

	NODE * local_node = (NODE*) malloc(sizeof(NODE));
	
	print_header(_header);

	for ( i = 0 ; i < _header -> nodes ; ++i){
		
		// if its a new iteration, make sure we allocated space
		// for the node
		if (local_node == NULL)
		{
			local_node = (NODE*) malloc(sizeof(NODE));
		}

		// read in the file name
		infile.read( (char*) (&local_node->name), NAME_SIZE);
		
		// this will be used for the key in the node map =>  _nodes
		const string name = local_node -> name;

		// populate the node with the correct data
		infile.read((char*) (&local_node->id), sizeof(uint64_t) );
		infile.read( (char*) (&local_node->link_id), sizeof(uint64_t));
		infile.read( (char*) (&local_node -> mode), sizeof (uint64_t));
		infile.read( (char*) (&local_node -> ctime), sizeof(uint64_t));
		infile.read( (char*) (&local_node -> atime), sizeof(uint64_t));
		infile.read( (char*) (&local_node -> mtime), sizeof(uint64_t) );
		
		infile.read( (char*) (&local_node -> uid), sizeof(uint32_t) );
		infile.read( (char*) (&local_node -> gid), sizeof(uint32_t) );
		infile.read( (char*) (&local_node -> size), sizeof(uint64_t) );
		

		// if the node size is > 0 then read in the blocks
		//   else set node -> blocks = NULL
		if ( local_node -> size > 0 ){
			const uint num_blocks = (local_node -> size / _header -> block_size) + 1;

			debugf("%s has %d blocks\n", name.c_str(), num_blocks);

			// malloc the space for the block offsets
			local_node -> blocks = (uint64_t*) malloc( sizeof(uint64_t) * num_blocks);
		
			// read in the block offsets and store them in the node
			j = 0;
			while ( j < num_blocks)
			{
				infile.read( (char*) (local_node -> blocks + j), sizeof(uint64_t));
				++j;
			}
		} else
		{
			local_node -> blocks = NULL;
		}


		// add the node to the map
		pair<string, NODE*> data = pair<string, NODE *>(name, local_node);
		_nodes.insert( pair<string, NODE*>(name, local_node ));

		// set local node to be null in case we need to make a new one
		local_node = NULL;
	}

	print_all_nodes();

	// set the _next_block_id
	_next_block_id = (int) header -> blocks;

	// read in the blocks
	j = 0;
	for ( j = 0 ; j < header -> blocks ; ++j )
	{
		BLOCK * block = (BLOCK*) malloc(sizeof(BLOCK));
		block -> data = (char*) malloc( _header -> block_size );

		debugf("Reading block: %d\n", j);
		infile.read(block->data, _header->block_size);
	
		debugf("data for %d\n%s\n", j, block->data);

		pair<int64_t, BLOCK*> data = pair<int64_t, BLOCK*>(j, block);
		_blocks.insert(data);
	}
	infile.close();
	printf("fs_drive complete\n");
	return 0;
}

//////////////////////////////////////////////////////////////////
//Open a file <path>. This really doesn't have to do anything
//except see if the file exists. If the file does exist, return 0,
//otherwise return -ENOENT
//////////////////////////////////////////////////////////////////
// this is DONE
int fs_open(const char *path, struct fuse_file_info *fi)
{
	debugf("fs_open: %s\n", path);
	NODEMAP::iterator iv = _nodes.find(path);

	if ( iv != _nodes.end() )
	{
		return 0;
	}

	return -ENOENT;
}

//////////////////////////////////////////////////////////////////
//Read a file <path>. You will be reading from the block and
//writing into <buf>, this buffer has a size of <size>. You will
//need to start the reading at the offset given by <offset>.
//////////////////////////////////////////////////////////////////
int fs_read(const char *path, char *buf, size_t size, off_t offset,
	    struct fuse_file_info *fi)
{
	debugf("fs_read: %s\n", path);
	
	debugf("\tsize: %d\n",(int) size);
	debugf("\toffset: %d\n",(int) offset);
	debugf("block_size: %d\n", _header -> block_size);


	NODEMAP::const_iterator iv = _nodes.find(path);
	const NODE * node = iv -> second;
	const uint num_blocks = node -> size > 0 ?  node ->size / _header -> block_size + 1 : 0;
	BLOCK * blocks[num_blocks];

	uint i = 0;
	
	debugf("has %d blocks\n", num_blocks);

	BLOCKMAP::iterator bv = _blocks.end();

	debugf("\nthe node\n");
	print_node(iv->second);
	debugf("\n");

	// get the blocks for this path
	// and store their pointers into an array in order
	debugf("\tBlocks:\n\t");
	while (i < num_blocks)
	{
		bv = _blocks.end();
		const uint block_num = node ->blocks[i];
		debugf("\ti = %d block = %d, ", i, block_num);
		bv = _blocks.find(block_num);
		if ( bv == _blocks.end() )
		{
			return -EIO;
		}
		debugf("\nGot block %d, storing at blocks[%d]\n", block_num,  i);
		blocks[i] = bv ->second;
//		debugf("data:\n%s\n", blocks[i]->data);
		++i;
	}
	
	debugf("\n");
	
	// find which block to use and where to start the copy
	const uint block_wanted = offset / _header->block_size;
	const uint new_offset = offset % _header->block_size;

	//debugf("Data is at block index: %d with offset: %d\n", block_wanted, new_offset);
	
	// if new_offset != 0 => may need to read across blocks
	
	// if the size is 0 do nothing
	if ( node -> size == 0 )
	{
		debugf("\tThis file has no data\n");
		return 0;
	}

	// make sure we're tryinbg to start the read at a valid block
	if ( block_wanted >= num_blocks )
	{
		debugf("invalid block number wanted\n");
		return -EIO;
	}

	// this is not an error
	//if ( new_offset >= _header->block_size )
	//{
	//	debugf("new_offset is >= block_size\n");
	//	return -EIO;
	//}

	
	// make a buffer that will hold all of the data from the starting block to
	// the end of the file
	// this is not efficient, but its very simple
	char * data = (char*) malloc(sizeof(char) * ((num_blocks - block_wanted)) * _header -> block_size);

	uint j = 0;

	// for each block
	//	copy that data into our buffer
	//	this will make all blocks be in a sequential char * buffer
	//	that we can read from below
	for ( j = 0 ; j < num_blocks - block_wanted ; ++j )
	{
		uint k = 0;
		for ( k = 0 ; k < _header -> block_size ; ++k )
		{
			uint block_offset = j * _header -> block_size;
			data[ block_offset + k] = blocks[block_wanted+j] -> data[k];
		}
		//data[j * _header -> block_size]
	}

//	debugf("Data in our buffer:\n%s", data);

	//set the src for the copy below
	const char * src = (char*) (data) + new_offset;
	uint bytes_read = 0;

	// see the max number of readable bytes
	//   this is the start pos of the start block all the way to the end of the 
	//   known blocks for this file
	unsigned int data_length = (num_blocks - block_wanted) * _header -> block_size;
	unsigned int readable_bytes = (data_length) - offset;

	debugf("\n\nSTATS\n\tfile size: %d\n\t\n\tdata length: %d\n\treadable bytes: %d\n\trequested read size: %d\n",
			node -> size, data_length,  readable_bytes, size);

	// check the number of readable bytes
	//	if size > readable_bytes
	//	then set size = readable_bytes
	if ( size > readable_bytes )
	{
		debugf("Trying to read more bytes than the file has\n");
		debugf("\tReadable bytes: %d\n\tSize: %d\n\tnum blocks: %d\n\tblock wanted: %d\n",
				readable_bytes, size, num_blocks, block_wanted);
		debugf("block size: %d\n", _header -> block_size);
		debugf("%d\n%d\n%d\n", num_blocks-block_wanted, (num_blocks-block_wanted) * _header-> block_size,
				((num_blocks-block_wanted)*_header->block_size)-offset);
		debugf("truncating size down from %d to %d\n", size, readable_bytes);
		size = readable_bytes;
	}

	if ( size > node -> size )
	{
		debugf("\tsize > node -> size, truncating from %d to %d\n",
				size, node -> size);
		size = node -> size;
	}



	debugf("\t copying %d bytes into os buffer\n", size);

	//copy the data from our buffer into the OS provided buffer
	for ( j = 0 ; j < size ; ++j )
	{
		buf[j] = src[j];
		++bytes_read;
	}

	time_t curr_time = time(NULL);
	iv -> second -> atime = curr_time;

	debugf("bytes read: %d\n", bytes_read);
	return bytes_read;
}

//////////////////////////////////////////////////////////////////
//Write a file <path>. If the file doesn't exist, it is first
//created with fs_create. You need to write the data given by
//<data> and size <size> into this file block. You will also need
//to write data starting at <offset> in your file. If there is not
//enough space, return -ENOSPC. Finally, if we're a read only file
//system (fi->flags & O_RDONLY), then return -EROFS
//If all works, return the number of bytes written.
//////////////////////////////////////////////////////////////////
int fs_write(const char *path, const char *data, size_t size, off_t offset,
	     struct fuse_file_info *fi)
{
	debugf("fs_write: %s\n", path);

	debugf("\tsize: %d\n\toffset: %d\n\n", size, offset);

	if ((fi->flags & O_RDONLY)) {
		debugf ("fs_write: File %s is read only.\n", path);
		return -EROFS;
	}

	NODEMAP::const_iterator iv = _nodes.find (path);

	if (iv == _nodes.end ()) {
		debugf ("fs_write: File %s doesn't exist. Creating new file.\n", path);
		return -ENOENT;
	}

	NODE *node = iv->second;

	if (node->mode & S_IFDIR) {
		debugf ("fs_write: %s is a directory.\n", path);
		return -EISDIR;
	}

	
	uint num_blocks = (node->size > 0) ? (node->size / _header->block_size + 1) : 0;

	debugf("\tnum blocks = %d\n", num_blocks);

	uint num_blocks_needed = ((size) / _header->block_size) + 1;

	debugf("\tblocks needed: %d\n", num_blocks_needed);

	uint start_block = (offset / _header->block_size);
	uint offset_in_block = (offset % _header->block_size);

	debugf("\tstart block: %d\n\toffset in start: %d\n", start_block, offset_in_block);

	uint i = 0;

	const uint total_blocks_needed = ((size + offset) / _header -> block_size) + 1;
	const uint have_blocks = node -> size == 0 ?  0 : ( node -> size / _header -> block_size) + 1;
	

	
	if ( total_blocks_needed > have_blocks )
	{
		debugf("\t need %d have %d\n", total_blocks_needed , have_blocks);
		
		if ( have_blocks == 0 ) {
			node -> blocks = (uint64_t*) malloc(total_blocks_needed * sizeof(uint64_t));
		} else {
			node -> blocks = (uint64_t*) realloc(node -> blocks , total_blocks_needed * sizeof(uint64_t));
		}

		for ( i = have_blocks ; i < total_blocks_needed ; ++i )
		{
			BLOCK * block = (BLOCK*) malloc(sizeof(BLOCK));
			block -> data = (char*) malloc(_header -> block_size);
			_blocks.insert(pair<uint64_t, BLOCK*>(_next_block_id, block));
			node -> blocks[i] = _next_block_id;
			debugf("\t added block with id %d at node -> blocks[%d]\n", _next_block_id, i);
			++_next_block_id;
			_header -> blocks += 1;
		}

	}

	/*


	if (num_blocks_needed > need_after_start) {
		//make the block index array bigger
		node->blocks = (uint64_t *) realloc (node->blocks, (num_blocks_needed * sizeof (uint64_t)));
		
		for (i = num_blocks; i < num_blocks_needed; i ++) {
			BLOCK *block = (BLOCK *) malloc (sizeof (BLOCK));
			block->data = (char *) malloc (sizeof (char) * _header->block_size);
			_blocks.insert (std::pair <int, BLOCK *> (_next_block_id, block));
			node->blocks [i] = _next_block_id;
			debugf("\tadded block with id: %d at node->block[%d]\n", _next_block_id, i);
			++_next_block_id;
			//inc the total number of blocks
			_header -> blocks += 1;
		}
	}
*/

	const unsigned int old_bytes = node -> size - offset;
	const unsigned int new_size = old_bytes + size;

	node -> size = offset + size;

//	return size;

	

	debugf("\nafter block add\n");
	print_node(node);

	i = start_block; //count for blocks
	uint j = offset_in_block; //count for block data
	uint k = 0; //count for write data
	uint block_id = 0;
//	debugf("\twriting data to node->block[%d]\n", i);

	debugf ("\t j = %d\n", j);
	debugf("\nstarting write\n\n");
	while (i < num_blocks_needed, k < size) {
		block_id = node->blocks [i];
		BLOCKMAP::iterator bv = _blocks.find(block_id);
	
		debugf("\twriting node -> blocks[%d] with id %d\n|", i, block_id);

		if ( bv == _blocks.end() )
		{
			debugf("ERROR! cant find block with id = %d\n", block_id);
			return -EIO;
		}

		BLOCK * block = bv -> second;

		while (j < _header->block_size && k < size) {
			block -> data[j] = data[k];
			++k;
			++j;
		}
		debugf("\twrote %d bytes  j = %d\n", k, j);
		//debugf("\tDATA\n %s", block -> data);
		j = 0; //offset doesn't matter after first block
		i ++;
	}

	time_t curr_time = time(NULL);
	node -> atime = curr_time;
	node -> mtime = curr_time;


	debugf("\nWROTE k = %d  NEEDED = %d\n", k, size);

	return k;
}

//////////////////////////////////////////////////////////////////
//Create a file <path>. Create a new file and give it the mode
//given by <mode> OR'd with S_IFREG (regular file). If the name
//given by <path> is too long, return -ENAMETOOLONG. As with
//fs_write, if we're a read only file system
//(fi->flags & O_RDONLY), then return -EROFS.
//Otherwise, return 0 if all succeeds.
//////////////////////////////////////////////////////////////////
int fs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	debugf("fs_create: %s\n", path);
	NODE * node;
	node = (NODE*) malloc( sizeof(NODE) );
	uint i = 0;
	string name = path;

	for ( i = 0 ; i < name.length() ; ++i)
	{
		node -> name[i] = name[i];
	}

	if ( fi -> flags & O_RDONLY ) {
		return -EROFS;
	}

	if ( name.length() > NAME_SIZE )
	{
		return -ENAMETOOLONG;
	}

	node -> mode = (mode | S_IFREG);
	time_t current_time = time (NULL);
	node -> atime = current_time;
	node -> mtime = current_time;
	node -> ctime = current_time;
	node -> size = 0;

	debugf("adding node\n");
	print_node(node);
	_header -> nodes += 1;
	_nodes.insert( pair<string, NODE*>(name, node) );
	
	return 0;
}

//////////////////////////////////////////////////////////////////
//Get the attributes of file <path>. A static structure is passed
//to <s>, so you just have to fill the individual elements of s:
//s->st_mode = node->mode
//s->st_atime = node->atime
//s->st_uid = node->uid
//s->st_gid = node->gid
// ...
//Most of the names match 1-to-1, except the stat structure
//prefixes all fields with an st_*
//Please see stat for more information on the structure. Not
//all fields will be filled by your filesystem.
//////////////////////////////////////////////////////////////////
// this is DONE
int fs_getattr(const char *path, struct stat *s)
{
	debugf("fs_getattr: %s\n", path);

	NODEMAP::iterator iv = _nodes.find(path);

	if ( iv == _nodes.end() )
	{
		return -ENOENT;
	}

	// get the number of blocks this node has
	unsigned int num_blocks = (iv -> second -> size /  _header -> block_size) + 1;
	
	// this is for displaying the stats
	//   its 512 * number of blocks
	const unsigned int display_block_size = 512;
	
	const float multiplier =  _header -> block_size / display_block_size;

	// check if its a directory
	const bool is_dir = (iv -> second -> mode & S_IFDIR) == S_IFDIR;

	unsigned int file_size = iv -> second -> size;

	if ( file_size == 0 ) num_blocks = 0;

	debugf("\t\nnum_blocks: %d\n\tdisp size: %d\n\t miltiplier: %f\n",
			num_blocks, display_block_size, multiplier);
	debugf("\tfile size: %d\n", file_size);

	NODE * node = iv -> second;
	s -> st_mode = node -> mode;
	s -> st_ino = node -> id;
	//s -> st_nlink
	s -> st_uid = node -> uid;
	s -> st_gid = node -> gid;
	// if its a dir then the size should be 0
	s -> st_size = is_dir ?  0 : file_size; // this part may be wrong
	s -> st_atime = node -> atime;
	s -> st_mtime = node -> mtime;
	s -> st_ctime = node -> ctime;
	s -> st_blksize = _header -> block_size;
	s -> st_blocks = node -> size / _header -> block_size + 1;
	debugf("Returning 0 for %s\n", path);
	return 0;
}

//////////////////////////////////////////////////////////////////
//Read a directory <path>. This uses the function <filler> to
//write what directories and/or files are presented during an ls
//(list files).
//
//filler(buf, "somefile", 0, 0);
//
//You will see somefile when you do an ls
//(assuming it passes fs_getattr)
//////////////////////////////////////////////////////////////////
int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
	       off_t offset, struct fuse_file_info *fi)
{
	debugf("fs_readdir: %s\n", path);

	//filler(buf, <name of file/directory>, 0, 0)
	filler(buf, ".", 0, 0);
	filler(buf, "..", 0, 0);

	NODEMAP::iterator iv = _nodes.find(path);
	if ( iv == _nodes.end() )
	{
		return -ENOENT;
	}

	// iv points to the "root" node in the subtree that is the directory
	const string root_path = iv -> second -> name;
	const size_t root_path_len = root_path.length();
	debugf("Root path: %s  length: %d\n", root_path.c_str(), root_path_len);
	
	// get to the fist contained item
	++iv;
	
	// iter through the values in the subtree based on root
	while ( iv != _nodes.end() )
	{
		
		string local_path = iv -> second -> name;
		debugf("local path: %s\n", local_path.c_str());
		//std::cout << "cout local " << local_path << endl;
	
		size_t contains_root = local_path.find(root_path);

		// make sure the local path is under the root path
		if ( contains_root == string::npos ) 
		{
			++iv;
			debugf("%s not under root path\n", local_path.c_str());
			continue;
		}

		
		local_path.erase(0, root_path_len);
		debugf("after erase: %s\n", local_path.c_str());
		
		size_t has_slash = local_path.rfind("/");

		// if no slash is found or only a leading slash is found
		if ( has_slash == string::npos) 
		{
			debugf("should add path: %s\n", local_path.c_str());
			filler(buf, local_path.c_str(), 0, 0);
		}
		
		// if its a sub dir
		if ( has_slash == 0 )
		{	
			debugf("its a subdir: %s\n", local_path.c_str());
			local_path.erase(0,1);
			filler(buf, local_path.c_str(), 0, 0);
		}

		++iv;
		
	}
	//You MUST make sure that there is no front slashes in the name (second parameter to filler)
	//Otherwise, this will FAIL.

	return 0;
}

//////////////////////////////////////////////////////////////////
//Open a directory <path>. This is analagous to fs_open in that
//it just checks to see if the directory exists. If it does,
//return 0, otherwise return -ENOENT
//////////////////////////////////////////////////////////////////
// this is DONE
int fs_opendir(const char *path, struct fuse_file_info *fi)
{
	debugf("fs_opendir: %s\n", path);
	NODEMAP::iterator iv = _nodes.find(path);
	if ( iv != _nodes.end() )
	{
		return 0;
	}

	return -ENOENT;
}

//////////////////////////////////////////////////////////////////
//Change the mode (permissions) of <path> to <mode>
//////////////////////////////////////////////////////////////////
// this is DONE
int fs_chmod(const char *path, mode_t mode)
{
	debugf("fs_chmod: %s %d\n", path, (int) mode);
	NODEMAP::iterator iv = _nodes.find(path);
	
	// if not found
	if ( iv == _nodes.end() )
	{
		return -ENOENT;
	}
	
	// set the new mode
	NODE * node = iv -> second;
	node -> mode = mode;

	return 0;
}

//////////////////////////////////////////////////////////////////
//Change the ownership of <path> to user id <uid> and group id <gid>
//////////////////////////////////////////////////////////////////
// this is DONE
int fs_chown(const char *path, uid_t uid, gid_t gid)
{
	debugf("fs_chown: %s\n", path);
	
	NODEMAP::iterator iv = _nodes.find(path);

	// if not exist
	if ( iv == _nodes.end() )
	{
		return -ENOENT;
	}

	NODE * node = iv -> second;
	node -> uid = uid;
	node -> gid = gid;
	return 0;
}

//////////////////////////////////////////////////////////////////
//Unlink a file <path>. This function should return -EISDIR if a
//directory is given to <path> (do not unlink directories).
//Furthermore, you will not need to check O_RDONLY as this will
//be handled by the operating system.
//Otherwise, delete the file <path> and return 0.
//////////////////////////////////////////////////////////////////
// this is DONE
int fs_unlink(const char *p)
{
	debugf("fs_unlink: %s\n", p);
	
	const string path = p; // since strings are nicer
	NODEMAP::iterator iv; // pts to the node in the map
	
	// make sure its not a dir
	const char last_letter = path[ path.length() - 1];
	if ( last_letter == '\\' || last_letter == '/')
	{
		debugf("Invalid: cannot unlink a directory: %s\n", p);
		return -EISDIR;
	}
	// else its just a file
	iv = _nodes.find(p);

	// make sure it exists
	if ( iv == _nodes.end() )
	{
		debugf("\ttrying to unlink a file that doesnt exist\n");
		return 0;
		//return -ENOENT;
	}
	
	uint num_blocks = iv -> second -> size == 0 ? 0 : iv -> second -> size / _header -> block_size + 1;

	for ( uint i = 0 ; i < num_blocks ; ++i )
	{
		uint id = iv -> second -> blocks[i];
		debugf("\tremoving block: %d\n", id);
		BLOCKMAP::iterator bv = _blocks.find(id);
		if ( bv == _blocks.end() )
		{
			debugf("\tERROR: trying to remove a non existant block: id = %d\n", id);
			debugf("\tblockmap size: %d  _next_block_id = %d\n", _blocks.size(), _next_block_id);
			return -EIO;
		}
		_blocks.erase(bv);
		_header -> blocks -= 1;
	}

	_header -> nodes -= 1;
	_nodes.erase(iv);

	return 0;
}

//////////////////////////////////////////////////////////////////
//Make a directory <path> with the given permissions <mode>. If
//the directory already exists, return -EEXIST. If this function
//succeeds, return 0.
//////////////////////////////////////////////////////////////////
int fs_mkdir(const char *path, mode_t mode)
{
	debugf("fs_mkdir: %s\n", path);
	
	NODEMAP::iterator iv = _nodes.find(path);

	// fail if already exists
	if ( iv != _nodes.end() ) 
	{
		return -EEXIST;
	}

	NODE * node = (NODE*) malloc(sizeof(NODE));

	const string name = path;

	uint i = 0;
	
	// copy the name in
	for ( i = 0 ; i < name.length(); ++i )
	{
		(node -> name)[i] = name[i];
	}
	
	// set the \0 ending 
	(node -> name)[name.length()] = '\0';

	// set the mode
	node -> mode = mode | S_IFDIR;

	time_t curr_time = time(NULL);
	node -> atime = curr_time;
	node -> mtime = curr_time;
	node -> ctime = curr_time;
	
	node -> size = 0;

	// make the pair and insert
	pair<string, NODE*> data = pair<string, NODE *>(name, node);
	_nodes.insert( data );

	_header -> nodes += 1;


	iv = _nodes.find(path);
	if ( iv == _nodes.end())
	{
		debugf("Error inserting a new directory node\n");
	}

	return 0;
}

//////////////////////////////////////////////////////////////////
//Remove a directory. You have to check to see if it is
//empty first. If it isn't, return -ENOTEMPTY, otherwise
//remove the directory and return 0.
//////////////////////////////////////////////////////////////////
int fs_rmdir(const char *path)
{
	debugf("fs_rmdir: %s\n", path);

	NODEMAP::iterator iv = _nodes.find(path);

	if ( iv == _nodes.end())
	{
		return -ENOENT;
	}

	NODE * node = iv -> second;


	unsigned int children = 0;
	const string root_path = node -> name;
	iv++;

	while ( iv != _nodes.end() ) 
	{
		string local_path = iv -> second -> name;
		size_t contains_root = local_path.find(root_path);
		if ( contains_root != string::npos )
		{
			debugf("\t%s contains %s\n", root_path.c_str(), local_path.c_str());
			children++;
		}
		iv++;
	}
	
	// if have kids, return error
	if ( children != 0 )
	{
		return -ENOTEMPTY;
	}

	_header -> nodes -= 1;
	_nodes.erase(_nodes.find(path));

	//return 0, its empty
	return 0;
}

//////////////////////////////////////////////////////////////////
//Rename the file given by <path> to <new_name>
//Both <path> and <new_name> contain the full path. If
//the new_name's path doesn't exist return -ENOENT. If
//you were able to rename the node, then return 0.
//////////////////////////////////////////////////////////////////
// this is DONE
int fs_rename(const char *path, const char *new_name)
{
	debugf("fs_rename: %s -> %s\n", path, new_name);

	NODEMAP::iterator iv = _nodes.find(path);
	
	if ( iv == _nodes.end())
	{
		return -ENOENT;
	}

	if (_nodes.find(new_name) == _nodes.end() )
	{
		debugf("\tnew path does not exist");
		//return -ENOENT;
	}
/*
	const string parent = path;
	const string new_parent = new_name;
	//vector<string> kids;
	
	NODEMAP::iterator itor = _nodes.begin();

	while ( iv != _nodes.end() )
	{
		string local_path = itor -> second -> name;
		size_t contains_parent = local_path.find(parent);
		if ( contains_parent != string::npos )
		{
			local_path.erase(contains_parent, parent.length());
			local_path = new_parent + "/" + local_path;
			for (unsigned  int i = 0 ; i < local_path.length() ; ++i )
			{
				itor -> second -> name[i] = local_path[i];
			}
			itor -> second -> name[local_path.length()] = '\0';
		}
		iv++;
	}
*/


	
	NODE * node = iv -> second;
	const string name = new_name;
	
	uint i = 0;

	// copy the name in
	for ( i = 0 ; i < name.length(); ++i )
	{
		(node -> name)[i] = name[i];
    }
    // set the \0 ending
	(node -> name)[name.length()] = '\0';
	
	// delete the old tree copy since you cant rename keys
	_nodes.erase(iv);

    // make the pair and insert
	pair<string, NODE*> data = pair<string, NODE *>(name, node);
	_nodes.insert( data );

	return 0;
	
}

//////////////////////////////////////////////////////////////////
//Rename the file given by <path> to <new_name>
//Both <path> and <new_name> contain the full path. If
//the new_name's path doesn't exist return -ENOENT. If
//you were able to rename the node, then return 0.
//////////////////////////////////////////////////////////////////
int fs_truncate(const char *path, off_t size)
{
	debugf("fs_truncate: %s to size %d\n", path, size);
	
	NODEMAP::iterator iv = _nodes.find(path);
	const string name = path;
	NODE* node = NULL;
	
	if ( iv == _nodes.end() ) {
		return -ENOENT;
	}

	node = iv -> second;

	//cant truncate it to be bigger than it is
	if ( (uint) size > node -> size ) {
		debugf("fs_truncate: size is bigger than node -> size: %d %d\n", 
				size, node -> size);
		return -EIO;
	}

	const unsigned int num_to_keep = (size / _header -> block_size) + 1;
	const unsigned int num_blocks = node -> size == 0 ? 0 : (node -> size / _header -> block_size) + 1;
	const unsigned int block_offset = size % _header -> block_size;
	

	if ( node -> size == 0 )
	{
		debugf("\tnode -> size is 0, not truncating\n");
		return 0;
	}

	if ( num_to_keep > num_blocks )
	{
		debugf("fs_truncate: cannot keep more blocks than we have: %d  %d\n",
				num_to_keep, num_blocks);
		return -EIO;
	}

	uint64_t  block_ids[num_blocks];

	unsigned int i = 0, j = 0;

	debugf("\tfetching the block ids\n");
	for ( j = 0 ; j < num_blocks ; ++j )
	{
		debugf("\tj = %d  id = %d\n", j, block_ids[j]);
		block_ids[j] = node -> blocks[j];
	}


	debugf("\tkeeping: %d\n\thave: %d\n\t", num_to_keep, num_blocks);

	// realloc the space for the smaller number of nodes
	node -> blocks = (uint64_t*) realloc(node -> blocks, num_to_keep * sizeof(uint64_t));
	
	debugf("\tcopying the remaining block ids back into the node\n");
	for ( i = 0 ; i < num_to_keep ; ++i )
	{
		node -> blocks[i] = block_ids[i];
	}

	const uint blocks_removed = num_blocks - num_to_keep;
	_header -> blocks -= blocks_removed;

	BLOCKMAP::iterator bv;
	
	for ( i = num_to_keep ; i < num_blocks ; ++i )
	{
		bv = _blocks.find(block_ids[i]);
		debugf("\tremoving block with id : %d\n", block_ids[i]);
		if ( bv != _blocks.end() )
		{
			_blocks.erase(bv);
		}
	}

	if ( size == 0 )
	{
		debugf("\tpassed size was zero\n");
		node -> size = 0;
		return 0;
	}

	const unsigned int size_to_fill = _header -> block_size - block_offset;
	bv = _blocks.find(node -> blocks[i]);

	//set the new node size
	node -> size = size;
	
	return 0;
}


static void dump_node(ofstream & out, NODEMAP::iterator iv)
{
	NODE * node = iv -> second;
	debugf("dump node: %s\n", node -> name);
	debugf("\nwriting name\n");
	out.write((char*)node->name, NAME_SIZE);
	
	uint64_t buf[7];
	buf[0] = node -> id;
	buf[1] = node -> link_id;
	buf[2] = node -> mode;
	buf[3] = node -> ctime;
	buf[4] = node -> atime;
	buf[5] = node -> mtime;
	buf[6] = node -> size;

	uint32_t buf2[2];
	buf2[0] = node -> uid;
	buf2[1] = node -> gid;

	debugf("\twriting buf\n");
	out.write((char*)buf, sizeof(uint64_t) * 6);
	debugf("\twrigint buf2\n");
	out.write((char*)buf2, sizeof(uint32_t) * 2);
	debugf("\twriting size\n");
	debugf("\tsize: %d\n", buf[6]);
	out.write((char*) (buf+6), sizeof(uint64_t));
}



//////////////////////////////////////////////////////////////////
//fs_destroy is called when the mountpoint is unmounted
//this should save the hard drive back into <filename>
//////////////////////////////////////////////////////////////////
void fs_destroy(void *ptr)
{
	const char *filename = (const char *)ptr;
	debugf("fs_destroy: %s\n", filename);

	ofstream out(filename, ofstream::binary);

	debugf("\tThe header\n");
	print_header(_header);

	out.write((char*)_header, sizeof(BLOCK_HEADER));

	map<uint64_t, uint64_t> name_map;

	BLOCKMAP::iterator bv = _blocks.begin();
	uint64_t new_index = 0;

	while ( bv != _blocks.end() )
	{
		pair<uint64_t, uint64_t> data = pair<uint64_t, uint64_t>( bv -> first, new_index);
		name_map.insert( data );
		debugf("\tblock %d gets mapped to %d\n", bv -> first, new_index);
		bv++;
		++new_index;
	}


	NODEMAP::iterator iv = _nodes.begin();

	while( iv != _nodes.end() )
	{
		debugf("\tdumping node: %s\n", iv -> second -> name);
		dump_node(out, iv);
		NODE * node = iv -> second;
		uint num_blocks = (node -> size > 0) ?  node -> size / _header -> block_size + 1 : 0;
		debugf("\tstarting to write %d block offsets\n", num_blocks);
		for ( int i = 0 ; i < num_blocks ; ++i )
		{
			uint64_t new_i = name_map.find( node -> blocks[i] ) -> second;
			out.write((char*)&new_i, sizeof(uint64_t));
		}
		iv++;
	}

	
	bv = _blocks.begin();

	while ( bv != _blocks.end() )
	{
		out.write(bv -> second -> data, _header -> block_size);
		bv++;
	}

	out.close();

	//Save the internal data to the hard drive
	//specified by <filename>
}

//////////////////////////////////////////////////////////////////
//int main()
//DO NOT MODIFY THIS FUNCTION
//////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	fuse_operations *fops;
	char *evars[] = { "./fs", "-f", "mnt", NULL };
	int ret;

	if ((ret = fs_drive(HARD_DRIVE)) != 0) {
		debugf("Error reading hard drive: %s\n", strerror(-ret));
		return ret;
	}
	//FUSE operations
	fops = (struct fuse_operations *) calloc(1, sizeof(struct fuse_operations));
	fops->getattr = fs_getattr;
	fops->readdir = fs_readdir;
	fops->opendir = fs_opendir;
	fops->open = fs_open;
	fops->read = fs_read;
	fops->write = fs_write;
	fops->create = fs_create;
	fops->chmod = fs_chmod;
	fops->chown = fs_chown;
	fops->unlink = fs_unlink;
	fops->mkdir = fs_mkdir;
	fops->rmdir = fs_rmdir;
	fops->rename = fs_rename;
	fops->truncate = fs_truncate;
	fops->destroy = fs_destroy;

	debugf("Press CONTROL-C to quit\n\n");

	return fuse_main(sizeof(evars) / sizeof(evars[0]) - 1, evars, fops,
			 (void *)HARD_DRIVE);
}
