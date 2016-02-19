.Phony: clean tests test

CXX = g++
LIBS = -lboost_filesystem -lboost_system -lboost_program_options
CXXFLAGS = -Wall -Wextra -Wpedantic -std=c++14 -O3 -ggdb3 -g3 -fdiagnostics-color=auto -fopenmp -Wno-error=narrowing
BUILD_NUMBER_FILE = build-number.txt

all: bin/get_cooccurence_from_dir bin/create_vocab
#all: bin/BNC_to_text bin/get_cooccurence_from_dir bin/sparse

dummy_build_folder := $(shell mkdir -p obj)
dummy_bin_folder := $(shell mkdir -p bin)

bin/BNC_to_text: src/BNC_to_text.cpp
	g++  $^ $(CXXFLAGS) -o $@ $(LIBS)

bin/get_cooccurence_from_dir: obj/get_cooccurence_from_dir.o obj/buffer_byte.o obj/ternary_tree.o obj/vocabulary.o obj/stream_reader.o obj/string_tools.o obj/file_io.o
	g++  $^ $(CXXFLAGS) -o $@ $(LIBS)

bin/create_vocab: obj/create_vocab.o obj/buffer_byte.o obj/ternary_tree.o obj/vocabulary.o obj/stream_reader.o obj/string_tools.o obj/file_io.o
	g++  $^ $(CXXFLAGS) -o $@ $(LIBS)

bin/my_w2v: obj/my_w2v.o obj/buffer_byte.o obj/ternary_tree.o obj/vocabulary.o obj/stream_reader.o obj/string_tools.o
	$(CXX)  $^ $(CXXFLAGS) -o $@ $(LIBS)

obj/my_w2v.o: src/my_w2v.cpp 
	g++  $^ $(CXXFLAGS) -c -o $@ 

obj/get_cooccurence_from_dir.o: src/get_cooccurence_from_dir.cpp 
	g++  $^ $(CXXFLAGS) -c -o $@ 

obj/create_vocab.o: src/create_vocab.cpp 
	g++  $^ $(CXXFLAGS) -c -o $@ 

obj/vocabulary.o: src/vocabulary.cpp
	g++  $^ $(CXXFLAGS) -c -o $@ 

obj/string_tools.o: src/string_tools.cpp
	g++  $^ $(CXXFLAGS) -c -o $@ 

obj/buffer_byte.o: src/basic_utils/buffer_byte.cpp
	g++  $^ $(CXXFLAGS) -c -o $@ 

obj/stream_reader.o: src/basic_utils/stream_reader.cpp
	g++  $^ $(CXXFLAGS) -c -o $@ 

obj/file_io.o: src/basic_utils/file_io.cpp
	g++  $^ $(CXXFLAGS) -c -o $@ 

obj/ternary_tree.o: src/ternary_tree.cpp
	g++  $^ $(CXXFLAGS) -c -o $@ 

bin/sparse: src/sparse.cpp
	g++  $^ $(CXXFLAGS) -o $@ $(LIBS)

tests: bin/test_vocab bin/test_tree

bin/test_vocab: src/tests/test_vocab.cpp obj/ternary_tree.o obj/vocabulary.o obj/stream_reader.o obj/string_tools.o
	g++  $^ $(CXXFLAGS) -o $@ $(LIBS)

bin/test_tree: obj/test_tree.o obj/string_tools.o obj/ternary_tree.o
	g++  $^ $(CXXFLAGS) -o $@ $(LIBS)

obj/test_tree.o: src/tests/test_tree.cpp 
	g++  $^ $(CXXFLAGS) -c -o $@ 

TEST_CORP = ../data/test_corpora/rus/punctuation/

test: all
	./bin/create_vocab $(TEST_CORP) temp/ --minimal_frequency=1
	./bin/get_cooccurence_from_dir $(TEST_CORP) temp/ --window_size=2 --debug=true 

clean:
	rm -f obj/*.o

BUILD_NUMBER_LDFLAGS  = -Xlinker --defsym -Xlinker __BUILD_DATE=$$(date +'%Y%m%d')
BUILD_NUMBER_LDFLAGS += -Xlinker --defsym -Xlinker __BUILD_NUMBER=$$(cat $(BUILD_NUMBER_FILE))

# Build number file.  Increment if any object file changes.
$(BUILD_NUMBER_FILE): bin/get_cooccurence_from_dir
	@if ! test -f $(BUILD_NUMBER_FILE); then echo 0 > $(BUILD_NUMBER_FILE); fi
	@echo $$(($$(cat $(BUILD_NUMBER_FILE)) + 1)) > $(BUILD_NUMBER_FILE)