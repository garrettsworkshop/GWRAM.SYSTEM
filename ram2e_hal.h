#ifndef RAM2E_HAL_H
#define RAM2E_HAL_H
#include <ctype.h>
#include <stdint.h>

char auxram_detect();
char ram2e_detect(char type);
uint16_t ramworks_getsize();
void ram2e_set_mask(char mask);
void ram2e_set_led(char enled);
void ram2e_flashled(char frames);

void ram2e_hal_set_type(char type);
void ram2e_erase();
void ram2e_save_start(char mask, char enled);
void ram2e_save_end(char mask, char enled);

#endif