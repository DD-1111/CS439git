#ifndef _ext2_h_
#define _ext2_h_

#include "ide.h"
#include "shared.h"
#include "atomic.h"
#include "heap.h"
#include "libk.h"

struct SuperBlock {
    uint32_t inodes_count;
    uint32_t blocks_count;
    uint32_t reserved_blocks_count;
    uint32_t free_blocks_count;
    uint32_t free_inodes_count;
    uint32_t first_data_block;
    uint32_t log_block_size;
    uint32_t log_frag_size;
    uint32_t blocks_per_group;
    uint32_t frags_per_group;
    uint32_t inodes_per_group;
    uint32_t mtime;
    uint32_t wtime;
    uint16_t mnt_count;
    uint16_t max_mnt_count;
    uint16_t magic;
    uint16_t state;
    uint16_t errors;
    uint16_t minor_rev_level;
    uint32_t lastcheck;
    uint32_t checkinterval;
    uint32_t creator_os;
    uint32_t rev_level;
    uint16_t def_resuid;
    uint16_t def_resgid;
    uint32_t first_inode;
    uint16_t inode_size;
    uint16_t block_group_nr;
    uint32_t feature_compat;
    uint32_t feature_incompat;
    uint32_t feature_ro_compat;
    char uuid[16];
    char volume_name[16];
    uint32_t algo_bitmap;
};

struct BlockGroup {
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;
    uint16_t free_blocks_count;
    uint16_t free_inodes_count;
    uint16_t used_dirs_count;
    uint16_t pad;
    char reserved[12];
};

// just the bits
struct NodeData {
    uint16_t mode;
    uint16_t uid;
    uint32_t size_low;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint16_t gid;
    uint16_t n_links;
    uint32_t n_sectors;
    uint32_t flags;
    uint32_t os1;
    uint32_t direct0;
    uint32_t direct1;
    uint32_t direct2;
    uint32_t direct3;
    uint32_t direct4;
    uint32_t direct5;
    uint32_t direct6;
    uint32_t direct7;
    uint32_t direct8;
    uint32_t direct9;
    uint32_t direct10;
    uint32_t direct11;
    uint32_t indirect_1;
    uint32_t indirect_2;
    uint32_t indirect_3;
    uint32_t gen;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t fragment;
    char os2[12];

    inline uint16_t get_type() {
        return mode >> 12;
    }

    bool is_dir() {
        return get_type() == 4;
    }

    bool is_file() {
        return get_type() == 8;
    }

    bool is_symlink() {
        return get_type() == 0xa;
    }

    void show(const char*);
};

// A wrapper around an i-node
class Node : public BlockIO { // we implement BlockIO because we
                              // represent data

    Shared<Ide> ide;
    Atomic<uint32_t> ref_count;

public:

    // i-number of this node
    const uint32_t number;
    NodeData data;

    Node(Shared<Ide> ide, uint32_t number, uint32_t block_size) : BlockIO(block_size), ide(ide), ref_count(0), number(number) {

    }

    virtual ~Node() {}

    // How many bytes does this i-node represent
    //    - for a file, the size of the file
    //    - for a directory, implementation dependent
    //    - for a symbolic link, the length of the name
    uint32_t size_in_bytes() override {
        return data.size_low;
    }

    // read the given block (panics if the block number is not valid)
    // remember that block size is defined by the file system not the device
    void read_block(uint32_t number, char* buffer) override;
    void write_block(uint32_t number, char* buffer) override;

uint32_t rename(const char* filename, const char* new_filename);
uint32_t remove(const char* filename);

    inline uint16_t get_type() {
        return data.get_type();
    }

    // true if this node is a directory
    bool is_dir() {
        return data.is_dir();
    }

    // true if this node is a file
    bool is_file() {
        return data.is_file();
    }

    // true if this node is a symbolic link
    bool is_symlink() {
        return data.is_symlink();
    }

    // If this node is a symbolic link, fill the buffer with
    // the name the link referes to.
    //
    // Panics if the node is not a symbolic link
    //
    // The buffer needs to be at least as big as the the value
    // returned by size_in_byte()
    void get_symbol(char* buffer);

    // Returns the number of hard links to this node
    uint32_t n_links() {
        return data.n_links;
    }

    void show(const char* msg) {
        data.show(msg);
    }

    template <typename Work>
    void entries(Work work) {
        ASSERT(is_dir());
        uint32_t offset = 0;

        while (offset < data.size_low) {
            uint32_t inode;
            read(offset,inode);
            //Debug::printf("entry: inode : %d\n", inode);
            uint16_t total_size;
            read(offset+4,total_size);
            //Debug::printf("total_size for start: %d\n", total_size);
            uint8_t name_length;
            read(offset+6,name_length);
            auto name = new char[name_length+1];
            name[name_length] = 0;
            auto cnt = read_all(offset+8,name_length,name);
            ASSERT(cnt == name_length);
           work(inode,name,offset);
            delete[] name;
            offset += total_size;
            //Debug::printf("current offset == %d\n", offset);
        }
    }

    uint32_t find(const char* name);

    // Returns the number of entries in a directory node
    //
    // Panics if not a directory
    uint32_t entry_count();

    friend class Shared<Node>;
};



class Transaction {
    public:
    Transaction(uint32_t txID):txID(txID), txStart(0),txEnd(0) {}
    Transaction* next;
    uint32_t txID;
    uint32_t txStart;
    uint32_t inum; // Node position
    uint32_t tgtsize;
    uint32_t tgtoffs;


    char* buffer = (char*)malloc(1024); 
    // Use physical design, write in twice, more straight forward and safer,
    // but more expensive.
    //limited to 1K
    uint32_t txEnd;

