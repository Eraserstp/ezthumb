
include Make.conf


INCDIR	= $(EXINC) -I./libsmm
LIBDIR	= $(EXLIB) -L./libsmm

CFLAGS	+= -D_FILE_OFFSET_BITS=64 $(INCDIR) `pkg-config gtk+-2.0 --cflags --libs`

OBJS	= main.o fixtoken.o ezthumb.o cliopt.o eznotify.o id_lookup.o ezgui.o

LIBS	= -lavcodec -lavformat -lavcodec -lswscale -lavutil -lgd
#	 -lfreetype -lbz2 -lm


ifeq	($(SYSTOOL),unix)
	TARGET	= ezthumb
else
	TARGET	= ezthumb.exe
endif

all: smm $(TARGET) install

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LIBDIR) -o $@ $(OBJS) $(LIBS) -lsmm

smm:
	make -C libsmm

vidlen : vidlen.c
	$(CC) -o $@ $^ $(CFLAGS) -lavformat

install:
	$(CP) $(TARGET) ~/bin

clean:
	make -C libsmm clean
	$(RM) $(TARGET) $(OBJS)

release:
	mkdir $(RELDIR)
	$(CP) $(TARGET) ezthumb.1 $(RELDIR)
ifeq	($(SYSTOOL),cygwin)
	$(CP) $(EXDLL) $(RELDIR)
endif
ifeq	($(SYSTOOL),mingw)
	$(CP) $(EXDLL) $(RELDIR)
endif

