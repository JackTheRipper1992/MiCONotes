# Schematic Hacking

* [核心板原理图](MiCOKit-3166-3239V0.1.pdf)
* [扩展板原理图](MiCOKit-EXT.pdf)

## 核心板原理图分析

* MCU引脚接口
  ![EWM3239_MCU.jpg](image/EWM3239_MCU.jpg)
* 启动状态选择拨码  
  ![Boot_Status.jpg](image/Boot_Status.jpg)
* 功耗检测电路
  ![Current_Detection.jpg](image/Current_Detection.jpg)
* Debug Port调试口  
  ![USB_TO_Debug_UART.jpg](image/USB_TO_Debug_UART.jpg)
* SWD下载接口，虽然使用JTAG下载，但实际上使用SWD下载，可以直接用ST-Link下载；  
  ![JTAG_SWD.jpg](image/JTAG_SWD.jpg)
* Arduino接口  
  * VDD、VBUS电源；
  * Reset信号，当按下核心板上的Reset按键时，同时复位对应的外围传感器；
  * 2路ADC转换引脚；
  * 一路I2C（各种传感器）；
  * 一路SPI（OLED）；
  * 6个GPIO引脚；
  * 一路用户UART口（另一路被当做Debug Port了）；
  ![Arduino_Connector_Compare.png](image/Arduino_Connector_Compare.png)

## 扩展板原理图分析

* OLED屏，SPI接口通信  
  ![OLED_SPI.jpg](image/OLED_SPI.jpg)
* RGB LED， I2C接口通信  
  ![RGB_LED.jpg](image/RGB_LED.jpg)
* 运动传感器，I2C接口通信  
  ![Motion_Sensor.jpg](image/Motion_Sensor.jpg)
* 亮度测量，ADC转换  
  ![Light_Measure.jpg](image/Light_Measure.jpg)
* 红外反射，ADC转换  
  ![Infrared_Reflective.jpg](image/Infrared_Reflective.jpg)
* 温湿度传感器  
  ![HAndTSensor.jpg](image/HAndTSensor.jpg)
* 2路按键  
  ![KeyBoard.jpg](image/KeyBoard.jpg)
