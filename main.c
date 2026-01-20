#include <avr/io.h>

int main(){
    DDRD = (1 << PD3);
}