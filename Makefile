CFLAGS ?= -ggdb -O0 -Wall -Wdeprecated-declarations -Wempty-body -Werror -Werror=float-conversion -Werror=implicit-function-declaration -Werror=incompatible-pointer-types -Werror=int-conversion -Werror=return-type -Wmissing-braces -Wmissing-field-initializers -Wno-missing-prototypes -Wno-strict-prototypes -Wno-trigraphs -Wno-unknown-pragmas -Wparentheses -Wpointer-sign -Wshadow -Wswitch -Wuninitialized -Wunknown-pragmas -Wunreachable-code -Wunused-function -Wunused-label -Wunused-parameter -Wunused-value -Wunused-variable -Wwrite-strings
INCLUDEDIRS ?= -I/opt/local/include
LDFLAGS ?= -L/opt/local/lib
XDG ?= -DXDG_ROOT

SRC = $(wildcard *.c) 
OBJS = $(patsubst %.c, %.o, $(SRC))

all: re3-installer

re3-installer: $(OBJS)
	$(CC) $(OBJS) -o re3-installer $(LDFLAGS) -lunshield

%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDEDIRS) $(XDG) $< -o $@

clean:
	rm -fR *.o re3-installer re3-installer.dSYM/
