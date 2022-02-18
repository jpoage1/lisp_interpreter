default: lispy

CPPFLAGS = -Wall
CFLAGS = -std=c99 -ledit -lm
LDFLAGS = -s
RM = rm -f

builtin.o: builtin.c
	$(COMPILE.c) builtin.c lispy.h
bar.o: bar.c header.h
	$(COMPILE.c) bar.c
baz.o: baz.c header.h
	$(COMPILE.c) baz.c
lispy: lispy.c mpc.c
	$(LINK.c) lispy.c mpc.c -o lispy

clean:
	$(RM) *.o
	$(RM) lispy
