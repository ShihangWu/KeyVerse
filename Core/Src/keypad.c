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


//

///*
// * keypad.c
// *
// *  Created on: Aug 22, 2025
// *      Author: ShihangWu
// */
//#include "main.h"
//#include "usb_device.h"
//#include "usbd_hid.h"  // 确保包含 HID 相关的头文件
//
//extern USBD_HandleTypeDef hUsbDeviceFS;
//
//typedef struct {
//	uint8_t MODIFIER;
//	uint8_t RESERVED;
//	uint8_t KEYCODE1;
//	uint8_t KEYCODE2;
//	uint8_t KEYCODE3;
//	uint8_t KEYCODE4;
//	uint8_t KEYCODE5;
//	uint8_t KEYCODE6;
//}keyboardReportDes;
//
//keyboardReportDes HIDkeyBoard={0,0,0,0,0,0,0,0};
//
//// 假设矩阵键值如下
//uint8_t keypad[5][4] = {
//    {0x2A, 0x54, 0x55, 0x00},  // 第一行
//    {0x5F, 0x60, 0x61, 0x56},  // 第二行
//    {0x5C, 0x5D, 0x5E, 0x57},  // 第三行
//    {0x59, 0x5A, 0x5B, 0x00},  // 第四行
//    {0x62, 0x63, 0x58, 0x00}   // 第五行
//};
//
//uint8_t KeyNum;
//
//void ScanKeypad(void) {
//	static uint8_t keyState[5][4] = {0};  // 按键状态缓存
//	static uint8_t keyPressed[5][4] = {0}; // 按键是否已经处理过
//
//	for (int row = 0; row < 5; row++) {
//    // 先将所有行设置为高电平
//    HAL_GPIO_WritePin(GPIOB, ROW0_Pin, GPIO_PIN_SET);
//    HAL_GPIO_WritePin(GPIOB, ROW1_Pin, GPIO_PIN_SET);
//    HAL_GPIO_WritePin(GPIOA, ROW2_Pin, GPIO_PIN_SET);
//    HAL_GPIO_WritePin(GPIOA, ROW3_Pin, GPIO_PIN_SET);
//    HAL_GPIO_WritePin(GPIOA, ROW4_Pin, GPIO_PIN_SET);
//
//    // 将当前行设置为低电平
//    if (row == 0) HAL_GPIO_WritePin(GPIOB, ROW0_Pin, GPIO_PIN_RESET);
//    if (row == 1) HAL_GPIO_WritePin(GPIOB, ROW1_Pin, GPIO_PIN_RESET);
//    if (row == 2) HAL_GPIO_WritePin(GPIOA, ROW2_Pin, GPIO_PIN_RESET);
//    if (row == 3) HAL_GPIO_WritePin(GPIOA, ROW3_Pin, GPIO_PIN_RESET);
//    if (row == 4) HAL_GPIO_WritePin(GPIOA, ROW4_Pin, GPIO_PIN_RESET);
//
//    // 消抖延迟（10ms），确保按键稳定
//    HAL_Delay(10);
//
//    // 扫描列
//    for (int col = 0; col < 4; col++) {
//        if (col == 0 && HAL_GPIO_ReadPin(GPIOB, COL0_Pin) == GPIO_PIN_RESET && keyState[row][col] == 0) {
//            keyState[row][col] = 1;  // 按下了键
//            if (keyPressed[row][col] == 0) {
//                KeyNum = keypad[row][col];  // 设置键码
//                HIDkeyBoard.KEYCODE1 = KeyNum;
//                USBD_HID_SendReport(&hUsbDeviceFS, &HIDkeyBoard, sizeof(HIDkeyBoard));
//                keyPressed[row][col] = 1;  // 标记该按键已经处理过
//            }
//        }
//        if (col == 1 && HAL_GPIO_ReadPin(GPIOB, COL1_Pin) == GPIO_PIN_RESET && keyState[row][col] == 0) {
//            keyState[row][col] = 1;
//            if (keyPressed[row][col] == 0) {
//                KeyNum = keypad[row][col];
//                HIDkeyBoard.KEYCODE1 = KeyNum;
//                USBD_HID_SendReport(&hUsbDeviceFS, &HIDkeyBoard, sizeof(HIDkeyBoard));
//                keyPressed[row][col] = 1;
//            }
//        }
//        if (col == 2 && HAL_GPIO_ReadPin(GPIOB, COL2_Pin) == GPIO_PIN_RESET && keyState[row][col] == 0) {
//            keyState[row][col] = 1;
//            if (keyPressed[row][col] == 0) {
//                KeyNum = keypad[row][col];
//                HIDkeyBoard.KEYCODE1 = KeyNum;
//                USBD_HID_SendReport(&hUsbDeviceFS, &HIDkeyBoard, sizeof(HIDkeyBoard));
//                keyPressed[row][col] = 1;
//            }
//        }
//        if (col == 3 && HAL_GPIO_ReadPin(GPIOA, COL3_Pin) == GPIO_PIN_RESET && keyState[row][col] == 0) {
//            keyState[row][col] = 1;
//            if (keyPressed[row][col] == 0) {
//                KeyNum = keypad[row][col];
//                HIDkeyBoard.KEYCODE1 = KeyNum;
//                USBD_HID_SendReport(&hUsbDeviceFS, &HIDkeyBoard, sizeof(HIDkeyBoard));
//                keyPressed[row][col] = 1;
//            }
//        }
//    }
//
//    // 按键松开后重置状态
//    for (int col = 0; col < 4; col++) {
//        if (HAL_GPIO_ReadPin(GPIOB, COL0_Pin) == GPIO_PIN_SET && keyState[row][0] == 1) {
//            keyState[row][0] = 0;
//            keyPressed[row][0] = 0;  // 重置已处理标志
//        }
//        if (HAL_GPIO_ReadPin(GPIOB, COL1_Pin) == GPIO_PIN_SET && keyState[row][1] == 1) {
//            keyState[row][1] = 0;
//            keyPressed[row][1] = 0;
//        }
//        if (HAL_GPIO_ReadPin(GPIOB, COL2_Pin) == GPIO_PIN_SET && keyState[row][2] == 1) {
//            keyState[row][2] = 0;
//            keyPressed[row][2] = 0;
//        }
//        if (HAL_GPIO_ReadPin(GPIOA, COL3_Pin) == GPIO_PIN_SET && keyState[row][3] == 1) {
//            keyState[row][3] = 0;
//            keyPressed[row][3] = 0;
//        }
//    }
//}
//
//	// 按键释放后发送释放报告
//	HIDkeyBoard.KEYCODE1 = 0x00;
//	USBD_HID_SendReport(&hUsbDeviceFS, &HIDkeyBoard, sizeof(HIDkeyBoard));
//	HAL_Delay(50);
//}

