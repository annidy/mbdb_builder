CCFLAGS=-std=c++0x
CFLAGS=
LDFLAGS=-lwsock32
OBJS=main.o mbdb_record.o mbdbdump.o sha1.o mman.o

.PHONY : clean

mbdb_builder: $(OBJS)
	g++ -o $@ $(OBJS) $(LDFLAGS) 

main.o: mbdb_builder/main.cpp
	g++ -c $< $(CCFLAGS)
mbdb_record.o: mbdb_builder/mbdb_record.cpp
	g++ -c $< $(CCFLAGS)
mbdbdump.o: mbdb_builder/mbdbdump.cpp
	g++ -c $< $(CCFLAGS)
sha1.o: mbdb_builder/sha1.cpp
	g++ -c $< $(CCFLAGS)
mman.o: mbdb_builder/mman.c
	gcc -c $< $(CFLAGS)

clean:
	rm *.o