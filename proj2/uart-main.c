
#include <reg932.h>
#include "uart.h"

void main() {

EA = 1;
uart_init();
uart_transmit('a');

//uart_init();
uart_transmit('q');
//uart_init();
uart_transmit('k');
//uart_init();
EA = 0;

EA = 1;
uart_transmit('y');
EA = 0;

while(1);
}
