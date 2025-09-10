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

/*add test encoder to rgb 4 phrase*/
//void getColorFromEncoder(uint8_t step, uint8_t *r, uint8_t *g, uint8_t *b) {
//    step = step % ENC_STEPS;  // 防止越界
//
//    if (step < 10) {
//        // 红 -> 绿
//        *r = MAX_COLOR - (step * MAX_COLOR / 10);
//        *g = step * MAX_COLOR / 10;
//        *b = 0;
//    } else if (step < 20) {
//        // 绿 -> 蓝
//        uint8_t s = step - 10;
//        *r = 0;
//        *g = MAX_COLOR - (s * MAX_COLOR / 10);
//        *b = s * MAX_COLOR / 10;
//    } else if (step < 30) {
//        // 蓝 -> 紫
//        uint8_t s = step - 20;
//        *r = s * MAX_COLOR / 10;  // 使紫色的红色逐渐增加
//        *g = 0;
//        *b = MAX_COLOR - (s * MAX_COLOR / 10);  // 蓝色逐渐减少
//    } else {
//        // 紫 -> 红
//        uint8_t s = step - 30;
//        *r = MAX_COLOR - (s * MAX_COLOR / 10);  // 红色逐渐增加
//        *g = 0;
//        *b = s * MAX_COLOR / 10;  // 紫色逐渐减少到蓝色
//    }
//}

/*add test encoder to rgb 3 phrase 2 rounds version 1*/
//void getColorFromEncoder(uint8_t step, uint8_t *r, uint8_t *g, uint8_t *b) {
//    step = step % ENC_STEPS;  // 防止越界
//
//    if (step < 26) {
//        // 红 -> 绿
//        *r = MAX_COLOR - (step * MAX_COLOR / 26);
//        *g = step * MAX_COLOR / 26;
//        *b = 0;
//    } else if (step < 52) {
//        // 绿 -> 蓝
//        uint8_t s = step - 26;
//        *r = 0;
//        *g = MAX_COLOR - (s * MAX_COLOR / 26);
//        *b = s * MAX_COLOR / 26;
//    } else {
//        // 蓝 -> 紫 -> 红
//        uint8_t s = step - 52;
//        *r = s * MAX_COLOR / 26;  // 使红色逐渐增加
//        *g = 0;
//        *b = MAX_COLOR - (s * MAX_COLOR / 26);  // 蓝色逐渐减少
//
//        // 紫 -> 红，保持过渡
//        if (step >= 66) {
//            uint8_t s2 = step - 66;
//            *r = MAX_COLOR - (s2 * MAX_COLOR / 14);  // 红色逐渐增加
//            *b = s2 * MAX_COLOR / 14;  // 紫色逐渐减少到蓝色
//        }
//    }
//}

/*add test encoder to rgb 3 phrase version 3*/
//void encoder_to_rgb(uint16_t value, uint8_t *r, uint8_t *g, uint8_t *b) {
//    // 将编码器值0-39映射到0-1的色相范围
//    // 假设value是偶数，确保3段分配均匀
//    if (value < 13) {  // 红 -> 绿 (0-12)
//        *r = 255 - (value * 255 / 13);  // 红色逐渐减少
//        *g = value * 255 / 13;  // 绿色逐渐增加
//        *b = 0;  // 蓝色始终为0
//    } else if (value < 26) {  // 绿 -> 蓝 (13-25)
//        uint8_t s = value - 13;
//        *r = 0;  // 红色始终为0
//        *g = 255 - (s * 255 / 13);  // 绿色逐渐减少
//        *b = s * 255 / 13;  // 蓝色逐渐增加
//    } else {  // 蓝 -> 红 (26-39)
//        uint8_t s = value - 26;
//        *r = s * 255 / 13;  // 红色逐渐增加
//        *g = 0;  // 绿色始终为0
//        *b = 255 - (s * 255 / 13);  // 蓝色逐渐减少
//    }
//}


/*add test encoder to rgb 3 phrase version 2*/
//void getColorFromEncoder(uint8_t step, uint8_t *r, uint8_t *g, uint8_t *b) {
//    step = step % ENC_STEPS;  // 防止越界
//
//    if (step < 13) {
//        // 红 -> 绿
//        *r = MAX_COLOR - (step * MAX_COLOR / 13);
//        *g = step * MAX_COLOR / 13;
//        *b = 0;
//    } else if (step < 25) {
//        // 绿 -> 蓝
//        uint8_t s = step - 13;
//        *r = 0;
//        *g = MAX_COLOR - (s * MAX_COLOR / 12);
//        *b = s * MAX_COLOR / 12;
//    } else {
//        // 蓝 -> 紫 -> 红
//        uint8_t s = step - 25;
//        *r = s * MAX_COLOR / 15;  // 使红色逐渐增加
//        *g = 0;
//        *b = MAX_COLOR - (s * MAX_COLOR / 15);  // 蓝色逐渐减少
//
//        // 紫 -> 红，保持过渡
//        if (step >= 33) {
//            uint8_t s2 = step - 33;
//            *r = MAX_COLOR - (s2 * MAX_COLOR / 7);  // 红色增加
//            *b = s2 * MAX_COLOR / 7;  // 紫色逐渐减少到蓝色
//        }
//    }
//}

