# Lab 10: ESP-NOW Block Network

ESP32-S3 sample sketch for sending discovery messages and listing nearby boards without a router.

**Sample only:** Sir's block sketch is not available yet. Class compatibility is not confirmed. For sample testing, every participating board needs this sketch and the same channel. Use the official block sketch when provided.

## Arduino IDE

1. Install **esp32 by Espressif Systems** through Boards Manager. No extra library is needed.
2. Open [espnow_block.ino](espnow_block/espnow_block.ino).
3. Select the correct ESP32-S3 board and port. **ESP32S3 Dev Module** is the generic option.
4. For the native USB connector, enable [USB CDC On Boot](https://docs.espressif.com/projects/arduino-esp32/en/latest/tutorials/cdc_dfu_flash.html). For a separate USB-to-UART connector, leave it disabled.
5. Upload, then open Serial Monitor at **115200 baud**.

Only the ESP32-S3 and USB data cable are needed. No OLED, LED, or jumper wires are required.

Build checked with ESP32-S3 and Espressif core **3.3.12**. Board-to-board testing is pending.

## Test

- Sample channel: **1**, set by `CHANNEL` at the top of the sketch.
- The display lists the board's MAC, elapsed time, and unique other boards heard since startup. Repeated messages do not increase the count.
- During the assigned block window, keep the board powered on and within about **5 metres** of the other boards. Do not connect the ESP32 to a Wi-Fi network.
- After at least **30 seconds**, capture the count and MAC list in a screenshot. Then record a **30-second screen video**.

## Results and submission

[castor_rhea_leen](castor_rhea_leen/) contains the results form. Results and evidence are pending.

Every student in the block submits the **same repository link** in MOLE. Keep evidence in a top-level `lastname_firstname` folder. Edit only the assigned folder; do not add another student's results.

Technical documentation: [Espressif ESP-NOW](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/espnow.html).
