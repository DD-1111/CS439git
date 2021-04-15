SHELL := $(shell which bash)
TEST_EXTS = .cc .ok
UTCS_ID ?= $(shell pwd | sed -e 's/.*_//')

MY_TESTS = ${addprefix ${UTCS_ID},${TEST_EXTS}}
SIG = ${firstword ${shell (ls ${MY_TESTS} > /dev/null 2>&1  && (cat ${MY_TESTS} | md5sum)) || echo "?" }}

POSSIBLE_TESTS = ${basename ${wildcard *${firstword ${TEST_EXTS}}}}
TESTS = ${sort ${POSSIBLE_TESTS}}
TEST_OKS = ${addsuffix .ok,${TESTS}}
TEST_RESULTS = ${addsuffix .result,${TESTS}}
TEST_TARGETS = ${addsuffix .test,${TESTS}}
TEST_OUTS = ${addsuffix .out,${TESTS}}
TEST_RAWS = ${addsuffix .raw,${TESTS}}
TEST_DIFFS = ${addsuffix .diff,${TESTS}}
TEST_LOOPS = ${addsuffix .loop,${TESTS}}

KVM_FLAG = 
QEMU_FLAGS = -no-reboot \
             -nographic\
             -smp 4\
             --monitor none \
             --serial file:$*.raw -drive file=kernel/build/$*.img,index=0,media=disk,format=raw \
             -device isa-debug-exit,iobase=0xf4,iosize=0x04 ${KVM_FLAG}

TIME = $(shell which time)

#KERNELS = ${addsuffix .kernel,${TESTS}}

.PHONY: ${TESTS} sig test tests all clean ${TEST_TARGETS}

all : ${TESTS};


$(TESTS) : % :
	@$(MAKE) -C kernel --no-print-directory build/$*.img || $(MAKE) -C kernel build/$*.img

clean:
	rm -rf *.diff *.raw *.out *.result *.kernel *.failure *.time
	(make -C kernel clean)

${TEST_RAWS} : %.raw : Makefile %
	@echo -n "$* ... "
	@rm -f $*.raw $*.failure
	@touch $*.failure
	@echo "*** failed to run, look in $*.failure for more details" > $*.raw
	-${TIME} --quiet -o $*.time -f "%E" timeout 10 qemu-system-i386 ${QEMU_FLAGS} > $*.failure 2>&1 || true

${TEST_OUTS} : %.out : Makefile %.raw
	-egrep '^\*\*\*' $*.raw > $*.out 2> /dev/null || true

${TEST_DIFFS} : %.diff : Makefile %.out %.ok
	-(diff -wBb $*.out $*.ok > $*.diff 2> /dev/null || true)

${TEST_RESULTS} : %.result : Makefile %.diff
	(test -z "`cat $*.diff`" && echo "pass" > $*.result) || echo "fail" > $*.result
	echo "$* `cat $*.result` `cat $*.time` [`qemu-system-i386 --version | head -n 1`] [`g++ --version | head -n 1`] [`/bin/date`]" >> history

${TEST_TARGETS} : %.test : Makefile %.result
	@echo "`cat $*.result` `cat $*.time`"

HOW_MANY = ${shell who | sed -e 's/ .*//' | sort | uniq | wc -l}
LOOP_LIMIT ?= 10

${TEST_LOOPS} : %.loop : %
	@echo "*******************************************************************************"
	@echo "*** This is NOT the sort of thing you run ALL THE TIME on a SHARED MACHINE  ***"
	@echo "*** In particular long running tests and tests that timeout                 ***"
	@echo "*******************************************************************************"
	@echo ""
	@echo "You can use LOOP_LIMIT to control the number if iterations. For example:"
	@echo "   LOOP_LIMIT=7 make -s $*.loop"
	@echo ""
	@echo "::::::: You are 1 of ${HOW_MANY} users on this machine"
	@echo ":::::::   all of them value their work and their time as much as you value yours"
	@echo ":::::::"
	@echo ""
	@let pass=0; \
	for ((i=1; i<=${LOOP_LIMIT}; i++)); do \
		echo -n  "[$$i/${LOOP_LIMIT}] "; \
		$(MAKE) -s $*.test; \
		if [ "`cat $*.result`" = "pass" ]; then let pass=pass+1; fi; \
	done; \
	echo ""; \
	echo "$$pass/${LOOP_LIMIT}"; \
	echo ""

test: Makefile ${TESTS} ${TEST_TARGETS} ;
	-@echo ""
	-@echo "pass:`(grep pass *.result | wc -l) || echo 0`/`(ls *.result | wc -l) || echo 0`"
	-@echo "fail:`(grep fail *.result | wc -l) || echo 0`/`(ls *.result | wc -l) || echo 0`"
	-@echo ""

###################### Test #################

sig:
	@echo ${SIG}

tests:
	@echo "$(TESTS)"

copy_student_tests:
ifeq ($(SIG),?)
	@echo "no tests"
else
ifeq ($(TEST_DIR),)
	@echo "no TEST_DIR"
else
	@for i in $(TEST_EXTS); do \
	    cp $(UTCS_ID)$$i $(TEST_DIR)/$(SIG)$$i; \
	done
endif
endif

copy_given_tests:
ifeq ($(TEST_DIR),)
	@echo "no TEST_DIR"
else
	@for i in $(TEST_EXTS); do \
	    cp t?$$i $(TEST_DIR); \
	done
endif

remove_tests:
	@for i in $(TEST_EXTS); do \
	    rm -rf *$$i; \
	done

copy_results:
ifeq ($(RESULTS_DIR),)
	@echo "no RESULTS_DIR"
else
	-cp *.result $(RESULTS_DIR)
	-cp *.ok $(RESULTS_DIR)
	-cp *.out $(RESULTS_DIR)
	-cp *.raw $(RESULTS_DIR)
endif

