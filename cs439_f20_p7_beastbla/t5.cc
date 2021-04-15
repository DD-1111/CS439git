#include "debug.h"
#include "atomic.h"
#include "threads.h"
#include "shared.h"
#include "blocking_lock.h"

// @Vassi
// https://www.streamscheme.com/wp-content/uploads/2020/04/poggers.png

// This test tests the following:
//  * Your constructors work for a shared pointer. This includes:
//      1. make constructor (Giving pointer)
//      2. blank constructor (nullptr)
//      3. copy constructor
//      4. move constructor
//  * Your operators work for a given shared pointer. This includes:
//      1. Copying a constructor
//      2. Reassinging to a pointer
//      3. Moving an r-value pointer
//  * Your destructors work for a given shared pointer. This includes:
//      1. Correctly deleting only when no shared pointers share a pointer
//
//  Some scary assumptions that were verified on Piazza:
//      1. Your implementation uses ref_count as the assumed variable in the shared pointer
//      2. ref_count is an uint32_t atomic variable
//      3. ref_count increases by 1 when a pointer is shared to another shared
//      4. ref_count decreases by 1 when a pointer is moved on from a shared
//      5. 0 in ref_count represents that the pointer is not being shared anymore

int ves_counters_del = 0;

template <typename T>
struct ves_Counter {
    Atomic<uint32_t> ref_count{0};

    ~ves_Counter() {
        ves_counters_del += 1;
        Debug::printf("*** Counter Deleted (%d total)\n", ves_counters_del);
    }
};

void print_test(int section, int test, const char* test_str) {
    Debug::printf("*** Test %d.%d - %s :\n", section, test, test_str);
}

void print_results(bool result, const char* fail_str) {
    if (result) {
        Debug::printf("***\t\t\tPASSED [POG]\n");
    } else {
        Debug::printf("***\t\t\tFAILED [NOT POG] (%s)\n", fail_str);
    }
}

Shared<ves_Counter<BlockingLock>> rval_shared() {
    return Shared<ves_Counter<BlockingLock>>::make();
}

