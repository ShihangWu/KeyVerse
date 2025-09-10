/*
 * keypad.h
 *
 *  Created on: Aug 23, 2025
 *      Author: ShihangWu
 */

#ifndef INC_KEYPAD_H_
#define INC_KEYPAD_H_

void ScanKeypad(void) ;

#endif /* INC_KEYPAD_H_ */




//#ifndef __KEYPAD_H
//#define __KEYPAD_H
//
//#include "stm32f1xx_hal.h"
//
//#define ROWS 5
//#define COLS 4
//
////// 按键矩阵定义
////extern const uint8_t keypad[ROWS][COLS];
//
//// 键盘状态结构体
//typedef struct {
//    uint8_t keyState[ROWS][COLS];      // 当前按键状态
//    uint8_t keyPressed[ROWS][COLS];    // 按键已处理标志
//    uint8_t current_row;               // 当前扫描行
//    uint32_t last_scan_time;           // 上次扫描时间
//    uint32_t debounce_time[ROWS][COLS]; // 每个按键的消抖时间
//} Keypad_State_t;
//
//extern Keypad_State_t keypad_state;
//extern uint8_t KeyNum;
//
//void KEYPAD_Init(void);
//void KEYPAD_ScanNonBlocking(void);
//void KEYPAD_SendKeyReport(uint8_t keycode);
//
//#endif
