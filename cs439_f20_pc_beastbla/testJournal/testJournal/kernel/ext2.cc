#include "ext2.h"
#include "libk.h"
#include "heap.h"

#if 0
template <typename T>
void println(T& t) {
    constexpr int N = sizeof(T);
    char* p = (char*) &t;
    for (int i=0; i<N; i++) {
        Debug::printf("%c",p[i]);
    }
    Debug::printf("\n");
}
#endif

Ext2::Ext2(Shared<Ide> ide): ide(ide), root(), ref_count(0) {
    SuperBlock sb;

    ide->read(1024,sb);

    iNodeSize = sb.inode_size;
    iNodesPerGroup = sb.inodes_per_group;
    numberOfNodes = sb.inodes_count;
    numberOfBlocks = sb.blocks_count;

    //Debug::printf("inodes_count %d\n",sb.inodes_count);
    //Debug::printf("blocks_count %d\n",sb.blocks_count);
    //Debug::printf("blocks_per_group %d\n",sb.blocks_per_group);
    //Debug::printf("inode_size %d\n",sb.inode_size);
    //Debug::printf("block_group_nr %d\n",sb.block_group_nr);
    //Debug::printf("first_inode %d\n",sb.first_inode);

    blockSize = uint32_t(1) << (sb.log_block_size + 10);
    
    nGroups = (sb.blocks_count + sb.blocks_per_group - 1) / sb.blocks_per_group;
    //Debug::printf("nGroups = %d\n",nGroups);
    ASSERT(nGroups * sb.blocks_per_group >= sb.blocks_count);

    auto superBlockNumber = 1024 / blockSize;
    //Debug::printf("super block number %d\n",superBlockNumber);

    auto groupTableNumber = superBlockNumber + 1;
    //Debug::printf("group table number %d\n",groupTableNumber);

    auto groupTableSize = sizeof(BlockGroup) * nGroups;
    //Debug::printf("group table size %d\n",groupTableSize);

    auto groupTable = new BlockGroup[nGroups];
    auto cnt = ide->read_all(groupTableNumber * blockSize, groupTableSize, (char*) groupTable);
    ASSERT(cnt == groupTableSize);

    iNodeTables = new uint32_t[nGroups];

    for (uint32_t i=0; i < nGroups; i++) {
        auto g = &groupTable[i];

        iNodeTables[i] = g->inode_table;

        //Debug::printf("group #%d\n",i);
        //Debug::printf("    block_bitmap %d\n",g->block_bitmap);
        //Debug::printf("    inode_bitmap %d\n",g->inode_bitmap);
        //Debug::printf("    inode_table %d\n",g->inode_table);

    }

    //Debug::printf("========\n");
    //Debug::printf("iNodeSize %d\n",iNodeSize);
    //Debug::printf("nGroups %d\n",nGroups);
    //Debug::printf("iNodesPerGroup %d\n",iNodesPerGroup);
    //Debug::printf("numberOfNodes %d\n",numberOfNodes);
    //Debug::printf("numberOfBlocks %d\n",numberOfBlocks);
    //Debug::printf("blockSize %d\n",blockSize);
    //for (unsigned i = 0; i < nGroups; i++) {
    //    Debug::printf("iNodeTable[%d] %d\n",i,iNodeTables[i]);
    //}

    //root = new Node(ide,2,blockSize);
    jsb = new Journal(ide);
    root = get_node(2);

    //root->show("root");

    //root->entries([](uint32_t inode, char* name) {
    //    Debug::printf("%d %s\n",inode,name);
    //});

    //println(sb.uuid);
    //println(sb.volume_name);
}

