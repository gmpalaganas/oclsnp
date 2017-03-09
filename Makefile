CC = g++
BOOST_COMPUTE = /usr/include/boost
INCLUDES = -Isrc/snp -Isrc/utils -I$(BOOST_COMPUTE)
LIBS = -lOpenCL -lre2 -pthread
CFLAGS = -std=c++14 -Wall
OBJS = build/snp/snp.o\
	   build/snp/regex_tree.o\
	   build/snp/boost_emulator.o\
	   build/utils/array.o\
	   build/utils/binary_reader.o\
	   build/utils/regex.o\
	   build/utils/opencl_error.o

all: build_directories bin/oclsnp bin/linsnp bin/kernels
	@echo Done

bin/oclsnp: build/main.o $(OBJS) 
	$(CC) -o $@ $(CFLAGS) $(INCLUDES) $(LIBS) $^

bin/linsnp: build/main_linear.o $(OBJS)
	$(CC) $(LIBS) build/main_linear.o build/utils/array.o build/snp/snp.o build/snp/regex_tree.o build/utils/binary_reader.o build/utils/regex.o -o bin/linsnp

build/%.o: src/%.cpp
	$(CC) $(CFLAGS) -c $(INCLUDES) $< $(LIBS) -o $@

build/utils/%.o: src/utils/%.cpp
	$(CC) $(CFLAGS) -c $(INCLUDES) $< $(LIBS) -o $@

build/snp/%.o: src/snp/%.cpp
	$(CC) $(CFLAGS) -c $(INCLUDES) $< $(LIBS) -o $@

bin/kernels: update_kernels
	@echo Copying Kernels

build_directories:
	mkdir -p build/utils build/snp

update_kernels:
	if [ -d "bin/kernels" ]; then rm -r bin/kernels; fi
	cp -r src/kernels bin/

clean:
	rm build/**/* build/* bin/oclsnp bin/linsnp bin/kernels -r
