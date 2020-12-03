/**
 *NAME: Dhanush Nadella,Jeremy Tsujihara
 *ID: 205150583,505311626
 *EMAIL: dnadella787@gmail.com,jtsujihara@ucla.edu
 *
 * relavent research:
 * 
 * https://man7.org/linux/man-pages/man2/pread.2.html
 * https://www.tutorialspoint.com/c_standard_library/c_function_gmtime.htm
 *  
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "ext2_fs.h"
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>

//MACROS
#define SUPBLOCK_OFFSET 1024


//GLOBAL VARIABLES
struct ext2_super_block super_block;
struct ext2_group_desc group_descriptor;

__u32 bsize;


void inode_summary(int fd, int num_inode, int index);

//FUNCTIONS

void inode_summary(int fd, int num_inode, int index);

void print_error(char *error, int exit_code, int errno_flag){
    fprintf(stderr, "%s\n", error);
    if (errno_flag == 1){
        fprintf(stderr, "Error: %s\n", strerror(errno));
    }
    exit(exit_code);
}


void read_print_sb(int fd){
    if (pread(fd, &super_block, sizeof(struct ext2_super_block), SUPBLOCK_OFFSET) < 0)
        print_error("Error using pread on super block", 1, 1);

    if (super_block.s_magic != EXT2_SUPER_MAGIC)
        print_error("Superblock not read properly, magic # wrong", 1, 0);

    bsize = EXT2_MIN_BLOCK_SIZE << super_block.s_log_block_size;
    fprintf(stdout, "SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n",
            super_block.s_blocks_count,
            super_block.s_inodes_count,
            bsize,
            super_block.s_inode_size,
            super_block.s_blocks_per_group,
            super_block.s_inodes_per_group,
            super_block.s_first_ino);
}


void group_summary(int fd){
    if (pread(fd, &group_descriptor, sizeof(struct ext2_group_desc), SUPBLOCK_OFFSET + bsize) < 0)
        print_error("Error using pread on group descriptor", 1, 1);

    int group_num = 0; //--> I think its just 0 always since theres only one group, could be wrong
    fprintf(stdout, "GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n",
            group_num,
            super_block.s_blocks_count, 
            super_block.s_inodes_count,
            group_descriptor.bg_free_blocks_count,
            group_descriptor.bg_free_inodes_count,
            group_descriptor.bg_block_bitmap,
            group_descriptor.bg_inode_bitmap,
            group_descriptor.bg_inode_table);
}


/**
 * NOTES about free_block_summary and free_inode_summary:
 * a) Can't tell if the max number of blocks I should be counting is the
      s_blocks_per_group or s_blocks_count --> count includes reserved too
      so I assume it should s_blocks_per_group instead. Similar issue for 
      the free_inode_summary()
 * b) two if statements at end are for debugging, rmbr to remove later (for both)
*/
void free_block_summary(int fd){
    char *block_bitmap = (char *)malloc(bsize * sizeof(char));
    if (pread(fd, block_bitmap, bsize, (group_descriptor.bg_block_bitmap * bsize)) < 0)
        print_error("Error using pread to read block bitmap", 1, 1);


    int num_freeblock_count = 0;
    unsigned int num_blocks_processed = 0;
    __u32 num_freeblock = super_block.s_first_data_block;

    for (size_t i = 0; i < bsize; i++){
        for (size_t j = 0; j < (sizeof(char) * 8); j++){
            int curr_bit = (block_bitmap[i] >> j) & 0x01;

            if (curr_bit == 0){
                fprintf(stdout, "BFREE,%d\n",
                        num_freeblock);
                
                num_freeblock_count++;  //number of free blocks to compare later
            } 
            num_freeblock++;            //block ID that the current bit is refers too
            num_blocks_processed++;     //number of blocks scanned so far

            //if the number of blocks scanned is the number of blocks in group, stop scanning
            if (num_blocks_processed == super_block.s_blocks_per_group){
                i = bsize;              //to break out of both loops
                break;
            }
        }
    }
    free(block_bitmap);

    //debug 
    if (num_freeblock_count != group_descriptor.bg_free_blocks_count)
        print_error("Counted number of free blocks in group doesn't match group descriptor number", 1, 0);

    if (num_blocks_processed != super_block.s_blocks_per_group)
        print_error("Counted more blocks than blocks in group", 1, 0);
}


