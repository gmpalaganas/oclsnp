CC = g++
INCLUDES = -Isrc/snp -Isrc/utils
LIBS = -lOpenCL -lre2
CFLAGS = -c -std=c++11 -Wall

all: oclsnp linsnp bin/kernels

oclsnp: build/main.o build/utils/array.o build/snp/snp.o build/snp/regex_tree.o build/snp/utils.o
	$(CC) $(LIBS) build/main.o build/utils/array.o build/snp/snp.o build/snp/regex_tree.o build/snp/utils.o -o bin/oclsnp

linsnp: build/main_linear.o build/utils/array.o build/snp/snp.o build/snp/regex_tree.o build/snp/utils.o
	$(CC) $(LIBS) build/main_linear.o build/utils/array.o build/snp/snp.o build/snp/regex_tree.o build/snp/utils.o -o bin/linsnp

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

build/snp/utils.o: src/snp/utils.cpp
	$(CC) $(CFLAGS) $(INCLUDES) src/snp/utils.cpp $(LIBS) -o $@

bin/kernels:
	cp -r src/kernels bin/

clean:
	rm *.o bin/oclsnp bin/linsnp bin/kernels -r
