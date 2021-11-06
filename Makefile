LIBS=
tt2bin: tt2bin.c 
	cc -o $@ $^ ${LIBS}
