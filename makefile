CCFLAGS=-std=c++0x
CFLAGS=
LDFLAGS=-lwsock32
OBJS=main.o mbdb_record.o mbdbdump.o sha1.o mman.o

.PHONY : clean

mbdb_builder: $(OBJS)
	g++ -g -o $@ $(OBJS) $(LDFLAGS) 

main.o: mbdb_builder/main.cpp
	g++ -g -c $< $(CCFLAGS)
mbdb_record.o: mbdb_builder/mbdb_record.cpp
	g++ -g -c $< $(CCFLAGS)
mbdbdump.o: mbdb_builder/mbdbdump.cpp
	g++ -g -c $< $(CCFLAGS)
sha1.o: mbdb_builder/sha1.cpp
	g++ -g -c $< $(CCFLAGS)
mman.o: mbdb_builder/mman.c
	gcc -g -c $< $(CFLAGS)

clean:
	rm -f *.o
	rm -f mbdb_builder.exe