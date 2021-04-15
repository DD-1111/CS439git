#include "ide.h"
#include "ext2.h"
#include "shared.h"
#include "libk.h"

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
        Debug::printf("***      is a file\n");
        auto sz = node->size_in_bytes();
        Debug::printf("***      contains %d bytes\n",sz);
        Debug::printf("***      has %d links\n",node->n_links());
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

    Debug::printf("In Kernel\n");

    // IDE device #1
    auto ide = Shared<Ide>::make(1);
    
    // We expect to find an ext2 file system there
    auto fs = Shared<Ext2>::make(ide);
   
   // get "/"
   auto root = fs->root;
   show("/",root,true);
   

   Debug::printf("*** Before change: \n");

   //  get "/hello"
   auto hello = fs->find(root,"hello");
   show("hello", hello, true);
   
   Debug::printf("*** After change: \n");
   //rename
   fs->renameFile("/", "hello", "goood");
   auto changed = fs->find(root,"goood");
   show("goood", changed, true);
   
   //-----------------testing remove()
   Debug::printf("*** testing remove(): \n");
   Debug::printf("*** Before change: \n");

   //  get "/goodbye"
   auto goodbye = fs->find(root,"goodbye");
   show("goodbye", goodbye, true);

   Debug::printf("*** After change: \n");
   //rename
   fs->removeFile("/", "goodbye");
   auto changed2 = fs->find(root,"goodbye");
   show("goood", changed2, true);




}


