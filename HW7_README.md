# HW7 - MCP4912 SPI DAC

## Files
- HW7_main.c
- CMakeLists.txt

## SPI pins used
- GP16 = MISO
- GP17 = CS
- GP18 = SCK
- GP19 = MOSI

## Other control pins
- GP20 = LDAC
- GP21 = SHDN

## DAC wiring
- Pico 3V3(OUT) -> MCP4912 VDD
- Pico GND -> MCP4912 VSS
- Pico GP17 -> MCP4912 CS
- Pico GP18 -> MCP4912 SCK
- Pico GP19 -> MCP4912 SDI
- Pico GP20 -> MCP4912 LDAC
- Pico GP21 -> MCP4912 SHDN
- MCP4912 VREFA -> 3.3V
- MCP4912 VREFB -> 3.3V
- MCP4912 VOUTA -> oscilloscope CH1
- MCP4912 VOUTB -> oscilloscope CH2

## Waveforms
- DAC A: 2 Hz sine wave, 0V to 3.3V
- DAC B: 1 Hz triangle wave, 0V to 3.3V
- Update rate: 200 Hz

## Notes
- This code is written for MCP4912, which is 10-bit.
- The code uses gain = 1x and VREF = 3.3V.
