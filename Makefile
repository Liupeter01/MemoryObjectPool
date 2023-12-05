BUILD_DIR:=build
CXXFLAGS:= -O0 -Wall -Wextra -g -std=c++14 -stdlib=libc++ -IMemoryPool/include
MEMORYPOOL_CXXFLAGS:= $(CXXFLAGS) 
OBJECTPOOL_CXXFLAGS:= $(CXXFLAGS) -IObjectPool/include 

MEMORYPOOL_BUILD_DIR:=$(BUILD_DIR)/MemoryPool
OBJECTPOOL_BUILD_DIR:=$(BUILD_DIR)/ObjectPool

TARGET := $(BUILD_DIR)/output

MEMORYPOOL_CXX_FILES := $(wildcard MemoryPool/src/*.cpp)
OBJECTPOOL_CXX_FILES := $(wildcard ObjectPool/src/*.cpp)

MEMORYPOOL_OBJ_FILES := $(MEMORYPOOL_CXX_FILES:MemoryPool/src/%.cpp=$(MEMORYPOOL_BUILD_DIR)/%_cpp.o)
OBJECTPOOL_OBJ_FILES := $(OBJECTPOOL_CXX_FILES:ObjectPool/src/%.cpp=$(OBJECTPOOL_BUILD_DIR)/%_cpp.o)

.PHONY: clean all build
all: $(TARGET)

#compile cpp files and generate obj files
$(MEMORYPOOL_BUILD_DIR)/%_cpp.o: MemoryPool/src/%.cpp
	mkdir -p $(@D)
	$(CXX) $(MEMORYPOOL_CXXFLAGS) -c $< -o $@

$(OBJECTPOOL_BUILD_DIR)/%_cpp.o: ObjectPool/src/%.cpp
	mkdir -p $(@D)
	$(CXX) $(OBJECTPOOL_CXXFLAGS) $(OBJECTPOOL_LIBS) -c $< -o $@
	
$(TARGET): $(MEMORYPOOL_OBJ_FILES) $(OBJECTPOOL_OBJ_FILES)
	$(CXX) $(OBJECTPOOL_CXXFLAGS) $^ -o $@

clean:
	@echo "Cleaning up..."
	rm -rf $(BUILD_DIR)