/*Selected add test encoder to rgb 3 phrase version 1*/
//void getColorFromEncoder(uint8_t step, uint8_t *r, uint8_t *g, uint8_t *b) {
//    step = step % ENC_STEPS; // 防止越界
//
//    if (step < 13) {
//        // 红 -> 绿
//        *r = MAX_COLOR - (step * MAX_COLOR / 13);
//        *g = step * MAX_COLOR / 13;
//        *b = 0;
//    } else if (step < 26) {
//        // 绿 -> 蓝
//        uint8_t s = step - 13;
//        *r = 0;
//        *g = MAX_COLOR - (s * MAX_COLOR / 13);
//        *b = s * MAX_COLOR / 13;
//    } else {
//        // 蓝 -> 红
//        uint8_t s = step - 26;
//        *r = s * MAX_COLOR / 14;
//        *g = 0;
//        *b = MAX_COLOR - (s * MAX_COLOR / 14);
//    }
//}

/*test hsv add encoder to rgb*/
/*modified version 2*/
//void encoder_to_rgb(uint16_t value, uint8_t *r, uint8_t *g, uint8_t *b) {
//    // 将编码器值0-39映射到0-1的色相范围
//    float h = (float)value / 40.0f;  // 0-39 映射到 0-1
//
//    // HSV转RGB
//    int i = h * 3;  // 0 到 2 之间的整数，用于选择颜色区间
//    float f = h * 3 - i;  // 色相在区间内的相对偏移
//    float p = 0.0f;  // s=1, v=1
//    float q = 1.0f - f;
//    float t = f;
//
//    // 修正：确保蓝色最亮，且蓝到红过渡顺畅
//    switch (i) {
//        case 0:
//            *r = 255;
//            *g = t * 255;
//            *b = p * 255;
//            break; // 红 -> 绿
//        case 1:
//            *r = q * 255;
//            *g = 255;
//            *b = p * 255;
//            break; // 绿 -> 蓝
//        case 2:
//            *r = p * 255;
//            *g = q * 255;
//            *b = 255;
//            break; // 蓝 -> 红
//
//        // 如果f接近蓝色接缝时，确保过渡平滑，特别是蓝色最亮部分
//        default:
//            *r = 255;
//            *g = 0;
//            *b = 255;  // 确保过渡区域平滑
//            break;
//    }
//}


/*modified veriosn 1*/
//void encoder_to_rgb(uint16_t value, uint8_t *r, uint8_t *g, uint8_t *b) {
//    // 将编码器值0-39映射到0-1的色相范围
//    float h = (float)(value % 40) / 40.0f;  // 0-39 映射到 0-1
//
//    // HSV转RGB
//    int i = h * 3;  // 0 到 2 之间的整数，用于选择颜色区间
//    float f = h * 3 - i;  // 色相在区间内的相对偏移
//    float p = 0.0f;  // s=1, v=1
//    float q = 1.0f - f;
//    float t = f;
//
//    // 修正：避免蓝色到红色时出现意外的绿色
//    if (i == 2 && f > 0.9) {  // 临近边界值时避免出现绿色
//        *r = 255;
//        *g = 0;
//        *b = 0;
//    } else {
//        switch (i) {
//            case 0: *r = 255; *g = t * 255; *b = p * 255; break; // 红 -> 绿
//            case 1: *r = q * 255; *g = 255; *b = p * 255; break; // 绿 -> 蓝
//            case 2: *r = p * 255; *g = q * 255; *b = 255; break; // 蓝 -> 红
//        }
//    }
//}

/*0-40*/
//void encoder_to_rgb(uint16_t value, uint8_t *r, uint8_t *g, uint8_t *b) {
//    // 将编码器值0-39映射到0-1的色相范围
//    float h = (float)(value % 40) / 40.0f;  // 0-39 映射到 0-1
//
//    // HSV转RGB
//    int i = h * 3;  // 0 到 2 之间的整数，用于选择颜色区间
//    float f = h * 3 - i;  // 色相在区间内的相对偏移
//    float p = 0.0f;  // s=1, v=1
//    float q = 1.0f - f;
//    float t = f;
//
//    switch (i) {
//        case 0: *r = 255; *g = t * 255; *b = p * 255; break; // 红 -> 绿
//        case 1: *r = q * 255; *g = 255; *b = p * 255; break; // 绿 -> 蓝
//        case 2: *r = p * 255; *g = q * 255; *b = 255; break; // 蓝 -> 红
//    }
//}

/* 0-65536*/
//void encoder_to_rgb(uint16_t value, uint8_t *r, uint8_t *g, uint8_t *b) {
//    // 关键修改：将编码器值直接映射到0-1的色相范围
//    float h = (float)value / 65535.0f;  // 0-65535 映射到 0-1
//
//    // HSV转RGB
//    int i = h * 6;
//    float f = h * 6 - i;
//    float p = 1.0f * (1 - 1.0f);  // s=1, v=1
//    float q = 1.0f * (1 - f * 1.0f);
//    float t = 1.0f * (1 - (1 - f) * 1.0f);
//
//    switch (i % 6) {
//        case 0: *r = 255; *g = t * 255; *b = p * 255; break; // 红到黄
//        case 1: *r = q * 255; *g = 255; *b = p * 255; break; // 黄到绿
//        case 2: *r = p * 255; *g = 255; *b = t * 255; break; // 绿到青
//        case 3: *r = p * 255; *g = q * 255; *b = 255; break; // 青到蓝
//        case 4: *r = t * 255; *g = p * 255; *b = 255; break; // 蓝到紫
//        case 5: *r = 255; *g = p * 255; *b = q * 255; break; // 紫到红
//    }
//}
