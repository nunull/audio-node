# audio-node

## Code examples

- `hi`: A basic Arduino example showcasing i2s throughput and LED blinking.
- `hi-aat`: A basic Arduino example using [Arduino Audio Tools](https://github.com/pschatzmann/arduino-audio-tools)
- `uac`: An incomplete ESP-IDF-based example showcasing USB UAC e.g. audio interface mode. Not yet working completely, but can be a basis.

## Hardware

### Amp

MAX98357A i2s amp, gain configurable using bridges

- DIN: `GPIO_NUM_5`
- BLCK: `GPIO_NUM_6`
- LRCLK: `GPIO_NUM_7`

### Mic

ICS-43434 MEMS i2s mic

- WS: `GPIO_NUM_2`
- SCK: `GPIO_NUM_3`
- SD: `GPIO_NUM_4`

### Button

- `GPIO_NUM_45`

### LED

- `GPIO_NUM_21`

### GPIOS

- `GPIO_NUM_9`, `GPIO_NUM_10`, `GPIO_NUM_11`, `GPIO_NUM_12`