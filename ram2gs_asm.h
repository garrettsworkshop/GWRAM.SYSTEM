#ifndef RAM2GS_ASM_H
#define RAM2GS_ASM_H

uint8_t __fastcall__ ram2gs_cmd(char cmd);
uint8_t __fastcall__ ram2gs_detect(char typecode);
uint8_t __fastcall__ ram2gs_getsize(void);

#endif /* RAM2GS_ASM_H */