void kernelMain() {
    int section = 1;
    int test = 1;

    // SHARED CONSTRUCTORS
    Debug::printf("*** TEST SECTION 1 --- SHARED CLASS\n");
    Debug::printf("Oh also this test assumes -> works to get the internal pointer\n");
    Debug::printf("If nothing at all is working make sure -> is all good to go.\n");

    // Test 1.1
    print_test(section, test, "Normal Initialization");
    Shared<ves_Counter<BlockingLock>> count_1 = Shared<ves_Counter<BlockingLock>>::make();
    Debug::printf("Real count: %u --- Exp count: %u\n", count_1->ref_count.get(), 1);
    Debug::printf("Method tested: Shared(T* it)\n");
    print_results(count_1->ref_count.get() == 1, "Incorrect counter val. Ref count should be 1 after association");
    test += 1;


    // Test 1.2
    print_test(section, test, "Nullptr Initialization");
    Shared<ves_Counter<BlockingLock>> count_2;
    Debug::printf("Initializing shared pointer w/ a nullptr. If you terminate/stop\n"
        " here you're likely deferencing a nullptr (are you trying to modify ptr?)\n");
    Debug::printf("Method tested: Shared()\n");
    print_results(true, "");
    test += 1;


    // Test 1.3 & 1.4
    print_test(section, test, "Copy Initialization (a)");
    Shared<ves_Counter<BlockingLock>> count_3 = count_1;
    Debug::printf("Calling the copy constructor for count_3. Essentially should\n"
        "add 1 to the ref count they'll now share since there should now be two\n"
        "different shareds (count_1 and count_3) looking at the same thing\n"
        "(the ves_Counter<BlockingLock> created w/ count_1)\n");
    Debug::printf("count_1->ref_count: %x --- ");
    Debug::printf("count_3->ref_count: %x\n");
    Debug::printf("count_1->ref_count.get(): %d --- ", count_1->ref_count.get());
    Debug::printf("count_3->ref_count.get(): %d\n", count_3->ref_count.get());
    Debug::printf("Method tested: Shared(const Shared& rhs)\n");
    print_results(&(count_1->ref_count) == &(count_3->ref_count), "Shared pointer is different. Are you not using rhs's pointer?");
    test += 1;

    print_test(section, test, "Copy Initialization (b)");
    print_results(count_3->ref_count.get() == 2, "Incorrect counter val. Could you have forgotten to add one to your counter?");
    test += 1;


    // Test 1.5
    print_test(section, test, "Move Initialization");
    Shared<ves_Counter<BlockingLock>> count_4 = rval_shared();
    Debug::printf("Calling the move constructor for count_4. This grabs a temporary value\n"
    "that our new shared will grab as opposed to doing expensive copy operations. If you're confused\n"
    "check out this video: https://youtu.be/JAOZjf4KneY\n");
    Debug::printf("Method tested: Shared(const Shared&& rhs)\n");
    print_results(count_4->ref_count.get() == 1, "Incorrect counter val. Could you have forgotten to add one to your counter?");
    test += 1;

    // SHARED ASSIGN OPERATIONS
    // Test 1.6 & 1.7
    print_test(section, test, "Normal Reassignment (to new Shared) (a)");
    auto lock_ptr = new ves_Counter<BlockingLock>{};
    count_2 = lock_ptr;
    Debug::printf("count_2->ref_count: %x --- lock_ptr->ref_count: %x\n", &(count_2->ref_count), &(lock_ptr->ref_count));
    Debug::printf("count_2->ref_count.get(): %d --- lock_ptr->ref_count.get(): %d\n", count_2->ref_count.get(), lock_ptr->ref_count.get());
    Debug::printf("Assigning counter_2 to use an already created blocking lock as a pointer.\n");
    Debug::printf("Method tested: Shared<T>& operator=(const Shared<T>& rhs)\n");
    print_results(count_2->ref_count.get() == 1, "Incorrect counter val. Could you have forgotten to add one to your counter?");
    test += 1;

    print_test(section, test, "Normal Reassignment (new internal pointer) (b)");
    print_results(&(count_2->ref_count) == &(lock_ptr->ref_count), "Different pointers found. Are you not sharing the pointer given to you?");
    test += 1;

    // Test 1.8 & 1.9 & 1.10
    print_test(section, test, "Normal Reassignment (new internal pointer) (a)");
    Debug::printf("Assigning counter_1 to use counter_2's pointer instead of counter_3's\n");
    Debug::printf("INFO BEFORE:\n");
    Debug::printf("count_1 : ref_count: %x ref_count.get(): %d\n", &(count_1->ref_count), count_1->ref_count.get());
    Debug::printf("count_2 : ref_count: %x ref_count.get(): %d\n", &(count_2->ref_count), count_2->ref_count.get());
    Debug::printf("count_3 : ref_count: %x ref_count.get(): %d\n", &(count_3->ref_count), count_3->ref_count.get());
    count_1 = count_2;
    Debug::printf("INFO AFTER:\n");
    Debug::printf("count_1 : ref_count: %x ref_count.get(): %d\n", &(count_1->ref_count), count_1->ref_count.get());
    Debug::printf("count_2 : ref_count: %x ref_count.get(): %d\n", &(count_2->ref_count), count_2->ref_count.get());
    Debug::printf("count_3 : ref_count: %x ref_count.get(): %d\n", &(count_3->ref_count), count_3->ref_count.get());
    Debug::printf("Method tested: Shared<T>& operator=(Shared<T>&& rhs)\n");
    print_results(count_1->ref_count.get() == 2, "Incorrect counter val. Could you have forgotten to add one to your counter? Are you moving from your old pointer?");
    test += 1;

    print_test(section, test, "Normal Reassignment (new internal pointer) (b)");
    print_results(&(count_1->ref_count) == &(count_2->ref_count), "Different pointers found. Are you not sharing the pointer given to you?");
    test += 1;

    print_test(section, test, "Normal Reassignment (new internal pointer) (c)");
    print_results(count_3->ref_count.get() == 1, "Incorrect counter val. Are you counting off when you move from an old thread?\n");
    test += 1;

    // Test 1.11 & 1.12 & 1.13
    print_test(section, test, "Move Reassignment (new internal pointer) (a)");
    Debug::printf("Assigning counter_1 to use an r-val shared and moving away from count_2\n");
    Debug::printf("INFO BEFORE:\n");
    Debug::printf("count_1 : ref_count: %x ref_count.get(): %d\n", &(count_1->ref_count), count_1->ref_count.get());
    Debug::printf("count_2 : ref_count: %x ref_count.get(): %d\n", &(count_2->ref_count), count_2->ref_count.get());
    count_1 = rval_shared();
    Debug::printf("INFO AFTER:\n");
    Debug::printf("count_1 : ref_count: %x ref_count.get(): %d\n", &(count_1->ref_count), count_1->ref_count.get());
    Debug::printf("count_2 : ref_count: %x ref_count.get(): %d\n", &(count_2->ref_count), count_2->ref_count.get());
    print_results(count_1->ref_count.get() == 1, "Incorrect counter val. Could you have forgotten to add one to your counter? Are you moving from your old pointer?");
    test += 1;

    print_test(section, test, "Move Reassignment (new internal pointer) (b)");
    print_results(&(count_1->ref_count) != &(count_2->ref_count), "Same pointer between count_1 and count_2. Are you not sharing the pointer given to you?");
    test += 1;

    print_test(section, test, "Normal Reassignment (new internal pointer) (c)");
    print_results(count_2->ref_count.get() == 1, "Incorrect counter val. Are you counting off when you move from an old thread?\n");
    test += 1;

    // SHARED EQUALITY OPERATIONS
    // Test 1.14 & 1.15 & 1.16
    print_test(section, test, "Pointer Equality (a)");
    Debug::printf("Just making sure your pointer (shared pointer) equalities are good to go\n");
    auto eq_ptr_1 = new ves_Counter<BlockingLock>{};
    auto eq_ptr_2 = new ves_Counter<BlockingLock>{};
    Shared<ves_Counter<BlockingLock>> count_5;
    Shared<ves_Counter<BlockingLock>> count_6;
    count_5 = eq_ptr_1;
    count_6 = eq_ptr_1;
    print_results(count_5 == eq_ptr_1, "Incorrect equality. Are you comparing your internal pointer?");
    count_5 = eq_ptr_2;
    test += 1;

    print_test(section, test, "Pointer Equality (b)");
    print_results(count_5 != eq_ptr_1, "Incorrect equality. Are you correctly removing the old pointer?");
    test += 1;
    
    print_test(section, test, "Pointer Equality (c)");
    print_results(count_5 == eq_ptr_2, "Incorrect equality. Are you comparing your internal pointer?");
    test += 1;

    // Test 1.17 & 1.18
    print_test(section, test, "Shared Equality (a)");
    Debug::printf("Just making sure your shared equalities are good to go\n");
    Debug::printf("Should be very similar results as above\n");
    Shared<ves_Counter<BlockingLock>> count_7 = count_6;
    print_results(count_6 == count_7, "Incorrect equality. Are you comparing your internal pointers correctly?");
    test += 1;

    print_test(section, test, "Shared Equality (b)");
    print_results(count_6 != count_5, "Incorrect equality. Are you updating your internal pointers correctly?");
    test += 1;

    // SHARED DESTRUCTOR OPERATIONS
    // Test 1.17 & 1.18
    print_test(section, test, "Shared Destructor (a)");
    Debug::printf("About to handle destructing pointers once we're done w/ 'em. This is\n");
    Debug::printf("where it's likely gonna get messy\n");
    Debug::printf("STATUS REPORT (count 6 and 7 should be the same):\n");
    Debug::printf("count_1 ref_counter: %x\n", &(count_1->ref_count));
    Debug::printf("count_2 ref_counter: %x\n", &(count_2->ref_count));
    Debug::printf("count_3 ref_counter: %x\n", &(count_3->ref_count));
    Debug::printf("count_4 ref_counter: %x\n", &(count_4->ref_count));
    Debug::printf("count_5 ref_counter: %x\n", &(count_5->ref_count));
    Debug::printf("count_6 ref_counter: %x\n", &(count_6->ref_count));
    Debug::printf("count_7 ref_counter: %x\n", &(count_7->ref_count));
    Debug::printf("We're gonna start combining them all into 1 giant shared pointer (and then removing it).\n");
    Debug::printf("The way you'll test this is just by comparing your output to the .ok file\n");
    Debug::printf("Don't think there's an easier way to check when destructors are checked. I'll throw in some assertion tests\n");
    Debug::printf("to make keeping track of where it goes wrong easier.\n");
    bool start_con = &(count_6->ref_count) == &(count_7->ref_count) && count_6->ref_count.get() == 2;
    print_results(start_con, "Incorrect setup. You probably shouldn't proceede (other issues first)");
    if (!start_con) {
        Debug::panic("Shuting down. You have a problem before your destructors so let's worry about these destructors last\n");
    }
    test += 1;

    count_4 = count_5;
    count_3 = count_4;
    print_test(section, test, "Destructor Checkpoint Check (doesn't check destruction) (b)");
    print_results(&(count_3->ref_count) == &(count_5->ref_count) && count_3->ref_count.get() == 3, "Incorrect equality. Are you updating your internal pointers correctly?");
    test += 1;

    count_3 = count_6;
    count_4 = count_6;
    count_5 = count_6;
    print_test(section, test, "Destructor Checkpoint Check (doesn't check destruction) (c)");
    print_results(count_3->ref_count.get() == 5, "Incorrect equality. Are you updating your internal pointers correctly?");
    test += 1;

    count_2 = count_1;
    count_3 = count_1;
    count_4 = count_1;
    count_5 = count_1;
    count_6 = count_1;
    count_7 = count_1;
    print_test(section, test, "Destructor Checkpoint Check (doesn't check destruction) (d)");
    print_results(count_1->ref_count.get() == 7, "Incorrect equality. Are you updating your internal pointers correctly?");

    Debug::printf("*** Test done! POG (Removing last big 'ol shared pointer\n");
}

