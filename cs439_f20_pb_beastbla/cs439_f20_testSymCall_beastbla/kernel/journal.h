#include "ext2.h"
#include "queue.h"
#include "future.h"
#include "shared.h"
    // metadata only: inode, inodeBitmap, dataBitmap
    // write/renameNode: only inode /dataBitmap change
    // newNode: not implemented yet
    // deleteNode: inode/inodeBitmap


struct Transaction{
    uint32_t txID;
    uint32_t txStart;
    uint32_t* inums; // Node position
    Node* node;    // Node content
    uint32_t* inode_entry;
    uint32_t* inode_data;
    uint32_t txEnd;
};

class Journal {
    uint32_t startOfJSB;
    uint32_t endOfJSB;
   // SomeKindofQueue<Transaction> txQueue;
    void setTailPtr(uint32_t offset);
    void clearBlock(uint32_t id); 
    bool checkValid(uint32_t id);
    void undoWrite(uint32_t id);
    void Jread(uint32_t id, uint32_t offset, void* buffer, uint32_t n);
    void Jwrite(uint32_t id, uint32_t offset, void* buffer, uint32_t n);
public:
    Journal(uint32_t start) {
        startOfJSB = start;
        endOfJSB = start;
    }
    ~Journal();
    void txAdd(Transaction* t); // add transaction to queue, lock the node
    uint32_t txCommit(); // return the block descriptor addr 
    // get transaction from queue, write to journal
    // descriptor+commit, release lock, advance tail pointer
    void writeComplete(uint32_t id); // Mark the journal descriptor block
    void replay(); // start from head ptr, check commit block, replay, stop at tail ptr
    void makeSpace(); // advance the head pointer (read descriptor blocks)
};


