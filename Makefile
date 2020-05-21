all:    ram2e140.po

ram2e140.po: ram2e.sys
	cp prodos140.po ram2e140.po
	cat ram2e.sys | java -jar ./AppleCommander-ac-1.6.0.jar -p ram2e140.po ram2e.system sys 0x2000

ram2e.sys: ram2e.o
	ld65 -o ram2e.sys ram2e.o -C apple2enh-system.cfg --lib apple2enh.lib -D __EXEHDR__=0

ram2e.o: ram2e.c
	cc65 ram2e.c -O --cpu 6502 -t apple2enh -o ram2e.s
	ca65 ram2e.s

ram2e.c:
	
.PHONY: clean
clean:
	rm -f ram2e.s *.o *.sys ram2e140.po
