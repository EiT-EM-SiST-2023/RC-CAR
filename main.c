#include <avr/io.h>
#include "pwm_control.h"

int main(){
    pwm_init();
    set_motor_power(50);
    while(1){}
}