    ~Transaction() {}

    void markStart() {
        txStart = 1;
    }

    void writeinTx(char* data) {
         memcpy(buffer, data, tgtsize);
    }

    void setNode(int num) {
        inum = num;
    }

    void markEnd() {
        txEnd = 1;
    }

    int checkComplete() { // 1 complete, 0 not complete.
        if (txStart && txEnd) {
            return 1;
        }
        return 0;
    } 

    void printTX() {
        Debug::printf("TXID == %d\n", txID);
        Debug::printf("Inode == %d\n", inum);
        Debug::printf("target offset == %d\n", tgtsize); 
         Debug::printf("TX status == %d (1 is complete, 0 is not)\n\n", checkComplete());
    }
};


class TransactionQueue {
  

private :
    Transaction* first = nullptr ;
    Transaction* last = nullptr;
    uint32_t circularBufferSize;
    uint32_t curTXID = 0;
public:

    TransactionQueue() {}
    TransactionQueue(uint32_t buffSize) :circularBufferSize(buffSize) {}
  
    Transaction* newTx () {
        auto curTX = new Transaction(curTXID);
        Debug::printf("newtx with id = %d created\n", curTXID);
        return curTX;
    }

    void addTx (Transaction* t) {
        t ->next = nullptr;
        if (first == nullptr) {
            first = t;
        } else if (curTXID < circularBufferSize) {   
            last->next = t;
        } else {
            last->next = t;
            //Transaction* temp = first;
            first = first->next;
            //delete [] temp;
        } 
        curTXID++;
        last = t;   
    }


    Transaction* getfirst () {
        return first;
    } 

    Transaction* getlast () {
        return last;
    } 

    uint32_t getnewestTXID() {
        return curTXID;
    }

    uint32_t commitNumberStored() {
        Transaction* cur = first;
        uint32_t cnt = 0;
        while (cur) {
            cnt ++;
            cur = cur->next;
        }
        return cnt;
    }

};

class Journal {

    Shared<Ide> ide;
    TransactionQueue* txQueue;
    uint32_t startOfJSB = 0;
    Atomic<uint32_t> ref_count = 0;
    //void clearBlock(uint32_t id); 

    int checkLastCommitValid();

    //void undoWrite(uint32_t id);

    
uint32_t txCommit(uint32_t inumber,uint32_t offs, uint32_t size, char* buffer);


public:
    Journal(Shared<Ide> ide) : ide(ide) {
        Debug::printf("\n*************   journal loaded....\n\n");
        txQueue = new TransactionQueue(30);
        Debug::printf("No previous transaction found\n\n");
        Debug::printf("\n************* \n\n");
    }
     virtual ~Journal() {}

   friend class Shared<Journal>;

   void redo(Transaction *cur);
   void checkConsistency();
    char* Jwrite(uint32_t inumber, uint32_t offs, uint32_t size ,char* buffer);
    
    // show all the commit s
    void printJournal();

    int checkWholeList(); // return whether all commits are healthy there, 1 is all good

    Transaction* getlastTX () {
        return txQueue->getlast();
    }
};


// This class encapsulates the implementation of the Ext2 file system
class Ext2 {
    // The device on which the file system resides
    Shared<Ide> ide;
public:
    // The root directory for this file system
    Shared<Node> root;
    Shared<Journal> jsb;
private:
    Atomic<uint32_t> ref_count;
    uint32_t blockSize;
    uint32_t numberOfNodes;
    uint32_t numberOfBlocks;
    uint32_t iNodeSize;
    uint32_t nGroups;
    uint32_t *iNodeTables;
    uint32_t iNodesPerGroup;
public:
    // Mount an existing file system residing on the given device
    // Panics if the file system is invalid
    Ext2(Shared<Ide> ide);

    friend class Shared<Ext2>;

    // Returns the block size of the file system. Doesn't have
    // to match that of the underlying device
    uint32_t get_block_size() {
        return blockSize;
    }

    // Returns the actual size of an i-node. Ext2 specifies that
    // an i-node will have a minimum size of 128B but could have
    // more bytes for extended attributes
    uint32_t get_inode_size() {
        return iNodeSize;
    }

    // Returns the node with the given i-number
    Shared<Node> get_node(uint32_t number);

    

    // If the given node is a directory, return a reference to the
    // node linked to that name in the directory.
    //
    // Returns a null reference if "name" doesn't exist in the directory
    //
    // Panics if "dir" is not a directory
    Shared<Node> find(Shared<Node> current, const char* path) {

        auto part = new char[257];
        uint32_t idx = 0;

        while (true) {
            while (path[idx] == '/') idx++;
            if ((current == nullptr) || (path[idx] == 0)) {
                goto done;
            }
            uint32_t i = 0;
            while (true) {
                auto c = path[idx];
                if ((c == 0) || (c == '/')) break;
                idx ++;
                ASSERT(i < 256);
                part[i++] = c;
            }
            part[i] = 0;
            auto number = current->find(part);
            if (number == 0) {
                current = Shared<Node>{};

                goto done;
            } else {
                current = get_node(number);
            }
        }

        done:
            delete[] part;
            return current;
    }





   //abs path of the directory
    /* return value: 1->succesfully remove, -1->no such file*/
    uint32_t renameFile(const char* path, const char* filename, const char* new_filename) {

        Shared<Node> node = find(root, path);
        uint32_t lenOld = K::strlen(filename);
        uint32_t lenNew = K::strlen(new_filename);

        //only support same length
        if(lenOld != lenNew) {
            return -1;
        }

        return node->rename(filename, new_filename);
    }


    uint32_t removeFile(const char* fileName){
       
        return root->remove(fileName);
    }

};




#endif