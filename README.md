
# Retirement Clock for ATmega4809 Curiosity Nano
Retirement clock implemented in C for ATmega4809 Curiosity Nano microcontroller as part of Embedded Microprocessor Systems course. 

## Contributors
* [@mabenj](https://github.com/mabenj)
* [@eselanne](https://github.com/eselanne)

### Functional Description
- Left button turns backlight on for 5 seconds(adjustable).
- Right button rotates views:
  - View #1: date and time (clock view).
  - View #2: days, hours, minutes, and seconds until retirement (countdown view).
  - View #3: days, hours, minutes, and seconds the system has been running.
- Serial user interface implements:
  - SETDATETIME dd.mm.yyyy hh:mi:ss
  - GET DATETIME
  - SET BIRTHDAY dd.mm.yyyy
  - GET BIRTHDAY
  - SET BACKLIGHT seconds
  - GET BACKLIGHT
- Sounds a buzzer when it is time to retireand displays "Go home, old-timer!"(centered, divided into two rows, obviously)

### ATmega4809 Data Sheet
http://ww1.microchip.com/downloads/en/DeviceDoc/ATmega4808-09-DataSheet-DS40002173B.pdf
