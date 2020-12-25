#NAME: Dhanush Nadella,Jeremy Tsujihara
#ID: 205150583,505311626
#EMAIL: dnadella787@gmail.com,jtsujihara@ucla.edu

#research:
#https://realpython.com/python-csv/


import sys
import csv

#global
super_block = None
group = None
inodes = []
dirents = []
indirects = []
free_blocks = []
free_Inodes = []


isconsistent = 0 #for every inconsistency add 1 to this value

#classes
class Superblock:
    def __init__(self,e):
        self.s_blocks_count = int(e[1])     #total number of blocks
        self.s_inodes_count = int(e[2])     #total number of inodes
        self.bsize = int(e[3])              #block size
        self.s_inode_size = int(e[4])       #size of inode structure
        self.s_blocks_per_group = int(e[5]) #blocks per group
        self.s_inodes_per_group = int(e[6]) #inodes per group
        self.s_first_ino = int(e[7])        #first non reserved inode

class Group:
    def __init__(self,e):
        self.group_num = int(e[1])
        self.blocks_count = int(e[2])
        self.inodes_count = int(e[3])
        self.bfree_count = int(e[4])
        self.ifree_count = int(e[5])
        self.block_bitmap = int(e[6])
        self.inode_bitmap = int(e[7])
        self.inode_table = int(e[8])


class Inode:
    def __init__(self,e):
        self.num_inode = int(e[1])
        self.ftype = (e[2])
        self.mode = int(e[3])
        self.i_uid = int(e[4])
        self.i_gid = int(e[5])
        self.i_links_count = int(e[6])
        self.ctime = e[7]
        self.mtime = e[8]
        self.atime = e[9]
        self.i_size = int(e[10])
        self.i_blocks = int(e[11])
        self.dirblocks_list = [int(x) for x in e[12:24]] if e[2] != 's' else []
        self.indirblocks_list = [int(x) for x in e[24:27]] if e[2] != 's' else []

class Directory:
    def __init__(self,e):
        self.num_inode = int(e[1])
        self.offset = int(e[2])
        self.inode = int(e[3])
        self.rec_len = int(e[4])
        self.name_len = int(e[5])
        self.name = e[6]

class Indirect:
    def __init__(self,e):
        self.num_inode = int(e[1])
        self.level = int(e[2])
        self.offset = int(e[3])
        self.index = int(e[4])
        self.curr_indir_block_index = int(e[5])

#functions

#Block Consistency Audits
def Block_Consistency_Audit():
    global isconsistent
    max_block_num = super_block.s_blocks_count 
    first_legal_block_num = group.inode_table + int((group.inodes_count * super_block.s_inode_size)/super_block.bsize)

    alloc_block_to_inode = dict() #a dictionary that maps free blocks to an array with pertinent array info
    duplicate_blocks = set()      #a set with all duplicate blocks

    #first check direct and indirect blocks of each inode
    for inode in inodes:
        logical_offset = 0

        if len(inode.dirblocks_list) != 0:
            for dirblock in inode.dirblocks_list:
                if dirblock == 0:
                    continue

                if dirblock < 0 or dirblock >= max_block_num: 
                    isconsistent += 1
                    print("INVALID BLOCK {0} IN INODE {1} AT OFFSET {2}".format(dirblock, inode.num_inode, logical_offset))
                elif dirblock > 0 and dirblock < first_legal_block_num:
                    isconsistent += 1
                    print("RESERVED BLOCK {0} IN INODE {1} AT OFFSET {2}".format(dirblock, inode.num_inode, logical_offset))

                alloc_block_to_inode.setdefault(dirblock,[]).append([inode.num_inode, 0, logical_offset])

                logical_offset += 1

        if len(inode.indirblocks_list) != 0:
            for i in range(0, 3):
                blocktype = ""
                indir_offset = 0
                if i == 0:
                    blocktype = "INDIRECT"
                    indir_offset = 12
                elif i == 1:
                    blocktype = "DOUBLE INDIRECT"
                    indir_offset = 268 
                else:
                    blocktype = "TRIPLE INDIRECT"
                    indir_offset = 65804 

                indirblock = inode.indirblocks_list[i]
                if indirblock == 0:
                    continue 

                if indirblock < 0 or indirblock >= max_block_num:
                    isconsistent += 1
                    print("INVALID {0} BLOCK {1} IN INODE {2} AT OFFSET {3}".format(blocktype, indirblock, inode.num_inode, indir_offset))
                elif indirblock > 0 and indirblock < first_legal_block_num:
                    isconsistent += 1
                    print("RESERVED {0} BLOCK {1} IN INODE {2} AT OFFSET {3}".format(blocktype, indirblock, inode.num_inode, indir_offset))

                alloc_block_to_inode.setdefault(indirblock,[]).append([inode.num_inode, i+1, indir_offset])


    for indirect in indirects:
        indir_level = indirect.level 
        indirect_offset = indirect.offset 

        indir_blocktype = ""
        if indir_level == 1:
            indir_blocktype = "INDIRECT"
        elif indir_level == 2:
            indir_blocktype = "DOUBLE INDIRECT"
        else:
            indir_blocktype = "TRIPLE INDIRECT"
    
        indir_block_index = indirect.curr_indir_block_index 
        if indir_block_index == 0:
            continue 
        
        if indir_block_index < 0 or indir_block_index >= max_block_num:
            isconsistent += 1
            print("INVALID {0} BLOCK {1} IN INODE {2} AT OFFSET {3}".format(indir_blocktype, indir_block_index, inode.num_inode, indirect_offset))
        elif indir_block_index > 0 and indir_block_index < first_legal_block_num:
            isconsistent += 1
            print("RESERVED {0} BLOCK {1} IN INODE {2} AT OFFSET {3}".format(indir_blocktype, indir_block_index, inode.num_inode, indirect_offset))

        alloc_block_to_inode.setdefault(indir_block_index,[]).append([indirect.num_inode, indir_level, indirect_offset])


    #figure out duplicate blocks
    for block in alloc_block_to_inode:
        if len(alloc_block_to_inode[block]) > 1:
            duplicate_blocks.add(block)


    for block_num in range(first_legal_block_num, max_block_num):
        if block_num not in alloc_block_to_inode and block_num not in free_blocks:
            isconsistent += 1
            print("UNREFERENCED BLOCK {0}".format(block_num))
    
        if block_num in alloc_block_to_inode and block_num in free_blocks:
            isconsistent += 1
            print("ALLOCATED BLOCK {0} ON FREELIST".format(block_num))
        
        
    for duplicate in duplicate_blocks:
        for arr in alloc_block_to_inode[duplicate]:
            inode_num = arr[0]
            level = arr[1]
            duplicate_offset = arr[2]

            dup_blocktype = ""
            if level == 0:
                dup_blocktype = "BLOCK"
            elif level == 1:
                dup_blocktype = "INDIRECT BLOCK"
            elif level == 2:
                dup_blocktype = "DOUBLE INDIRECT BLOCK"
            else:
                dup_blocktype = "TRIPLE INDIRECT BLOCK"
            
            print("DUPLICATE {0} {1} IN INODE {2} AT OFFSET {3}".format(dup_blocktype, duplicate, inode_num, duplicate_offset))

          


