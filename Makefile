INCS = $(shell pkg-config --cflags libmnl)
LIBS = $(shell pkg-config --libs libmnl)
CFLAGS = -ggdb
CC = /home/lakkarajun/mscc-ent/SAMA5D3_EDS/Yuiko/buildroot/output/host/bin/arm-buildroot-linux-uclibcgnueabihf-gcc 
LDFLAGS = 

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $(INCS) $<

nl-app: nl-app.o
	$(CC) -o $@ $< $(LIBS)

clean:
	/bin/rm -f *.o
	/bin/rm -f nl-app
