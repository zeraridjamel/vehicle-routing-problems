
CPP = g++

CURL_CFLAGS := $(shell pkg-config --cflags curlpp)
CURL_LIBS := $(shell pkg-config --libs curlpp)
JSON_LIBS = -L /lib/x86_64-linux-gnu/ -ljson

BASETRASH = baseTrash
BASECLASSES = baseClasses
INITTRASH = initTrash
SOLVERTRASH = solverTrash
PSQLTRASH = psqlTrash

BUILD_DIR = ./build
BIN_DIR = ./bin

INCLUDES = -I../../$(BASECLASSES) -I../../$(BASETRASH) -I../../$(INITTRASH) -I../../$(SOLVERTRASH) -I. -I.. -I/usr/include/postgresql/9.2/server
CPPFLAGS = -g -O0 -MMD  -MP $(CURL_CFLAGS) $(DEFINES) -I/usr/local/include -DPGROUTING 
LDFLAGS = -lgd $(INCLUDES) -L$(BASECLASSES) -L$(BASETRASH) -L$(INITTRASH) $(CURL_LIBS) $(JSON_LIBS) -L/usr/local/lib -lglog

SRCS =  
OBJS = $(addprefix $(BUILD_DIR)/,$(SRCS:.cpp=.o))
HEADS = $(SRCS:.cpp=.h)



