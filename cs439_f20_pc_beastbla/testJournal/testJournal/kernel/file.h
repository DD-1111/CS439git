#ifndef _FILE_H_
#define _FILE_H_

#include "atomic.h"
#include "stdint.h"
#include "shared.h"
#include "kernel.h"
#include "threads.h"
#include "ide.h"
#include "ext2.h"
#include "debug.h"


class File {
    Atomic<uint32_t> ref_count;
public:
    File(): ref_count(0) {}
    virtual ~File() {}
    virtual bool isFile() = 0;
    virtual bool isDirectory() = 0;
    virtual off_t size() = 0;
    virtual off_t seek(off_t offset) = 0;
    virtual ssize_t read(void* buf, size_t size){
        return -1;
    }
    virtual ssize_t write(void* buf, size_t size){
        return -1;
    }

    friend class Shared<File>;
};

class OpenFile : public File{
	Shared<Node> node;
	uint32_t offset;

public:

	OpenFile(Shared<Node> n, uint32_t offset = 0) : node(n), offset(offset) {}
	~OpenFile() {
	}
    
	 bool isFile()  { return node ->is_file();}

     bool isDirectory()  { return node ->is_dir(); }

     off_t size()  { return node ->size_in_bytes(); }

     off_t seek(off_t offset)  {
    	this -> offset = offset;
        //Debug::printf("seek to offset = %d", offset);
    	return offset;
    }

     ssize_t read(void* buf, size_t size)  {
        //Debug::printf("openfile read:\n");
    	int32_t num = node -> read_all(offset, size, (char*)buf);
        //Debug::printf("openfilee read called, %d read\n", num);
    	if (num < 0) 
    		return num;
    	offset += num;
    	return num;
    }

     ssize_t write(void* buf, size_t size)  {
       // Debug::printf("))))openFile wirte called\n");
        using namespace gheith;
        // call the journal to buffer the content
    
        char* bufferContent = root_fs->jsb->Jwrite(node->number, offset, size, (char*)buf);

        // checkpointing the writing 
        // redo the content  
        auto lastTX = root_fs->jsb->getlastTX();

        auto tgtInumFromJournal = lastTX->inum;
        auto tgtNode = root_fs->get_node(tgtInumFromJournal);
        auto tgtSize = lastTX->tgtsize;
        auto tgtoffs = lastTX->tgtoffs;
        auto cunt = tgtNode->write(tgtoffs,tgtSize,(char*)bufferContent);
       // auto cunt = node ->write(offset, size, (char*)buf);
        offset += cunt;
    	return cunt;
    }
};


#endif
