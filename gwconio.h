#ifndef GWCONIO_H
#define GWCONIO_H

void gwcputsxy (unsigned char x, unsigned char y, const char* s);
void gwcputs (const char* s);
void __fastcall__ gwcputcxy (unsigned char x, unsigned char y, char c);
void __fastcall__ gwcputc (char c);

#endif /* GWCONIO_H */
