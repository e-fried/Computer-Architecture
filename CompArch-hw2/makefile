CC = g++
PROG_OBJS = Cache.o MemorySys.o cacheSim.o
PROG = cacheSim


DEBUG_FLAG =  -g
COMP_FLAG = -std=c++11 -Wall -DNDEBUG

##################### main: Two Level Cache simulator
$(PROG) : $(PROG_OBJS)
	$(CC) $(DEBUG_FLAG) $(PROG_OBJS) -o $@


##################### Two Level Cache objects

Cache.o: Cache.cpp Cache.h TagUnit.h
MemorySys.o: MemorySys.cpp MemorySys.h Cache.h TagUnit.h VictimCache.h
cacheSim.o: cacheSim.cpp MemorySys.h Cache.h TagUnit.h VictimCache.h


clean:
	rm -f $(PROG) $(PROG_OBJS)
