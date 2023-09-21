#ifndef RAM2GS_HAL_H
#define RAM2GS_HAL_H
#include <ctype.h>
#include <stdint.h>

uint8_t __fastcall__ ram2gs_detect(char typecode);
uint8_t __fastcall__ ram2gs_getsize(void);
uint8_t __fastcall__ ram2gs_flashled1(void);

void ram2gs_set(char en8meg, char enled);

void ram2gs_flashled(char frames);

void ram2gs_hal_set_type(char typecode);
void ram2gs_erase();
void ram2gs_save_start(char en8meg, char enled);
void ram2gs_save_end(char en8meg, char enled);

#endif