# pico-hd44780
Controlling HD44780 display with Raspberry Pi Pico

This is my first attempt to program RaspberryPi PIO and DMA. I wanted to control HD44780 display with simple API and without bit banging.

Program uses small fifo which contains data to send to display. You can control size of the fifo by changing value of HD44780_BUFFERSIZE. If there is free space in fifo, functions for sending data to display do not block and data are send to display in background by DMA/PIO.

Many programmers use only write mode (RW pin is connected to GND). They have to count with the time display need to process data. I use different method where BUSY flag is read from display back (RW pin is controlled).

## Connections

    PIN N+0 = D4
    PIN N+1 = D5
    PIN N+2 = D6
    PIN N+3 = D7
    PIN N+4 = RS
    PIN N+5 = RW
    PIN N+6 = E
    
Value of ```N``` is set in hd44780_init function. 
My display can be powered by 3.3V so if RW pin is HI there is correct voltage on input pins of pico. If you use different voltage, some extra circuit must be used. In my example I use pins 16 to 22.

## API

Use function ```hd44780_init``` to initialize library. You can pass ```pio``` and ```sm``` arguments to select which PIO and state machine is used. Pin ```N``` is defined by argument ```pin``` last argument defines frequency.

After that you can use functions hd44780_* to control display - they are described in datasheet of HD44780.

## Performance

When 2MHz frequency is used, I managed to draw approx. 300frames per second.

## Screenshot from logic analyzer

![obrazek](https://user-images.githubusercontent.com/3948538/121377825-f592df80-c942-11eb-93d4-37e674c91607.png)

## Example

![IMG_20210609_165236b](https://user-images.githubusercontent.com/3948538/121378657-af8a4b80-c943-11eb-86fe-38e5f6b1d06d.jpg)

## Conclusion

This is only attempt to learn something new. This is definitely not the best implementation. If you know better way, how to do it, let me know :)