def Inode_Directory_Audit():
    global isconsistent
    first_inode = super_block.s_first_ino
    inode_count = super_block.s_inodes_count
    allocated = []
    ref_count = []
    parent_list = []

    for i in range(inode_count + first_inode):
        ref_count.append(0)
        parent_list.append(0)

    parent_list[2] = 2
    
    for allocated_inode in inodes:
        if allocated_inode.num_inode:
            allocated.append(allocated_inode.num_inode)
            if allocated_inode.num_inode in free_Inodes:
                isconsistent += 1
                print("ALLOCATED INODE {0} ON FREELIST".format(allocated_inode.num_inode))
    for i in range(first_inode, inode_count):
        if i not in allocated and i not in free_Inodes:
            isconsistent += 1
            print("UNALLOCATED INODE {0} NOT ON FREELIST".format(i))

    for dir_ent in dirents:
        child = dir_ent.inode
        cname = dir_ent.name
        if child < 0 or child > inode_count:
            isconsistent += 1
            print("DIRECTORY INODE {0} NAME {1} INVALID INODE {2}".format(dir_ent.num_inode, dir_ent.name, child))
        elif child not in allocated:
            isconsistent += 1
            print("DIRECTORY INODE {0} NAME {1} UNALLOCATED INODE {2}".format(dir_ent.num_inode, dir_ent.name, child))
        else:
            ref_count[child] += 1

        if cname !=  "'.'" and cname != "'..'": 
            parent_list[child] = dir_ent.num_inode 

    for i in inodes:
        if ref_count[i.num_inode] != i.i_links_count:
            print("INODE {0} HAS {1} LINKS BUT LINKCOUNT IS {2}".format(i.num_inode, ref_count[i.num_inode], i.i_links_count))
    
    for dir_ent in dirents:
        if dir_ent.name == "'.'" and dir_ent.num_inode != dir_ent.inode:
            isconsistent += 1
            print("DIRECTORY INODE {0} NAME '.' LINK TO INODE {1} SHOULD BE {2}".format(dir_ent.num_inode, dir_ent.inode, dir_ent.num_inode))
        if dir_ent.name == "'..'" and dir_ent.inode != parent_list[dir_ent.num_inode]:
            isconsistent += 1
            print("DIRECTORY INODE {0} NAME '..' LINK TO INODE {1} SHOULD BE {2}".format(dir_ent.num_inode, dir_ent.inode, parent_list[dir_ent.num_inode]))
    


def main():
    if len(sys.argv) != 2:
        sys.stderr.write("Error: Must provide CSV argument\n")
        sys.exit(1)

    try:
        input_file  = open(sys.argv[1], 'r')
        csv_file = csv.reader(input_file)
    except IOError:
        print("File could not be opened", file=sys.stderr)
        exit(1)

    for line in csv_file:
        if line[0] == "SUPERBLOCK":
            global super_block
            super_block = Superblock(line)
        elif line[0] == "GROUP":
            global group 
            group = Group(line)
        elif line[0] == "INODE":
            global inodes 
            inodes.append(Inode(line))
        elif line[0] == "DIRENT":
            global dirents
            dirents.append(Directory(line))
        elif line[0] == "INDIRECT":
            global indirects
            indirects.append(Indirect(line))
        elif line[0] == "BFREE":
            global free_blocks 
            free_blocks.append(int(line[1]))
        elif line[0] == "IFREE":
            global free_Inodes 
            free_Inodes.append(int(line[1]))
        else:
            sys.stderr.write("Error: Invalid argument in CSV file\n")
            sys.exit(2)

    #call Block Consistency Audits
    Block_Consistency_Audit()

    #call Inode_Directory_Audit
    Inode_Directory_Audit() 

    if(isconsistent):
        sys.exit(2)
    else:
        sys.exit(0)



if __name__ == '__main__':
    main()