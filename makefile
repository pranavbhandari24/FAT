CXX=gcc

all: mfs

mfs: mfs.c
	${CXX} -o mfs mfs.c

clean:
	rm mfs