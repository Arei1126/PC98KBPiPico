#  PC-9801 Series Keyboard Converter with Raspberry Pi Pico
Intended for 2nd gen PC-9801 keyboard (no NFER key, mini DIN 8 pin) ]
It dosen't support controll command through Txd, so it can't control keyboard such as LEDs.
To do this, ~RST must be connected to GPIO which Hardware Serial2 available. (GPIO3 is not)
~~~
On host/converter:
    8Pin mini DIN
       ___ ___
      /  |_|  \
     / 8  7  6 \
    | 5    4  3 |
     \_ 2   1 _/
       \_____/
     (receptacle)

    Pin mini DIN        Raspberry Pi Pico
    ---------------------------------------------
    1  ~RST(TXD)        GP3
    2   GND             GND
    3  ~RDY             GP4
    4   RXD             GP2
    5  ~RTY             CC
    6   NC
    7   NC
    8   5V              VCC
~~~
# References 
[TMK PC98-USB Converter](https://github.com/tmk/tmk_keyboard/wiki/PC-9801-Keyboard)
[PC98 to USB keyboard protocol converter](https://github.com/tmk/tmk_keyboard/tree/master/converter/pc98_usb)
[KeyboardPico](https://github.com/HisashiKato/KeyboardPico/)
