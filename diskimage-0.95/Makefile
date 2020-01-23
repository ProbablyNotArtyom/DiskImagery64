CC=gcc
CFLAGS=-g -Wall


%.o: %c
	$(CC) $(CFLAGS) -c $<


all: test_bam test_dir test_read test_write test_format

test_bam: test_bam.o diskimage.o
	$(CC) -o test_bam $^ $(LDFLAGS)

test_dir: test_dir.o diskimage.o
	$(CC) -o test_dir $^ $(LDFLAGS)

test_read: test_read.o diskimage.o
	$(CC) -o test_read $^ $(LDFLAGS)

test_write: test_write.o diskimage.o
	$(CC) -o test_write $^ $(LDFLAGS)

test_format: test_format.o diskimage.o
	$(CC) -o test_format $^ $(LDFLAGS)


clean:
	rm -f test_bam test_dir test_read test_write test_format
	rm -f *.o *~ test_*.exe
