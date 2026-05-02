#ifndef __CONFIG__
#define __CONFIG__
#define USER_NAME_MAX       32
#define CONFIG_ADRES        0x1000
#define CONFIG_USER_OFFSET  0x1002
#define CONFIG_PASS_OFFSET  0x1022

#include <stdint.h>
#include "../services/Memory/memory.h"

extern uint8_t user_name[32];
extern uint8_t user_pass[32];
extern void init_config();
#endif
