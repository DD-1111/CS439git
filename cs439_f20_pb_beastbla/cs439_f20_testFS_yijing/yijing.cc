#include "ide.h"
#include "ext2.h"
#include "shared.h"
#include "libk.h"

/*
    This is test case use t0's method show().
    1.Checks if we can't find the file in nested directory
    and prints out file in each level.
    2.Check non exist file
*/

void show(const char* name, Shared<Node> node, bool show) {

    Debug::printf("*** looking at %s\n",name);

    if (node == nullptr) {
        Debug::printf("***      does not exist\n");
        return;
    } 

    if (node->is_dir()) {
        Debug::printf("***      is a directory\n");
        Debug::printf("***      contains %d entries\n",node->entry_count());
        Debug::printf("***      has %d links\n",node->n_links());
    } else if (node->is_symlink()) {
        Debug::printf("***      is a symbolic link\n");
        auto sz = node->size_in_bytes();
        Debug::printf("***      link size is %d\n",sz);
        auto buffer = new char[sz+1];
        buffer[sz] = 0;
        node->get_symbol(buffer);
        Debug::printf("***       => %s\n",buffer);
    } else if (node->is_file()) {
        // Debug::printf("***      is a file\n");
        auto sz = node->size_in_bytes();
        // Debug::printf("***      contains %d bytes\n",sz);
        // Debug::printf("***      has %d links\n",node->n_links());
        if (show) {
            auto buffer = new char[sz+1];
            buffer[sz] = 0;
            auto cnt = node->read_all(0,sz,buffer);
            CHECK(sz == cnt);
            CHECK(K::strlen(buffer) == cnt);
            // can't just print the string because there is a 1000 character limit
            // on the output string length.
            for (uint32_t i=0; i<cnt; i++) {
                Debug::printf("%c",buffer[i]);
            }
            delete[] buffer;
            Debug::printf("\n");
        }
    } else {
        Debug::printf("***    is of type %d\n",node->get_type());
    }
}

/* Called by one CPU */
void kernelMain(void) {

    // IDE device #1
    auto ide = Shared<Ide>::make(1);
    
    // We expect to find an ext2 file system there
    auto fs = Shared<Ext2>::make(ide);

   auto root = fs->root;

   //files in ./yijing.dir
   show("/file1.txt",fs->find(root,"file1.txt"),true);
   Debug::printf("\n");
   show("/file2.txt",fs->find(root,"file2.txt"),true);
   Debug::printf("\n");

   auto dir1 = fs->find(root, "dir1");
   auto dir2 = fs->find(dir1, "dir2");
   auto dir3 = fs->find(dir2, "dir3");
   auto dir4 = fs->find(dir3, "dir4");

   show("/test",fs->find(root,"test"),true);
   Debug::printf("\n");
   show("/dir1/a.txt",fs->find(dir1,"a.txt"),true);
   Debug::printf("\n");
   show("/dir1/dir2/b.txt",fs->find(dir2,"b.txt"),true);
   Debug::printf("\n");
   show("/dir1/dir2/dir3/c.txt",fs->find(dir3,"c.txt"),true);
   Debug::printf("\n");
   show("/dir1/dir2/dir3/dir4/d.txt",fs->find(dir4,"d.txt"),true);
   Debug::printf("\n");

   Debug::printf("*** PASS!\n");
}