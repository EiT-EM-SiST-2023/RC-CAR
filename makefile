NAME = RCcar
SHELL = cmd.exe
SERIAL = COM3
ARDUINO = -F -V -c arduino -P $(SERIAL) -b 115200

DEF = -DF_CPU=16000000UL -D__AVR_ATmega328P__

SRC = main.c pwm_control.c
OBJ = $(SRC:.c=.o)

INC = -I.

Build : $(NAME).hex

Burn : Build
	avrdude $(ARDUINO) -p ATMEGA328P -U flash:w:$(NAME).hex:i

$(NAME).hex : $(NAME).elf
	avr-objcopy -j .text -j .data -O ihex $< $@

$(NAME).elf : $(OBJ)
	avr-gcc -mmcu=atmega328p -o $@ $^

%.o : %.c
	avr-gcc -Os $(DEF) -mmcu=atmega328p -c $< -o $@ $(INC)

clean:
	del -f *.o *.elf *.hex