//





///*
// * keypad.c
// *
// *  Created on: Aug 22, 2025
// *      Author: ShihangWu
// */
//#include "main.h"
//#include "usb_device.h"
//#include "usbd_hid.h"  // 确保包含 HID 相关的头文件
//#include "keypad.h"
//
//extern USBD_HandleTypeDef hUsbDeviceFS;
//
//// 首先定义键盘报告结构体
//typedef struct {
//    uint8_t modifier;
//    uint8_t reserved;
//    uint8_t keycode[6]; // 最多支持6个按键同时按下
//} keyboardReportDes;
//
////typedef struct {
////	uint8_t MODIFIER;
////	uint8_t RESERVED;
////	uint8_t KEYCODE1;
////	uint8_t KEYCODE2;
////	uint8_t KEYCODE3;
////	uint8_t KEYCODE4;
////	uint8_t KEYCODE5;
////	uint8_t KEYCODE6;
////}keyboardReportDes;
//
////keyboardReportDes HIDkeyBoard={0,0,0,0,0,0,0,0};
//
//// 假设矩阵键值如下
//uint8_t keypad[5][4] = {
//    {0x59, 0x5A, 0x5B, 0x5C},  // 第一行
//    {0x5D, 0x5E, 0x5F, 0x60},  // 第二行
//    {0x54, 0x55, 0x56, 0x57},  // 第三行
//    {0x00, 0x00, 0x00, 0x00},  // 第四行
//    {0x00, 0x00, 0x00, 0x00}   // 第五行
//};
//
////version 2
//Keypad_State_t keypad_state = {0};
//uint8_t KeyNum = 0;
//
//void KEYPAD_Init(void) {
//    // 初始化所有行为高电平
//    HAL_GPIO_WritePin(ROW0_GPIO_Port, ROW0_Pin, GPIO_PIN_SET);
//    HAL_GPIO_WritePin(ROW1_GPIO_Port, ROW1_Pin, GPIO_PIN_SET);
//    HAL_GPIO_WritePin(ROW2_GPIO_Port, ROW2_Pin, GPIO_PIN_SET);
//    HAL_GPIO_WritePin(ROW3_GPIO_Port, ROW3_Pin, GPIO_PIN_SET);
//    HAL_GPIO_WritePin(ROW4_GPIO_Port, ROW4_Pin, GPIO_PIN_SET);
//}
//
//void KEYPAD_SendKeyReport(uint8_t keycode) {
//	 keyboardReportDes report = {0};
//	 report.keycode[0] = keycode; // 第一个按键位置
//
//	  // 发送时需要转换为uint8_t指针
//	 USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t*)&report, sizeof(report));
//}
//
//void KEYPAD_ScanNonBlocking(void) {
//    uint32_t current_time = HAL_GetTick();
//
//    // 每5ms扫描一行（可调整）
//    if (current_time - keypad_state.last_scan_time < 5) {
//        return;
//    }
//    keypad_state.last_scan_time = current_time;
//
//    // 1. 先将所有行设置为高电平（释放上一行）
//    HAL_GPIO_WritePin(ROW0_GPIO_Port, ROW0_Pin, GPIO_PIN_SET);
//    HAL_GPIO_WritePin(ROW1_GPIO_Port, ROW1_Pin, GPIO_PIN_SET);
//    HAL_GPIO_WritePin(ROW2_GPIO_Port, ROW2_Pin, GPIO_PIN_SET);
//    HAL_GPIO_WritePin(ROW3_GPIO_Port, ROW3_Pin, GPIO_PIN_SET);
//    HAL_GPIO_WritePin(ROW4_GPIO_Port, ROW4_Pin, GPIO_PIN_SET);
//
//    // 2. 将当前行设置为低电平
//    switch (keypad_state.current_row) {
//        case 0: HAL_GPIO_WritePin(ROW0_GPIO_Port, ROW0_Pin, GPIO_PIN_RESET); break;
//        case 1: HAL_GPIO_WritePin(ROW1_GPIO_Port, ROW1_Pin, GPIO_PIN_RESET); break;
//        case 2: HAL_GPIO_WritePin(ROW2_GPIO_Port, ROW2_Pin, GPIO_PIN_RESET); break;
//        case 3: HAL_GPIO_WritePin(ROW3_GPIO_Port, ROW3_Pin, GPIO_PIN_RESET); break;
//        case 4: HAL_GPIO_WritePin(ROW4_GPIO_Port, ROW4_Pin, GPIO_PIN_RESET); break;
//    }
//
//    // 3. 立即扫描列（不需要延迟）
//    for (int col = 0; col < COLS; col++) {
//        GPIO_PinState pin_state;
//
//        // 读取列状态
//        switch (col) {
//            case 0: pin_state = HAL_GPIO_ReadPin(COL0_GPIO_Port, COL0_Pin); break;
//            case 1: pin_state = HAL_GPIO_ReadPin(COL1_GPIO_Port, COL1_Pin); break;
//            case 2: pin_state = HAL_GPIO_ReadPin(COL2_GPIO_Port, COL2_Pin); break;
//            case 3: pin_state = HAL_GPIO_ReadPin(COL3_GPIO_Port, COL3_Pin); break;
//            default: pin_state = GPIO_PIN_SET; break;
//        }
//
//        // 按键按下（低电平有效）
//        if (pin_state == GPIO_PIN_RESET) {
//            if (keypad_state.keyState[keypad_state.current_row][col] == 0) {
//                // 第一次检测到按下，记录时间
//                keypad_state.keyState[keypad_state.current_row][col] = 1;
//                keypad_state.debounce_time[keypad_state.current_row][col] = current_time;
//            }
//
//            // 消抖确认（按下持续10ms以上）
//            if (current_time - keypad_state.debounce_time[keypad_state.current_row][col] > 20) {
//                if (!keypad_state.keyPressed[keypad_state.current_row][col]) {
//                    // 发送按键按下报告
//                    KeyNum = keypad[keypad_state.current_row][col];
//                    KEYPAD_SendKeyReport(KeyNum);
//                    keypad_state.keyPressed[keypad_state.current_row][col] = 1;
//                }
//            }
//        } else {
//            // 按键释放
//            if (keypad_state.keyState[keypad_state.current_row][col] == 1) {
//                keypad_state.keyState[keypad_state.current_row][col] = 0;
//                keypad_state.keyPressed[keypad_state.current_row][col] = 0;
//
//                // 发送按键释放报告
//                KEYPAD_SendKeyReport(0x00);
//            }
//        }
//    }
//
//    // 4. 移动到下一行
//    keypad_state.current_row = (keypad_state.current_row + 1) % ROWS;
//}
//
////void KEYPAD_ScanNonBlocking(void) {
////    uint32_t current_time = HAL_GetTick();
////
////    // 每5ms扫描一行（可调整）
////    if (current_time - keypad_state.last_scan_time < 5) {
////        return;
////    }
////    keypad_state.last_scan_time = current_time;
////
////    // 1. 先将所有行设置为高电平（释放上一行）
////    HAL_GPIO_WritePin(ROW0_GPIO_Port, ROW0_Pin, GPIO_PIN_SET);
////    HAL_GPIO_WritePin(ROW1_GPIO_Port, ROW1_Pin, GPIO_PIN_SET);
////    HAL_GPIO_WritePin(ROW2_GPIO_Port, ROW2_Pin, GPIO_PIN_SET);
////    HAL_GPIO_WritePin(ROW3_GPIO_Port, ROW3_Pin, GPIO_PIN_SET);
////    HAL_GPIO_WritePin(ROW4_GPIO_Port, ROW4_Pin, GPIO_PIN_SET);
////
////    // 2. 将当前行设置为低电平
////    switch (keypad_state.current_row) {
////        case 0: HAL_GPIO_WritePin(ROW0_GPIO_Port, ROW0_Pin, GPIO_PIN_RESET); break;
////        case 1: HAL_GPIO_WritePin(ROW1_GPIO_Port, ROW1_Pin, GPIO_PIN_RESET); break;
////        case 2: HAL_GPIO_WritePin(ROW2_GPIO_Port, ROW2_Pin, GPIO_PIN_RESET); break;
////        case 3: HAL_GPIO_WritePin(ROW3_GPIO_Port, ROW3_Pin, GPIO_PIN_RESET); break;
////        case 4: HAL_GPIO_WritePin(ROW4_GPIO_Port, ROW4_Pin, GPIO_PIN_RESET); break;
////    }
////
////    // 3. 立即扫描列（不需要延迟）
////    for (int col = 0; col < COLS; col++) {
////        GPIO_PinState pin_state;
////
////        // 读取列状态
////        switch (col) {
////            case 0: pin_state = HAL_GPIO_ReadPin(COL0_GPIO_Port, COL0_Pin); break;
////            case 1: pin_state = HAL_GPIO_ReadPin(COL1_GPIO_Port, COL1_Pin); break;
////            case 2: pin_state = HAL_GPIO_ReadPin(COL2_GPIO_Port, COL2_Pin); break;
////            case 3: pin_state = HAL_GPIO_ReadPin(COL3_GPIO_Port, COL3_Pin); break;
////            default: pin_state = GPIO_PIN_SET; break;
////        }
////
////        // 按键按下（低电平有效）
////        if (pin_state == GPIO_PIN_RESET) {
////            if (keypad_state.keyState[keypad_state.current_row][col] == 0) {
////                // 第一次检测到按下，记录时间
////                keypad_state.keyState[keypad_state.current_row][col] = 1;
////                keypad_state.debounce_time[keypad_state.current_row][col] = current_time;
////            }
////
////            // 消抖确认（按下持续10ms以上）
////            if (current_time - keypad_state.debounce_time[keypad_state.current_row][col] > 20) {
////                if (!keypad_state.keyPressed[keypad_state.current_row][col]) {
////                    // 发送按键按下报告
////                    KeyNum = keypad[keypad_state.current_row][col];
////                    KEYPAD_SendKeyReport(KeyNum);
////                    keypad_state.keyPressed[keypad_state.current_row][col] = 1;
////                }
////            }
////        } else {
////            // 按键释放
////            if (keypad_state.keyState[keypad_state.current_row][col] == 1) {
////                keypad_state.keyState[keypad_state.current_row][col] = 0;
////                keypad_state.keyPressed[keypad_state.current_row][col] = 0;
////
////                // 发送按键释放报告
////                KEYPAD_SendKeyReport(0x00);
////            }
////        }
////    }
////
////    // 4. 移动到下一行
////    keypad_state.current_row = (keypad_state.current_row + 1) % ROWS;
////}
//
////version 1
////uint8_t KeyNum;
////
////void ScanKeypad(void) {
////	static uint8_t keyState[5][4] = {0};  // 按键状态缓存
////	static uint8_t keyPressed[5][4] = {0}; // 按键是否已经处理过
////
////	for (int row = 0; row < 5; row++) {
////    // 先将所有行设置为高电平
////    HAL_GPIO_WritePin(GPIOB, ROW0_Pin, GPIO_PIN_SET);
////    HAL_GPIO_WritePin(GPIOB, ROW1_Pin, GPIO_PIN_SET);
////    HAL_GPIO_WritePin(GPIOA, ROW2_Pin, GPIO_PIN_SET);
////    HAL_GPIO_WritePin(GPIOA, ROW3_Pin, GPIO_PIN_SET);
////    HAL_GPIO_WritePin(GPIOA, ROW4_Pin, GPIO_PIN_SET);
////
////    // 将当前行设置为低电平
////    if (row == 0) HAL_GPIO_WritePin(GPIOB, ROW0_Pin, GPIO_PIN_RESET);
////    if (row == 1) HAL_GPIO_WritePin(GPIOB, ROW1_Pin, GPIO_PIN_RESET);
////    if (row == 2) HAL_GPIO_WritePin(GPIOA, ROW2_Pin, GPIO_PIN_RESET);
////    if (row == 3) HAL_GPIO_WritePin(GPIOA, ROW3_Pin, GPIO_PIN_RESET);
////    if (row == 4) HAL_GPIO_WritePin(GPIOA, ROW4_Pin, GPIO_PIN_RESET);
////
////    // 消抖延迟（10ms），确保按键稳定
////    HAL_Delay(10);
////
////    // 扫描列
////    for (int col = 0; col < 4; col++) {
////        if (col == 0 && HAL_GPIO_ReadPin(GPIOB, COL0_Pin) == GPIO_PIN_RESET && keyState[row][col] == 0) {
////            keyState[row][col] = 1;  // 按下了键
////            if (keyPressed[row][col] == 0) {
////                KeyNum = keypad[row][col];  // 设置键码
////                HIDkeyBoard.KEYCODE1 = KeyNum;
////                USBD_HID_SendReport(&hUsbDeviceFS, &HIDkeyBoard, sizeof(HIDkeyBoard));
////                keyPressed[row][col] = 1;  // 标记该按键已经处理过
////            }
////        }
////        if (col == 1 && HAL_GPIO_ReadPin(GPIOB, COL1_Pin) == GPIO_PIN_RESET && keyState[row][col] == 0) {
////            keyState[row][col] = 1;
////            if (keyPressed[row][col] == 0) {
////                KeyNum = keypad[row][col];
////                HIDkeyBoard.KEYCODE1 = KeyNum;
////                USBD_HID_SendReport(&hUsbDeviceFS, &HIDkeyBoard, sizeof(HIDkeyBoard));
////                keyPressed[row][col] = 1;
////            }
////        }
////        if (col == 2 && HAL_GPIO_ReadPin(GPIOB, COL2_Pin) == GPIO_PIN_RESET && keyState[row][col] == 0) {
////            keyState[row][col] = 1;
////            if (keyPressed[row][col] == 0) {
////                KeyNum = keypad[row][col];
////                HIDkeyBoard.KEYCODE1 = KeyNum;
////                USBD_HID_SendReport(&hUsbDeviceFS, &HIDkeyBoard, sizeof(HIDkeyBoard));
////                keyPressed[row][col] = 1;
////            }
////        }
////        if (col == 3 && HAL_GPIO_ReadPin(GPIOA, COL3_Pin) == GPIO_PIN_RESET && keyState[row][col] == 0) {
////            keyState[row][col] = 1;
////            if (keyPressed[row][col] == 0) {
////                KeyNum = keypad[row][col];
////                HIDkeyBoard.KEYCODE1 = KeyNum;
////                USBD_HID_SendReport(&hUsbDeviceFS, &HIDkeyBoard, sizeof(HIDkeyBoard));
////                keyPressed[row][col] = 1;
////            }
////        }
////    }
////
////    // 按键松开后重置状态
////    for (int col = 0; col < 4; col++) {
////        if (HAL_GPIO_ReadPin(GPIOB, COL0_Pin) == GPIO_PIN_SET && keyState[row][0] == 1) {
////            keyState[row][0] = 0;
////            keyPressed[row][0] = 0;  // 重置已处理标志
////        }
////        if (HAL_GPIO_ReadPin(GPIOB, COL1_Pin) == GPIO_PIN_SET && keyState[row][1] == 1) {
////            keyState[row][1] = 0;
////            keyPressed[row][1] = 0;
////        }
////        if (HAL_GPIO_ReadPin(GPIOB, COL2_Pin) == GPIO_PIN_SET && keyState[row][2] == 1) {
////            keyState[row][2] = 0;
////            keyPressed[row][2] = 0;
////        }
////        if (HAL_GPIO_ReadPin(GPIOA, COL3_Pin) == GPIO_PIN_SET && keyState[row][3] == 1) {
////            keyState[row][3] = 0;
////            keyPressed[row][3] = 0;
////        }
////    }
////}
////
////// 按键释放后发送释放报告
////HIDkeyBoard.KEYCODE1 = 0x00;
////USBD_HID_SendReport(&hUsbDeviceFS, &HIDkeyBoard, sizeof(HIDkeyBoard));
////HAL_Delay(50);
////}
//
//
//
//
