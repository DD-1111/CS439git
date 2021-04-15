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

SuperBlock info;

Ext2::Ext2(Shared<Ide> ide) : ide(ide), root(), ref_count(0)
{
    SuperBlock sb;

    ide->read(1024, sb);

    info = sb;

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
    auto cnt = ide->read_all(groupTableNumber * blockSize, groupTableSize, (char *)groupTable);
    ASSERT(cnt == groupTableSize);

    iNodeTables = new uint32_t[nGroups];

    for (uint32_t i = 0; i < nGroups; i++)
    {
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

    root = get_node(2);

    //root->show("root");

    //root->entries([](uint32_t inode, char* name) {
    //    Debug::printf("%d %s\n",inode,name);
    //});

    //println(sb.uuid);
    //println(sb.volume_name);
}

Shared<Node> Ext2::get_node(uint32_t number)
{
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

    auto out = Shared<Node>::make(ide, number, blockSize);
    ide->read(nodeOffset, out->data);
    return out;
}

////////////// NodeData //////////////

void NodeData::show(const char *what)
{
    Debug::printf("%s\n", what);
    Debug::printf("    mode 0x%x\n", mode);
    Debug::printf("    uid %d\n", uid);
    Debug::printf("    gif %d\n", gid);
    Debug::printf("    n_links %d\n", n_links);
    Debug::printf("    n_sectors %d\n", n_sectors);
}

///////////// Node /////////////

void Node::get_symbol(char *buffer)
{
    ASSERT(is_symlink());
    auto sz = size_in_bytes();
    if (sz <= 60)
    {
        memcpy(buffer, &data.direct0, sz);
    }
    else
    {
        auto cnt = read_all(0, sz, buffer);
        ASSERT(cnt == sz);
    }
}



void Node::read_block(uint32_t index, char *buffer)
{
    ASSERT(index < data.n_sectors / (block_size / 512));

    auto refs_per_block = block_size / 4;

    uint32_t block_index;

    if (index < 12)
    {
        uint32_t *direct = &data.direct0;
        block_index = direct[index];
    }
    else if (index < (12 + refs_per_block))
    {
        ide->read(data.indirect_1 * block_size + (index - 12) * 4, block_index);
    }
    else
    {
        block_index = 0;
        Debug::panic("index = %d\n", index);
    }

    auto cnt = ide->read_all(block_index * block_size, block_size, buffer);
    ASSERT(cnt == block_size);
}

void Node::write_block(uint32_t index, char *buffer)
{

    // Debug::printf("index = %d\n",index);
    // Debug::printf("assert index = %d\n", data.n_sectors / (block_size / 512));

    ASSERT(index < data.n_sectors / (block_size / 512));

    auto refs_per_block = block_size / 4;

    uint32_t block_index;

    if (index < 12)
    {
        uint32_t *direct = &data.direct0;
        block_index = direct[index];
    }
    else if (index < (12 + refs_per_block))
    {
        ide->read(data.indirect_1 * block_size + (index - 12) * 4, block_index);
    }
    else
    {
        block_index = 0;
        Debug::panic("index = %d\n", index);
    }

    auto cnt = ide->write_all(block_index * block_size, buffer, block_size);
    ASSERT(cnt == block_size);
}

uint32_t Node::find(const char *name)
{
    uint32_t out = 0;

    entries([&out, name](uint32_t number, const char *nm, uint32_t) {
        if (K::streq(name, nm))
        {
            out = number;
        }
    });

    return out;
}

uint32_t Node::entry_count()
{

    uint32_t count = 0;
    entries([&count](uint32_t, const char *, uint32_t) {
        count += 1;
    });
    return count;
}

uint32_t Node::rename(const char *fileName, const char *new_filename)
{

    uint32_t off = 0;
    uint32_t res = -1;

    entries([&off, fileName, &res](uint32_t number, const char *name, uint32_t offset) {
        if (K::streq(fileName, name))
        {
            off = offset;
            res = 1;
        }
    });

    if (res == 1)
    {
        //off+8 is where the name starts
        write_all(off + 8, (char *)new_filename, K::strlen(fileName));
    }
    return res;

    // uint8_t name_length = (uint8_t)(K::strlen(new_filename));
    // char *name = (char *)malloc(name_length + 1);
    // name[name_length] = 0;
    // read_all(off + 8, name_length, name);
    // Debug::printf("%s\n", name);
}

uint32_t Node::remove(const char *fileName)
{

    uint32_t res = -1;
    uint32_t off = 0;
    uint32_t count = 0;

    entries([fileName, &res, &off](uint32_t, const char *name, uint32_t offset) {
        if (K::streq(fileName, name))
        {
            res = 1;
            off = offset;
        }
    });

    // Debug::printf("offset: %d\n", off);

    if (res == 1)
    {

        // Debug::printf("in Node::remove() res==1\n");
        uint32_t start = off;
        uint32_t next = off + 8 + K::strlen(fileName) + 1;
        //moving following entries one unit forward
        while (next < data.size_low)
        {

            // Debug::printf("start : %d\n", start);
            // Debug::printf("next : %d\n", next);
            // Debug::printf("len : %d\n", K::strlen(fileName));

            // Debug::printf("-----next entry\n");

            uint32_t inode;
            read(next, inode);
            // Debug::printf("inode : %d\n", inode);

            uint16_t total_size;
            read(next + 4, total_size);
            // Debug::printf("total_size : %d\n", total_size);

            uint8_t name_length;
            read(next + 6, name_length);
            // Debug::printf("name_length : %d\n", name_length);

            char *name = (char *)malloc(name_length);
            name[name_length] = 0;
            auto nbytesRead = read_all(next + 8, name_length, name);
            // Debug::printf("name : %s\n", name);
            ASSERT(nbytesRead == name_length);


            uint16_t targetSize;
            read(start + 4, targetSize);
            uint16_t sizeSum = targetSize + total_size;

            //if next entry is the last one, we need to update total size
            if ((next + total_size) >= data.size_low)
            {
                //no change for inode
                //increase total size of target node : sizetarget + sizeNext
                //change the lenCur to sizeSum - 8; //8 is meta data beside name
                //change the nameCur to nameNext

                // Debug::printf("-----before\n");
                // Debug::printf("inode : %d\n", ( char *)(inode));
                // Debug::printf("name_length : %d\n", ( char *)((uint32_t)name_length));
                // Debug::printf("sizeSum : %d\n", ( char *)((uint32_t)sizeSum));
                // Debug::printf("name : %s\n", name);
                // Debug::printf("-----start write\n");

                uint16_t next_len;
                read(start + 6, next_len);

                write_all(start, (char *)(inode), 4);

                write_all(start + 4, (char *)((uint32_t)sizeSum), 2);

                write_all(start + 6, (char *)((uint32_t)name_length), 1);

                write_all(start + 8, name, next_len);

                break;
            }
            else
            {
                // Debug::printf("in else\n");
                write_all(start, (char *)(inode), 4);

                write_all(start + 4, (char *)((uint32_t)sizeSum), 4);

                write_all(start + 6, (char *)((uint32_t)name_length), 2);

                write_all(start + 8, name, name_length);


                start += total_size;
                next += total_size;

                // Debug::printf("---finish name\n");
                free(name);
            }
            count++;

            // Debug::printf("-----current entry\n");

            // read(start, inode);
            // Debug::printf("inode : %d\n", inode);

            // read(start + 4, total_size);
            // Debug::printf("total_size : %d\n", total_size);

            // read(start + 6, name_length);
            // Debug::printf("name_length : %d\n", name_length);

            // name = (char *)malloc(name_length);
            // name[name_length] = 0;
            // nbytesRead = read_all(start + 8, name_length, name);
            // Debug::printf("name : %s\n", name);
        }
    }
    return res;
}


