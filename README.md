# Aligned
Proposed by [<u>Tina Samie</u>](https://github.com/tantoon94), [<u>Dankao Chen</u>](https://github.com/Reikimen), [<u>Jiaying Shen</u>](https://github.com/JY-SHENNNN), [<u>YouTian Peng</u>](https://github.com/rorschachwilpeng)

## What is Aligned?

It’s a pair of handheld devices designed to help people stay emotionally close, even when physically apart. Rooted in the quiet power of alignment—both in direction and in feeling—Aligned offers a new way to share presence across distance.

Instead of words or screens, it uses orientation and light.
You gently rotate your device toward a presence that matters.

Once aligned, a single press shares a feeling—subtle, intentional, and beyond language.

**Aligned isn’t just a compass—it’s your emotional North Star.**

---- 

👉[Discover **Aligned** in this short video](https://www.youtube.com/watch?v=aNNHWGniDm8)


***
### Function Design
<div style="text-align: center;">
  <img src="https://raw.githubusercontent.com/Reikimen/CASA0021-Group-Prototype/refs/heads/main/Img/flowchart.png" alt="Flowchart">
</div>

**Sender side:**

After pressing the start button, the device turns on. If it is the first time using the device, connect WiFi through your phone. Then, the magnetic sensor will start to calibrate. If it has already been calibrated before, the device will start to find the direction of the paired receiver. Once the device is correctly aligned, the user can choose an emotion using the buttons.

**Receiver side:**

If this is the first time using the device, connect WiFi through your phone. Then, the magnetic sensor will start to calibrate. If calibration is already done before, the device will wait for a message from the sender. After receiving the message, the device enters "seek mode" with LED guidance:

- :green_circle: Green light means the device is pointing in the correct direction.
- :red_circle:Red light means the direction is wrong.

When all the LEDs turn green, the correct direction is found.
Then, the LEDs will show the emotional message sent by the sender.

Light instruction:
- yellow: happy :smile:
- blue: sad  :cry:
- red: angry    	:angry:
   

***
### Hardware Design
<div style="text-align: center;">
  <img src="https://raw.githubusercontent.com/Reikimen/CASA0021-Group-Prototype/refs/heads/main/Img/GPP_bb.jpg" alt="Flowchart">
</div>


| Component                 | Description                                           |
|--------------------------|-------------------------------------------------------|
| ESP8266          | Main MCU, handles logic and communication |
| NeoPixel Ring         | RGB LED ring for visual feedback|
| Push Buttons             | Three small tactile buttons for user input            |
| Big Red Button           | Primary start button             |
| Switch Button          | Control power on or off             |
| HMC5883L                 | 3-axis digital compass for direction sensing          |
| USB battery Module            | LiPo battery charging and protection module           |
| LiPo Battery (3.7V, 1000mAh) | Portable power source for the entire system      |
| vibration | Provide hearable and touchable effect                |
| MOSFET |  Provide higher power for vibration|

<div>

    pin4    --- button1(sad)
    pin18   --- button2(happy)
    pin19   --- button3(angry)
    pin23   --- start button4
    pin16   --- Motor
    pin2    --- LED ring
    D22     --- SCL
    D21     --- SDA
    VCC     --- 3.3V 
    GND     --- GND
</div>


***


----
**Reference:**

1. [https://github.com/troelssiggaard/ESP32-fritzing-module](https://forum.fritzing.org/t/esp-wroom-32d-firebeetle/13869/5)

2. https://github.com/adafruit/Fritzing-Library/blob/master/parts/retired/Neopixel%2024%20Ring.fzpz
