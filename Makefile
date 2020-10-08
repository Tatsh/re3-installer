CFLAGS ?= -ggdb -O0 -Wall -Wdeprecated-declarations -Wempty-body -Werror -Werror=float-conversion -Werror=implicit-function-declaration -Werror=incompatible-pointer-types -Werror=int-conversion -Werror=return-type -Wmissing-braces -Wmissing-field-initializers -Wno-missing-prototypes -Wno-strict-prototypes -Wno-trigraphs -Wno-unknown-pragmas -Wparentheses -Wpointer-sign -Wshadow -Wswitch -Wuninitialized -Wunknown-pragmas -Wunreachable-code -Wunused-function -Wunused-label -Wunused-parameter -Wunused-value -Wunused-variable -Wwrite-strings
INCLUDEDIRS ?= -I/opt/local/include
LDFLAGS ?= -L/opt/local/lib
XDG ?= -DXDG_ROOT

re3-installer: main.c
	$(CC) $(CFLAGS) $(INCLUDEDIRS) $(XDG) main.c -o re3-installer $(LDFLAGS) -lunshield

clean:
	rm -fR re3-installer re3-installer.dSYM/
