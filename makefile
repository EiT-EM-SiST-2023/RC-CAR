NAME = RCcar
SERIAL = COM3 #com to which arduino is connected
ARDUINO = -F -V -c arduino -P $(SERIAL) -b 115200

DEF = -DF_CPU=16000000UL -D__AVR_ATmega328P__

SRC = \
main.c

INC = \
-I.

Burn : Build
	avrdude $(ARDUINO) -p ATMEGA328P -U flash:w:$(NAME).hex:i

Build : $(NAME).elf
	avr-objcopy -j .text -j .data -O ihex $< $(NAME).hex
	
$(NAME).elf : $(NAME).o
	avr-gcc -Os $(DEF) -mmcu=atmega328p $(SRC) $(INC) -o $@
	
$(NAME).o : $(SRC)
	avr-gcc -g -Os $(DEF) -mmcu=atmega328p $^ $(INC) -o $@
