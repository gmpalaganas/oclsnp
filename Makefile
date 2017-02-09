CC = g++
INCLUDES = -Isrc/snp -Isrc/utils
LIBS = -lOpenCL -lre2 -pthread
CFLAGS = -c -std=c++11 -Wall

all: oclsnp linsnp bin/kernels

oclsnp: build/main.o build/utils/array.o build/snp/snp.o build/snp/regex_tree.o build/utils/binary_reader.o build/utils/regex.o
	$(CC) $(LIBS) build/main.o build/utils/array.o build/snp/snp.o build/snp/regex_tree.o build/utils/binary_reader.o build/utils/regex.o -o bin/oclsnp

linsnp: build/main_linear.o build/utils/array.o build/snp/snp.o build/snp/regex_tree.o build/utils/binary_reader.o build/utils/regex.o
	$(CC) $(LIBS) build/main_linear.o build/utils/array.o build/snp/snp.o build/snp/regex_tree.o build/utils/binary_reader.o build/utils/regex.o -o bin/linsnp

build/main.o: src/main.cpp 
	$(CC) $(CFLAGS) $(INCLUDES) src/main.cpp $(LIBS) -o $@

build/main_linear.o: src/main_linear.cpp
	$(CC) $(CFLAGS) $(INCLUDES) src/main_linear.cpp $(LIBS) -o $@

build/utils/array.o: src/utils/array.cpp
	mkdir build/utils/;$(CC) $(CFLAGS) $(INCLUDES) src/utils/array.cpp $(LIBS) -o $@ 

build/snp/snp.o: src/snp/snp.cpp
	mkdir build/snp/;$(CC) $(CFLAGS) $(INCLUDES) src/snp/snp.cpp $(LIBS) -o $@

build/snp/regex_tree.o: src/snp/regex_tree.cpp
	$(CC) $(CFLAGS) $(INCLUDES) src/snp/regex_tree.cpp $(LIBS) -o $@

build/utils/binary_reader.o: src/utils/binary_reader.cpp
	$(CC) $(CFLAGS) $(INCLUDES) src/utils/binary_reader.cpp $(LIBS) -o $@

build/utils/regex.o: src/utils/regex.cpp
	$(CC) $(CFLAGS) $(INCLUDES) src/utils/regex.cpp $(LIBS) -o $@

bin/kernels:
	if [ -d "bin/kernels" ]; then rm -r bin/kernels; fi
	cp -r src/kernels bin/

update_kernels:
	if [ -d "bin/kernels" ]; then rm -r bin/kernels; fi
	cp -r src/kernels bin/

clean:
	rm *.o bin/oclsnp bin/linsnp bin/kernels -r