void free_inode_summary(int fd){
    char *inode_bitmap = (char *)malloc(bsize * sizeof(char));
    if (pread(fd, inode_bitmap, bsize, (group_descriptor.bg_inode_bitmap * bsize)) < 0)
        print_error("Error using pread to read inode bitmap", 1, 1);

    int num_freeinode_count = 0;
    unsigned int num_inodes_processed = 0;
    __u32 num_freeinode = 1;
    for (size_t i = 0; i < bsize; i++){
        for (size_t j = 0; j < (sizeof(char) * 8); j++){
            int curr_bit = (inode_bitmap[i] >> j) & 0x01;

            if (curr_bit == 0){
                fprintf(stdout, "IFREE,%d\n",
                        num_freeinode);

                num_freeinode_count++;     //number of free inodes to compare later
            }
            else{
                inode_summary(fd, num_freeinode, num_freeinode - 1);
            }
            num_freeinode++;               //number of free inode that the bit refers to
            num_inodes_processed++;        //total number of bits/inodes scanned
            
            //if the number if inodes scanned is equal to the inodes per group, stop 
            if (num_inodes_processed == super_block.s_inodes_per_group){
                i = bsize;
                break;
            }
        }
    }
    free(inode_bitmap);

    //debug 
    if (num_freeinode_count != group_descriptor.bg_free_inodes_count)
        print_error("Counted number of free blocks in group doesn't match group descriptor number", 1, 0);

    if (num_inodes_processed != super_block.s_inodes_per_group)
        print_error("Counted more blocks than blocks in group", 1, 0);
}


void read_print_dir_entries(int fd, int inode, int bnum){
  struct ext2_dir_entry dir_ent;
  long OFFSET = (bnum-1)*bsize + SUPBLOCK_OFFSET;

  unsigned int i = 0;
  while(i < bsize){
    memset(dir_ent.name, 0, 256); //maybe?
    pread(fd, &dir_ent, sizeof(dir_ent), OFFSET + i);
    if(dir_ent.inode){
      memset(&dir_ent.name[dir_ent.name_len], 0, 256 - dir_ent.name_len);
      fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,'%s'\n",
	      inode,
	      i,
	      dir_ent.inode,
	      dir_ent.rec_len,
	      dir_ent.name_len,
	      dir_ent.name);
    }
    i = i + dir_ent.rec_len;
  }
}


/**
fd - file descriptor
num_inode - number of the inode in decimal, this part calculated in the free_inode_summary
            and transferred to the inode_summary function
indirection_level - level of indirection (3,2,1)
logical_offset - the logical offset (this is the part I'm having trouble with)
index - the block number of the indirect block pointers

NOTES:
    a) I think we need some way to check if the indirect blocks are also directories
       to call the read_print_dir_entires function too? not sure how to do this
    b) not sure if my logical offsets are correct, I got them from the part "i_block"
       in ext2 documentation given
       i) also check back on the initial function calls in inode_summary
    c) im not sure im understanding the indexing for the blocks properly either
*/
void indirect_block_helper(int fd, int num_inode, int indirection_level, int logical_offset, int index, char ftype){
    long OFFSET = bsize * index;

    __u32 *indirect_block_indices = (__u32*)malloc(bsize);

    if (pread(fd, indirect_block_indices, bsize, OFFSET) <  0)
        print_error("Error using pread on indirect block", 1, 1);

    __u32 curr_indir_block_index;
    for (size_t i = 0; i < (bsize / sizeof(__u32)); i++){
        curr_indir_block_index = indirect_block_indices[i];
        if (curr_indir_block_index != 0){
            if ((ftype == 'd') && (indirection_level == 1))
                read_print_dir_entries(fd, num_inode, curr_indir_block_index);

            fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n",
                    num_inode,
                    indirection_level,
                    logical_offset,
                    index, 
                    curr_indir_block_index);
                
            if (indirection_level == 2)
                indirect_block_helper(fd, num_inode, 1, logical_offset, curr_indir_block_index, ftype);

            if (indirection_level == 3)
                indirect_block_helper(fd, num_inode, 2, logical_offset, curr_indir_block_index, ftype);
        }

        logical_offset += pow(256, indirection_level - 1);
    }
    free(indirect_block_indices);
}


