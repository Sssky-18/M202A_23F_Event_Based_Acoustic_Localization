#if defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif
// 24 x 24 gridicons_types
const unsigned char gridicons_types[] PROGMEM = { /* 0X01,0X01,0XB4,0X00,0X40,0X00, */
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
0x3F, 0xFF, 0xFE, 0x1F, 0xFF, 0xFE, 0x1F, 0xFF, 
0xFC, 0x0F, 0xFF, 0xFD, 0x0F, 0xFF, 0xFF, 0xC7, 
0xFF, 0x00, 0x83, 0xFF, 0x00, 0xC3, 0xFF, 0x00, 
0x81, 0xFF, 0xC0, 0xFF, 0xF8, 0x70, 0xFF, 0xE0, 
0x10, 0xFF, 0xE0, 0x18, 0xFF, 0xC0, 0x08, 0xFF, 
0xC0, 0x0F, 0xFF, 0xC0, 0x0F, 0xFF, 0xC0, 0x0F, 
0xFF, 0xE0, 0x1F, 0xFF, 0xE0, 0x1F, 0xFF, 0xF8, 
0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
