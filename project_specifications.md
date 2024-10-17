# Software Team


## INFO
Our job is to handle the digital side of the MP3 player: we make the thing work, in terms of software. Playing music, navigating menus, making things show up on the screen, etc. We will work with the hardware team on the display, and maybe also talk about adding a speaker.


## Hardware Features:
- Rotary Dial
- 4 Buttons
- Ili LCD Screen
- ESP32


### Microprocessor
ESP-32-S3-WROOM-1 (maybe? This could change if we or the hardware team realize that there’s something else we’d like to do). 
See the [datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf)

### DAC
MAX 98357a 12s Amp

### Screen
ILI9341 (has SD card)

### Input
4 buttons and rotary encoder-1


## Libraries for mp3 
Pin requirements for parts 

## Must-Have Features
- Audio to Jack
- DOOM
- Load Songs (SD card)
- Navigate Menu/UI


## Backlog
- Bluetooth integration
- Pebble backlight 
- Customizable audio menu
- PLAY DOOM
- Wifi-based song uploading
- Ogg and other formats 
- UI - Equilizer

## Libraries
- Audio: [ESP32-audioI2S](https://github.com/schreibfaul1/ESP32-audioI2S/tree/master)

## Research
- [Audio Jack/Drivers Research](https://docs.google.com/document/d/1Oj3tkaLAvQZwekmTf7ODMa6XOVWbadpt8SI9NW8nfO0/edit?usp=sharing)


## Next Steps
- Research DAC Driver (mp3)
- Research LCD Driver 
- Reading SD FIles (ESP understating)
- DOOM possibility 


hardware : Ryan and Nate