Shared<Node> Ext2::get_node(uint32_t number) {
    ASSERT(number > 0);
    ASSERT(number <= numberOfNodes);
    auto index = number - 1;

    auto groupIndex = index / iNodesPerGroup;
    //Debug::printf("groupIndex %d\n",groupIndex);
    ASSERT(groupIndex < nGroups);
    auto indexInGroup = index % iNodesPerGroup;
    auto iTableBase = iNodeTables[groupIndex];
    ASSERT(iTableBase <= numberOfBlocks);
    //Debug::printf("iTableBase %d\n",iTableBase);
    auto nodeOffset = iTableBase * blockSize + indexInGroup * iNodeSize;
    //Debug::printf("nodeOffset %d\n",nodeOffset);

    auto out = Shared<Node>::make(ide,number,blockSize);
    ide->read(nodeOffset,out->data);
    return out;
}


////////////// NodeData //////////////

void NodeData::show(const char* what) {
    Debug::printf("%s\n",what);
    Debug::printf("    mode 0x%x\n",mode);
    Debug::printf("    uid %d\n",uid);
    Debug::printf("    gif %d\n",gid);
    Debug::printf("    n_links %d\n",n_links);
    Debug::printf("    n_sectors %d\n",n_sectors);
}

///////////// Node /////////////

void Node::get_symbol(char* buffer) {
    ASSERT(is_symlink());
    auto sz = size_in_bytes();
    if (sz <= 60) {
        memcpy(buffer,&data.direct0,sz);
    } else {
        auto cnt = read_all(0,sz,buffer);
        ASSERT(cnt == sz);
    }
}

void Node::read_block(uint32_t index, char* buffer) {
    // Debug::printf("node_readBlock");
    ASSERT(index < data.n_sectors / (block_size / 512));

    auto refs_per_block = block_size / 4;

    uint32_t block_index;

    if (index < 12) {
        uint32_t* direct = &data.direct0;
        block_index = direct[index];
    } else if (index < (12 + refs_per_block)) {
        ide->read(data.indirect_1 * block_size + (index - 12) * 4,block_index);

    } else {
        block_index = 0;
        Debug::panic("index = %d\n",index);
    }

    auto cnt = ide->read_all(block_index * block_size, block_size,buffer);
    ASSERT(cnt == block_size);
}

void Node::write_block(uint32_t number, char* buffer) {
    // Debug::printf("node_writeBlock");
    uint32_t block_index;

    if (number < 12) {
        uint32_t* direct = &data.direct0;
        block_index = direct[number];
        uint32_t offset = block_index * block_size;
        ide->write_all(offset, block_size, buffer);
        return;
    }
    number -= 12;
    const uint32_t pc1 = block_size / sizeof(uint32_t);
    if(number < pc1) {
        uint32_t offset = (data.indirect_1 * block_size) + (number * sizeof(uint32_t));
        ide->write_all(offset, sizeof(  uint32_t), (char*) & offset);
        offset *= block_size;
        ide->write_all(offset, block_size, buffer);
        return;
    } else
    {
        Debug::printf("wrting to the second indirectory of indirection! Not implemented.");
    }
    return;
}


uint32_t Node::find(const char* name) {
    uint32_t out = 0;

    entries([&out,name](uint32_t number, const char* nm, uint32_t) {
        if (K::streq(name,nm)) {
            out = number;
        }
    });

    return out;
}

uint32_t Node::entry_count() {
    ASSERT(is_dir());
    uint32_t count = 0;
    entries([&count](uint32_t,const char*, uint32_t) {
        count += 1;
    });
    return count;
}

uint32_t Node::rename(const char* fileName, const char* new_filename) {

    uint32_t off = 0;
    uint32_t res = -1;

    entries([&off,fileName, &res](uint32_t number, const char* name, uint32_t offset) {
        if (K::streq(fileName,name)) {
            off = offset;
            res = 1;
           // Debug::printf("offset == %d\n", off);
        }
    });

    if(res == 1) {
        write_all(off+8, K::strlen(fileName), (char *)new_filename);
    }

    return res;
}

