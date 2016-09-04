#ifndef __APP_H__
#define __APP_H__

#define CMD_START                       0x01
#define CMD_STOP                        0x02
#define CMD_SETTING                     0x03
#define CMD_GET_TEMPERATURE             0x10

#define CMD_SET_SSR1                    0x20
#define CMD_SET_SSR2                    0x21
#define CMD_GET_SSR1                    0x22
#define CMD_GET_SSR2                    0x23

#define CMD_SET_FAN1                    0x30
#define CMD_SET_FAN2                    0x31

typedef struct __AppCmd {
    u8 type;
    u8 len;
    u8 data[40];
} AppCmd;

void set_ssr1(double ssr);
void set_ssr2(double ssr);

#endif