BASECLASSES_SRCS = $(wildcard ../../$(BASECLASSES)/*.cpp)
BASECLASSES_HDS = $(UTIL_SRCS:.cpp=.h)
BASECLASSES_OBJS = $(addprefix $(BUILD_DIR)/$(BASECLASSES)/,$(notdir $(BASECLASSES_SRCS:.cpp=.o)))
BASECLASSES_HEADS = ../../$(BASECLASSES)/twpath.h ../../$(BASECLASSES)/twc.h ../../$(BASECLASSES)/twbucket.h ../../$(BASECLASSES)/plot.h ../../$(BASECLASSES)/singleton.h ../../$(BASECLASSES)/timer.h ../../$(BASECLASSES)/pg_types_vrp.h ../../$(BASECLASSES)/logger.h$(BASECLASSES_HDS)


BASETRASH_SRCS = $(wildcard ../../$(BASETRASH)/*.cpp)
BASETRASH_HDS = $(BASETRASH_SRCS:.cpp=.h) ../../$(BASETRASH)/trashconfig.h
BASETRASH_OBJS = $(addprefix $(BUILD_DIR)/$(BASETRASH)/,$(notdir $(BASETRASH_SRCS:.cpp=.o)))
BASETRASH_HEADS = $(BASETRASH_HDS) $(BASECLASSES_HEADS)

INITTRASH_SRCS = $(wildcard ../../$(INITTRASH)/*.cpp)
INITTRASH_HDS = $(INITTRASH_SRCS:.cpp=.h)
INITTRASH_OBJS = $(addprefix $(BUILD_DIR)/$(INITTRASH)/,$(notdir $(INITTRASH_SRCS:.cpp=.o)))
INITTRASH_HEADS = $(INITTRASH_HDS)  $(BASECLASSES_HEADS)

SOLVERTRASH_SRCS = $(wildcard ../../$(SOLVERTRASH)/*.cpp)
SOLVERTRASH_HDS = $(SOLVERTRASH_SRCS:.cpp=.h)
SOLVERTRASH_OBJS = $(addprefix $(BUILD_DIR)/$(SOLVERTRASH)/,$(notdir $(SOLVERTRASH_SRCS:.cpp=.o)))
SOLVERTRASH_HEADS = $(SOLVERTRASH_HDS)  $(BASECLASSES_HEADS)

PSQLTRASH_SRCS = $(wildcard ../*.cpp)
PSQLTRASH_HDS = $(wildcard ../*.h)
PSQLTRASH_OBJS = $(addprefix $(BUILD_DIR)/$(PSQLTRASH)/,$(notdir $(PSQLTRASH_SRCS:.cpp=.o)))
PSQLTRASH_HEADS = $(PSQLTRASH_HDS) $(BASECLASSES_HEADS)


DEPS = $(addprefix $(BUILD_DIR)/,$(SRCS:.cpp=.d))




all:   $(OBJS) $(BASECLASSES_OBJS) $(BASETRASH_OBJS) $(INITTRASH_OBJS) $(SOLVERTRASH_OBJS) $(PSQLTRASH_OBJS) build/tester.o ./bin/tester



build/tester.o: tester.cpp $(BASECLASSES_HEADS) $(BASETRASH_HEADS) $(INITTRASH_HEADS) $(SOLVERTRASH_HDS) $(HEADS)
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

build/baseClasses/%.o: ../../$(BASECLASSES)/%.cpp  ../../$(BASECLASSES)/%.h $(BASECLASSES_HEADS)
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

build/baseTrash/%.o: ../../$(BASETRASH)/%.cpp ../../$(BASETRASH)/%.h  $(BASECLASSES_HEADS) $(BASETRASH_HEADS)
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c $< -o $@


build/initTrash/%.o: ../../$(INITTRASH)/%.cpp  ../../$(INITTRASH)/%.h  $(BASECLASSES_HEADS) $(BASETRASH_HEADS) $(INITTRASH_HEADS)
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

build/solverTrash/%.o: ../../$(SOLVERTRASH)/%.cpp  ../../$(SOLVERTRASH)/%.h  $(BASECLASSES_HEADS) $(BASETRASH_HEADS) $(SOLVERTRASH_HEADS)
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

build/psqlTrash/%.o: ../%.cpp  $(BASECLASSES_HEADS) $(BASETRASH_HEADS) $(PSQLTRASH_HEADS)
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

build/%.o: %.cpp %.h $(BASECLASSES_HEADS) $(BASETRASH_HEADS) $(INITTRASH_HEADS) $(SOLVERTRASH_HEADS)
	$(CPP) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

bin/tester: build/tester.o $(OBJS) $(BASECLASSES_OBJS) $(BASETRASH_OBJS) $(INITTRASH_OBJS) $(SOLVERTRASH_OBJS) $(PSQLTRASH_OBJS) $(BASECLASSES_HEADS) $(BASETRASH_HEADS) $(INITTRASH_HEADS) $(SOLVERTRASH_HEADS) $(PSQLTRASH_HEADS)
	$(CPP) $^ -o $@ $(LDFLAGS)

test: ./bin/tester test.txt
	./bin/tester  >./logs/muni-b.out

valgrind: ./bin/tester test.txt
	valgrind -v --track-origins=yes --leak-check=full ./bin/tester

callgrind-8: ./bin/tester test.txt
	valgrind --tool=callgrind --callgrind-out-file=logs/callgrind.out ./bin/tester
	callgrind_annotate logs/callgrind.out | tee logs/callgrind.txt


.PHONY: clean

clean:
	rm -f bin/tester $(OBJS) $(BASECLASSES_OBJS) $(BASETRASH_OBJS) $(INITTRASH_OBJS)
	find . -name "*.d" -type f -delete
	find . -name "*.png" -type f -delete
	mkdir -p $(BIN_DIR)
	echo $(INITTRASH)
	mkdir -p $(BUILD_DIR)/$(BASECLASSES) $(BUILD_DIR)/$(BASETRASH) $(BUILD_DIR)/$(INITTRASH) $(BUILD_DIR)/$(SOLVERTRASH) $(BUILD_DIR)/$(PSQLTRASH) logs


cleanAll: clean
	$(MAKE) -C ../../$(BASECLASSES) clean
	$(MAKE) -C ../../$(BASETRASH) clean
	$(MAKE) -C ../../$(INITTRASH) clean
	$(MAKE) -C ../../$(SOLVERTRASH) clean
	rm ./bin/tester

-include $(DEPS)

