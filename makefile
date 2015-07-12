CCFLAGS=-std=c++0x
CFLAGS=
CPP=g++ -g
CC=cc -g

ifeq ($(OS),Windows_NT)
	LDFLAGS=-lwsock32
	OBJS=main.o mbdb_record.o mbdbdump.o sha1.o mman.o
else
	LDFLAGS=
	OBJS=main.o mbdb_record.o mbdbdump.o sha1.o
endif

.PHONY : clean

mbdbbuilder: $(OBJS)
	$(CPP) -o $@ $(OBJS) $(LDFLAGS) 

main.o: mbdb_builder/main.cpp
	$(CPP) -c $< $(CCFLAGS)
mbdb_record.o: mbdb_builder/mbdb_record.cpp
	$(CPP) -c $< $(CCFLAGS)
mbdbdump.o: mbdb_builder/mbdbdump.cpp
	$(CPP) -c $< $(CCFLAGS)
sha1.o: mbdb_builder/sha1.cpp
	$(CPP) -c $< $(CCFLAGS)
mman.o: mbdb_builder/mman.c
	$(CC) -c $< $(CFLAGS)

clean:
	rm -f *.o
	rm -f mbdbbuilder.exe mbdbbuilder