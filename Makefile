default: lispy

CPPFLAGS = -Wall
CFLAGS = -std=c99 -ledit
LDFLAGS = -s
RM = rm -f

builtin.o: builtin.c
	$(COMPILE.c) builtin.c lispy.h
lval.o: lval.c
	$(COMPILE.c) lval.c
lenv.o: lenv.c
	$(COMPILE.c) lenv.c
lispy: mpc.c builtin.o lval.o lenv.o lispy.c 
	$(LINK.c) mpc.c builtin.o lval.o lenv.o lispy.c -o lispy -lreadline -lm

clean:
	$(RM) *.o
	$(RM) lispy