//        ms:                              .--o`     
//   /----ymmdo--------------------------:/. `+---:` 
//   + o::/yNmmdy/::::://:+: s::/::::::/o-   +/:+::. 
//  `/ dho:.+dmmmd+```````:: s````````:-   -:.--:o-- 
//  .: symmms:odmmm:/`````:: s``````:/. `-:---` ++`/ 
//  --`o`ymmmmy:+hm+mh/```:: s```.--:/:-::-.  `::+ + 
//  /.oy//sdmmmm::-/Nmmd+.:: s`.--` /.-./`  `-:.-s-+ 
//  + /Nmmdo+ymm+ydoohmmmdy/ s/.  -::-:/: .:/:-.`:-+ 
//  + -oymmmds+oosmmdsoymmms/s/`-:::.`:/:/:-`  `:s + 
//  + /s:+ymmmm/++sdmmmh/o/`/s+:::. .-::./`  .-:-s-+ 
//  + :mmds/+ymsymdyshds:` `:s./``.:::-+./.-/:-.`::+ 
//  + /+hNmmh+:+/mmmmd+. ./+-y-/:/:-` .+-:/:`` `:s + 
//  + .-:+dmmmh+dsoyddo`-/o-`+:/+.``.-:+++.   .::- + 
//  +`ossosyhmmommdysooo/.``.y:+ .-o+-.-++`.-++///-/`
// `/ odmmmds+o+/dmmN-/-`.:+:s-+:::-. `/./---:-..//:.
// .: o.smmmmmh`--:oh./.::/-.o..:. `.-::.:-``  .:+:--
// -- s.-:+hdmN+odyo/`o/:..:-s+.+.-:::.+./``.--.`:/.:
// /. s.hhs+/+os-mmmm./.`-++.-:-::-.` `/-/------+:+`/
// +  s`-dmmmdh.:/osy/.-::-.-s-`/` `.--.`:-.``.:..o +
// +  s`.-ohdmNsodhs:+/:-./+::+::-:----+--``.:-```o +
// + `o`-yhhyssy/ymmy/--:::-`/.:/-..` :-+:-::-:/``s +
// + .+``.smmmmd/.+o/o:::.:/:s:/```.:::`:..``--```s +
// + :/````:ohmdd.dd-/:::/:-.+:/::/:.::/.`---.````s +
// + /-````-hdhyo:/h///:-`-/-s:..```-:-//:-:/`````s +
// + +:.````-hmmmmh//--::/:/:h-.-:://-/```-:````..s +
// /:.-::/-.``/oyhd:+/:-.`:/`y/:--..:+:--:.`..:/::.-/
//   .---`-::/ydhy./---://s/::-::/:++syhhy:/::.---.  
//       .---`-+yd+//-..-::osmhdmhsNmmmmo:.---.      
//           --:-:-``-::.`/sodss+ohmhss+:--          
//            .::--+:`-:/::: s-/::-shmmmy-           
//         `/+/-.   `--:``:. /..---`  ./sdy:         
//       .:-`           .------`          `-/`       
