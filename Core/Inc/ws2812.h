#ifndef INC_WS2812_H_
#define INC_WS2812_H_

#include "tim.h"

#define LED_COUNT 17

extern uint8_t color[LED_COUNT][3];

/*test encoder to rgb*/
#define MAX_COLOR 255
#define ENC_STEPS 40  // 编码器一圈40步

void WS2812_Set(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
void WS2812_SetAll(uint8_t r, uint8_t g, uint8_t b);
void WS2812_Update();
void getColorFromEncoder(uint8_t step, uint8_t *r, uint8_t *g, uint8_t *b);
//void encoder_to_rgb(uint16_t value, uint8_t *r, uint8_t *g, uint8_t *b);

#endif /* INC_WS2812_H_ */
