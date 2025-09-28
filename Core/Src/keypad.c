#include "main.h"
#include "usb_device.h"
#include "usbd_hid.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

// 配置参数
#define LONG_PRESS_DELAY 500    // 长按判定时间(ms)
#define REPEAT_INTERVAL 100     // 长按重复间隔(ms)
#define DEBOUNCE_TIME 20        // 消抖时间(ms)
#define SCAN_INTERVAL 2         // 扫描间隔(ms)

typedef struct {
    uint8_t MODIFIER;
    uint8_t RESERVED;
    uint8_t KEYCODE1;
    uint8_t KEYCODE2;
    uint8_t KEYCODE3;
    uint8_t KEYCODE4;
    uint8_t KEYCODE5;
    uint8_t KEYCODE6;
} keyboardReportDes;

keyboardReportDes HIDkeyBoard = {0,0,0,0,0,0,0,0};

uint8_t keypad[5][4] = {
    {0x2A, 0x54, 0x55, 0x00}, {0x5F, 0x60, 0x61, 0x56},
    {0x5C, 0x5D, 0x5E, 0x57}, {0x59, 0x5A, 0x5B, 0x00},
    {0x62, 0x63, 0x58, 0x00}
};

// 键盘状态结构
typedef struct {
    uint8_t keyState[5][4];           // 当前按键状态
    uint8_t keyPressed[5][4];         // 按键已处理标志
    uint32_t press_time[5][4];        // 按键按下时间
    uint8_t long_press_flag[5][4];    // 长按标志
    uint32_t last_repeat_time[5][4];  // 上次重复时间
    uint32_t debounce_time[5][4];     // 消抖时间
    uint8_t current_row;              // 当前扫描行
    uint32_t last_scan_time;          // 上次扫描时间
} Keypad_State;

Keypad_State keypad_state = {0};

void KEYPAD_Init(void) {
    // 初始化所有行为高电平
    HAL_GPIO_WritePin(GPIOB, ROW0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, ROW1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, ROW2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, ROW3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, ROW4_Pin, GPIO_PIN_SET);
}

void ScanKeypad(void) {
    uint32_t current_time = HAL_GetTick();

    // 控制扫描频率
    if (current_time - keypad_state.last_scan_time < SCAN_INTERVAL) {
        return;
    }
    keypad_state.last_scan_time = current_time;

    // 1. 先将所有行设置为高电平
    HAL_GPIO_WritePin(GPIOB, ROW0_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, ROW1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, ROW2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, ROW3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, ROW4_Pin, GPIO_PIN_SET);

    // 2. 将当前扫描行设置为低电平
    switch (keypad_state.current_row) {
        case 0: HAL_GPIO_WritePin(GPIOB, ROW0_Pin, GPIO_PIN_RESET); break;
        case 1: HAL_GPIO_WritePin(GPIOB, ROW1_Pin, GPIO_PIN_RESET); break;
        case 2: HAL_GPIO_WritePin(GPIOA, ROW2_Pin, GPIO_PIN_RESET); break;
        case 3: HAL_GPIO_WritePin(GPIOA, ROW3_Pin, GPIO_PIN_RESET); break;
        case 4: HAL_GPIO_WritePin(GPIOA, ROW4_Pin, GPIO_PIN_RESET); break;
    }

    // 3. 扫描当前行的所有列
    for (int col = 0; col < 4; col++) {
        uint8_t key_detected = 0;

        // 读取列状态
        switch(col) {
            case 0: key_detected = (HAL_GPIO_ReadPin(GPIOB, COL0_Pin) == GPIO_PIN_RESET); break;
            case 1: key_detected = (HAL_GPIO_ReadPin(GPIOB, COL1_Pin) == GPIO_PIN_RESET); break;
            case 2: key_detected = (HAL_GPIO_ReadPin(GPIOB, COL2_Pin) == GPIO_PIN_RESET); break;
            case 3: key_detected = (HAL_GPIO_ReadPin(GPIOA, COL3_Pin) == GPIO_PIN_RESET); break;
        }

        int row = keypad_state.current_row;

        // 按键按下检测
        if (key_detected) {
            if (keypad_state.keyState[row][col] == 0) {
                // 第一次检测到按下，记录时间
                keypad_state.keyState[row][col] = 1;
                keypad_state.debounce_time[row][col] = current_time;
            }

            // 消抖确认
            if (current_time - keypad_state.debounce_time[row][col] > DEBOUNCE_TIME) {
                if (!keypad_state.keyPressed[row][col]) {
                    // 首次按下
                    keypad_state.keyPressed[row][col] = 1;
                    keypad_state.press_time[row][col] = current_time;
                    keypad_state.long_press_flag[row][col] = 0;

                    // 发送按键按下报告
                    HIDkeyBoard.KEYCODE1 = keypad[row][col];
                    USBD_HID_SendReport(&hUsbDeviceFS, &HIDkeyBoard, sizeof(HIDkeyBoard));
                } else {
                    // 按键保持按下，检查长按
                    if (!keypad_state.long_press_flag[row][col]) {
                        // 检查是否达到长按时间
                        if ((current_time - keypad_state.press_time[row][col]) > LONG_PRESS_DELAY) {
                            keypad_state.long_press_flag[row][col] = 1;
                            keypad_state.last_repeat_time[row][col] = current_time;
                        }
                    } else {
                        // 长按重复发送
                        if ((current_time - keypad_state.last_repeat_time[row][col]) > REPEAT_INTERVAL) {
                            keypad_state.last_repeat_time[row][col] = current_time;
                            HIDkeyBoard.KEYCODE1 = keypad[row][col];
                            USBD_HID_SendReport(&hUsbDeviceFS, &HIDkeyBoard, sizeof(HIDkeyBoard));
                        }
                    }
                }
            }
        } else {
            // 按键释放
            if (keypad_state.keyState[row][col] == 1) {
                keypad_state.keyState[row][col] = 0;
                keypad_state.keyPressed[row][col] = 0;
                keypad_state.long_press_flag[row][col] = 0;

                // 发送释放报告
                HIDkeyBoard.KEYCODE1 = 0x00;
                USBD_HID_SendReport(&hUsbDeviceFS, &HIDkeyBoard, sizeof(HIDkeyBoard));
            }
        }
    }

    // 4. 移动到下一行
    keypad_state.current_row = (keypad_state.current_row + 1) % 5;
}


