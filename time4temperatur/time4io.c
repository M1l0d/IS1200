#include <stdint.h>
#include <pic32mx.h>

int getsw(void) {
return ((PORTD >> 8) & 0xF);  
}

int getbtns(void) {
return ((PORTD >> 5) & 0x7);
}

int getbtn1(void) {
return ((PORTF >> 1) & 0x1);
}