uint32_t Node::remove(const char* fileName){
    uint32_t res = -1;
    uint32_t off = 0;
    //uint32_t count = 0;
    //Debug::printf("start to remove node\n");
    entries([fileName, &res, &off](uint32_t, const char* name, uint32_t offset) {
        if (K::streq(fileName,name)) {
            res = 1;
             //Debug::printf("entry found\n");
            off = offset;
           // Debug::printf("offset == %d\n", off);
        }
    });

    if(res == 1) {       
        //Debug::printf("in Node::remove() res==1\n");
        uint32_t start = off;
        const char* deleted = "";
        write_all(start, 4, (char *)deleted);
        write_all(start + 6, 1, (char *)deleted);
        write_all(start + 7, 1, (char *)deleted);
        write_all(start + 8, K::strlen(fileName), (char *)deleted);  
        Debug::printf("File removed\n");
    }

    return res;
}


////////////////////Journal///////////////////

uint32_t Journal::txCommit(uint32_t inumber,uint32_t offs, uint32_t size, char* buffer){
    
   Transaction* newTXwithID = txQueue->newTx();
   txQueue->addTx(newTXwithID);
    Debug::printf("\nCalling Journal:___");

   // mark the txstart
   newTXwithID->markStart();
   Debug::printf("TXStart marked___");
   // set node and bitmap content
   newTXwithID->setNode(inumber);
   // write in the buffer inside of transction
     newTXwithID->tgtoffs = offs;
   newTXwithID->tgtsize = size;
   newTXwithID->writeinTx(buffer);
  
   // Safely get buffered into the Journal!!!
   // mark the txend
      Debug::printf("Og datas and dest offs buffered___");
   newTXwithID->markEnd();
    Debug::printf("TXEnd marked___\n");

   // one tranction completed

   return newTXwithID->txID;
}// return the newest TXID


void Journal::redo(Transaction *cur)
{
    // auto node = Ext2::get_node(cur->inum);
    char *bufferInNode = new char[cur->tgtsize];
    ide->read_all(cur->tgtoffs, cur->tgtsize, bufferInNode);
    //if content in inode is not equal to buffer in commit
    //,it means it's not consistent and we need to redo it
    if (!K::streq(bufferInNode, cur->buffer))
    {
        //calculate correct offset
        ide->write_all(cur->tgtoffs, cur->tgtsize, cur->buffer);
    }
}

int Journal::checkWholeList()
{
    Transaction *currentTx = txQueue->getfirst();
    int allgood = 1;
    int IncompleteID;
    while (currentTx)
    {
        if (!currentTx->checkComplete())
        {
            allgood = 0;
            IncompleteID = currentTx->txID;
            Debug::printf("transaction id = %d is not completed\n", IncompleteID);
            redo(currentTx);
            break;
        }
        currentTx = currentTx->next;
    }
    return allgood;
}

void Journal::checkConsistency()
{
    
    //first check if journals in buffer are valid
    checkWholeList();

    //check if we need to redo the last commit
    Transaction *cur = txQueue->getlast();
    redo(cur);
}



int Journal::checkLastCommitValid() {
    return txQueue->getlast()->checkComplete();
}


char* Journal::Jwrite(uint32_t inumber, uint32_t offs, uint32_t size ,char* buffer) {
    // first, write into journal
    // store the writein data with the form of transaction
    char* returnedBuf = new char[0];
    int lastid = txCommit(inumber,offs, size, buffer); 
    // transaction complete. Now do checkpointing 

    if (checkLastCommitValid()) {
        Debug::printf("____Transaction ID = %d successfully commited*****\n",lastid);
        Transaction* recentCommit = txQueue->getlast();
        returnedBuf = recentCommit->buffer;

        return returnedBuf;
        //put check pointing outside 
        // &+*** to avoid structure including problem
    } else
    {
        Debug::printf("there is a dirty transaction recorded in TX id = %d\n", txQueue->getnewestTXID());
        Debug::printf("Last commit disregarded...\n");
    }
    return returnedBuf;

}

void Journal::printJournal() {
    Debug::printf("***Visualizing journal....***\n");
    Debug::printf("Current Stored commit : %d/30 \n", txQueue->commitNumberStored());
    Debug::printf("TXQUEUE:\n");
    Transaction* cur = txQueue->getlast();
    while (cur) {
        cur ->printTX();
        cur = cur->next;
    }
    Debug::printf("***Journal End***\n");
}
