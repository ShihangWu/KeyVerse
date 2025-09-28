#include "ws2812.h"
#include <stdint.h>

#define Code0			30
#define Code1			60
#define CodeReset		0

uint8_t color[LED_COUNT][3];

void WS2812_Set(uint8_t index, uint8_t r, uint8_t g, uint8_t b){
	color[index][0] = r;
	color[index][1] = g;
	color[index][2] = b;
}

void WS2812_SetAll(uint8_t r, uint8_t g, uint8_t b){
	for  (uint8_t i = 0; i < LED_COUNT; i++){
		WS2812_Set(i, r, g, b);
	}
}

void WS2812_Update(){
	static uint16_t data[LED_COUNT * 3 * 8  +  1];

	for (int i = 0; i  < LED_COUNT; i++){
		uint8_t r = color[i][0];
		uint8_t g = color[i][1];
		uint8_t b = color[i][2];

		for (int j = 0; j < 8; j++){
			data[24 * i + j] = (g & (0x80 >> j)) ? Code1 : Code0;
			data[24 * i + 8 + j] = (r & (0x80 >> j)) ? Code1 : Code0;
			data[24 * i + 16 + j] = (b & (0x80 >> j)) ? Code1 : Code0;
		}
	}
	data[LED_COUNT * 3] = CodeReset;

	HAL_TIM_PWM_Stop_DMA(&htim3, TIM_CHANNEL_1);
	__HAL_TIM_SetCounter(&htim3, 0);
	HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_1, (uint32_t *)data, sizeof(data)/sizeof(uint16_t));
}

/*Selected add test encoder to rgb 3 phrase version 1*/
void getColorFromEncoder(uint8_t step, uint8_t *r, uint8_t *g, uint8_t *b) {
    step = step % ENC_STEPS; // 防止越界

    if (step < 13) {
        // 红 -> 绿
        *r = MAX_COLOR - (step * MAX_COLOR / 13);
        *g = step * MAX_COLOR / 13;
        *b = 0;
    } else if (step < 26) {
        // 绿 -> 蓝
        uint8_t s = step - 13;
        *r = 0;
        *g = MAX_COLOR - (s * MAX_COLOR / 13);
        *b = s * MAX_COLOR / 13;
    } else {
        // 蓝 -> 红
        uint8_t s = step - 26;
        *r = s * MAX_COLOR / 14;
        *g = 0;
        *b = MAX_COLOR - (s * MAX_COLOR / 14);
    }
}


