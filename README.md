# KeyVerse

---

## Setup

### About hardware

- MCU: STM32F103C8T6
- 灯：WS2812B
- 旋转编码器：EC11
- OLED显示屏：基于SSD1306驱动的OLED显示屏

### About software

- 基于HAL库，使用STM32CubeIDE开发
- 裸机开发，顺序执行的结构，有一些状态机的思想（`main.c` 中 `status`变量用于切换键盘模式）

---

## 现有的功能

- 上电后键盘灯为红色，OLED屏显示`KeyVerse`。此时为键盘输入状态，为防止误触编码器导致颜色突变，转动编码器不会有任何反应。按下键盘按键能够向电脑输入0-9数字以及一些运算符，每个按键均与电脑的小键盘区域的按键对应，也可通过`Num lock`键切换层，实现输入数字或光标上下左右移动等功能。

  > 在电脑键盘上的`Num lock`键（`Num lock`灯亮）已按下的键盘映射
  >
  > （`Num lock`灯灭时则为与小键盘另一层一一对应的上下左右等功能）
  >
  > | backspace |  /   |   *   |      |
  > | :-------: | :--: | :---: | :--: |
  > |     7     |  8   |   9   |  -   |
  > |     4     |  5   |   6   |  +   |
  > |     1     |  2   |   3   |      |
  > |     0     |  .   | enter |      |

- 按下旋转编码器后，OLED屏显示图片。此时为灯颜色调节状态，为防止误触，按下键盘按键不会有任何反应。转动编码器可以切换键盘灯的颜色，顺时针旋转，颜色由红色过渡到绿色过渡到蓝色在过渡回红色，周期为一圈，即转动一圈后颜色相同。

- 再次按下旋转编码器后，OLED屏显示`KeyVerse`，回到键盘输入状态。

---

## 代码中可以直接修改而无需修改程序结构的部分

- 键盘映射： [`keypad.c `](./Core/Src/keypad.c) 中 keypad变量，键码需参考官方的与USB HID相关的文档
- 灯光颜色：[`ws2812.c`](./Core/Src/ws2812.c)
- OLED显示屏显示内容：[`oled.h`](./Core/Src/font.c) 

---

## 需要注意的地方

- 用STM32CubeMX重新初始化（即修改.ioc文件）后，需将[`usbd_hid.c`](./Middlewares/ST/STM32_USB_Device_Library/Class/HID/Src/usbd_hid.c)和[`usbd_hid.h`](./Middlewares/ST/STM32_USB_Device_Library/Class/HID/Src/usbd_hid.h)重新修改为键盘对应的代码