char* time_func(time_t time){
  time_t rawtime = time;
  struct tm* info;
  info = gmtime(&rawtime);
  char* ret = malloc(sizeof(char)*32);
  strftime(ret, 32, "%m/%d/%y %H:%M:%S", info);
  return ret;
}


/**
 * NOTES about inode_summary and how its called in free_inode_summary:
   a) I think the offset for the pread should be the comment because the reserved
      inodes aren't in the table and you want to read sizeof(inode) bytes for each 
      inode. The inode table is located at (bsize * bg_inode_table) and then we 
      just index.
   b) the index value is the (inode number in free_inode_summary) - 1. (-1) because the
      starting inode number referred to in the bitmap is 1. So we subtract 1 from it
      since the first inode will be at exactly (bg_inode_table * bsize)
*/
void inode_summary(int fd, int num_inode, int index){
  struct ext2_inode inode;
  long OFFSET = (group_descriptor.bg_inode_table * bsize) + (index * sizeof(inode));
  pread(fd, &inode, sizeof(inode), OFFSET);

  if(inode.i_links_count != 0 && inode.i_mode != 0){
    char ftype = '?';

    if((inode.i_mode & 0xF000) == 0x8000){
      ftype = 'f';
    }
    else if((inode.i_mode & 0xF000) == 0x4000){
      ftype = 'd';
    }
    else if((inode.i_mode & 0xF000) == 0xA000){
      ftype = 's';
    }

    char* ctime = time_func(inode.i_ctime);
    char* mtime =	time_func(inode.i_mtime);
    char*	atime = time_func(inode.i_atime); 

    fprintf(stdout, "INODE,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d",
	  num_inode,
	  ftype,
	  inode.i_mode & 0xFFF,
	  inode.i_uid,
	  inode.i_gid,
	  inode.i_links_count,
	  ctime,
	  mtime,
	  atime,
	  inode.i_size,
	  inode.i_blocks); //maybe i_blocks?

    if(ftype == 's' && inode.i_size <= 60){
      fprintf(stdout,",%u", inode.i_block[0]);
    }
    else{
      for(int i = 0; i < 15; i++){
	fprintf(stdout,",%u", inode.i_block[i]);
      }
    }

    fprintf(stdout,"\n");

    for(int i = 0; i<12; i++){
      if(inode.i_block[i] != 0 && ftype == 'd'){
        read_print_dir_entries(fd, num_inode, inode.i_block[i]);
      }
    }

    if(inode.i_block[12] != 0){
      //indirect level 1
      indirect_block_helper(fd, num_inode, 1, 12, inode.i_block[12], ftype);
    }
    if(inode.i_block[13] != 0){
      //indirect level 2
      indirect_block_helper(fd, num_inode, 2, (256 + 12), inode.i_block[13], ftype);
    }
    if(inode.i_block[14] != 0){
      //indirect level 3
      indirect_block_helper(fd, num_inode, 3, (256 * 256 + 256 + 12), inode.i_block[14], ftype);
    }
  }
}
  



int main(int argc, char** argv){
    if (argc != 2){
        print_error("Incorrect usage. Correct usage: ./lab3a FILESYSIMG", 1, 0);
    }

    char *filesystem = argv[1];

    int filesystem_fd = open(filesystem, O_RDONLY);
    if (filesystem_fd < 0)
        print_error("Error opening filesystem image.", 1, 1);


    read_print_sb(filesystem_fd);
    group_summary(filesystem_fd);

    free_block_summary(filesystem_fd);
    free_inode_summary(filesystem_fd);

    exit(0);
}
