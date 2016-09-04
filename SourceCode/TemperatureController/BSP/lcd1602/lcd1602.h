#ifndef __LCD1602_H__
#define __LCD1602_H__
#include "stm32f10x.h"

void lcd1602_init(void);
void lcd1602_write_cmd(uint8_t cmd);
void lcd1602_write_data(uint8_t data);
void lcd1602_write_char(uint8_t x, uint8_t y, char c);
void lcd1602_write_string(uint8_t x, uint8_t y, char *str);
void lcd1602_clear(void);

#endif
