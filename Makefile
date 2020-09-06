cflags = -O --cpu 6502 -t apple2

all:    GWRAM.po GWRAM.dbg.po

obj:
	@mkdir obj

bin:
	@mkdir bin

obj/main.o: obj main.c
	cc65 main.c $(cflags) -o obj/main.s
	ca65 obj/main.s -o obj/main.o

obj/ram2e.o: obj ram2e.c
	cc65 ram2e.c $(cflags) -o obj/ram2e.s
	ca65 obj/ram2e.s -o obj/ram2e.o

obj/ram2e.dbg.o: obj ram2e.c
	cc65 ram2e.c $(cflags) -o obj/ram2e.dbg.s -DSKIP_RAM2E_DETECT
	ca65 obj/ram2e.dbg.s -o obj/ram2e.dbg.o

obj/ram2gs_asm.o: obj ram2gs_asm.s
	ca65 ram2gs_asm.s -o obj/ram2gs_asm.o

obj/ram2gs.o: obj ram2gs.c
	cc65 ram2gs.c $(cflags) -o obj/ram2gs.s
	ca65 obj/ram2gs.s -o obj/ram2gs.o

obj/ram2gs.dbg.o: obj ram2gs.c
	cc65 ram2gs.c $(cflags) -o obj/ram2gs.dbg.s -DSKIP_RAM2GS_DETECT
	ca65 obj/ram2gs.dbg.s -o obj/ram2gs.dbg.o

obj/util.o: obj util.c
	cc65 util.c $(cflags) -o obj/util.s
	ca65 obj/util.s -o obj/util.o

obj/gwconio.o: obj gwconio.s
	ca65 gwconio.s -o obj/gwconio.o

bin/main.sys: bin obj/main.o obj/ram2e.o obj/ram2gs.o obj/ram2gs_asm.o obj/util.o obj/gwconio.o
	ld65 -o bin/main.sys obj/main.o obj/ram2gs.o obj/ram2e.o obj/ram2gs_asm.o obj/util.o obj/gwconio.o -C apple2-system.cfg --lib apple2.lib -D __EXEHDR__=0

bin/main.dbg.sys: bin obj/main.o obj/ram2e.dbg.o obj/ram2gs.dbg.o obj/ram2gs_asm.o obj/util.o obj/gwconio.o
	ld65 -o bin/main.dbg.sys obj/main.o obj/ram2gs.dbg.o obj/ram2e.dbg.o obj/ram2gs_asm.o obj/util.o obj/gwconio.o -C apple2-system.cfg --lib apple2.lib -D __EXEHDR__=0

GWRAM.po: bin/main.sys
	cp prodos140.po bin/GWRAM.po
	cat bin/main.sys | java -jar ./AppleCommander-ac-1.6.0.jar -p bin/GWRAM.po gwram.system sys 0x2000

GWRAM.dbg.po: bin/main.dbg.sys
	cp prodos140.po bin/GWRAM.dbg.po
	cat bin/main.dbg.sys | java -jar ./AppleCommander-ac-1.6.0.jar -p bin/GWRAM.dbg.po gwram.system sys 0x2000

.PHONY: clean
clean:
	rm -fr bin obj
