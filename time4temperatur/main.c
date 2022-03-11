/*
 * I2C Example project for the mcb32 toolchain
 * Demonstrates the temperature sensor and display of the Basic IO Shield
 * Make sure your Uno32-board has the correct jumper settings, as can be seen
 * in the rightmost part of this picture:
 * https://reference.digilentinc.com/_media/chipkit_uno32:jp6_jp8.png?w=300&tok=dcceb2
 */

#include <pic32mx.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/* Address of the temperature sensor on the I2C bus */
#define TEMP_SENSOR_ADDR 0x48

/* Temperature sensor internal registers */
typedef enum TempSensorReg TempSensorReg;
enum TempSensorReg {
  TEMP_SENSOR_REG_TEMP,
  TEMP_SENSOR_REG_CONF,
  TEMP_SENSOR_REG_HYST,
  TEMP_SENSOR_REG_LIMIT,
};

// custom delay, delay in certain milliseconds
void delay_ms(int ms) {
  int i;
  while (ms > 0) {
    ms = ms - 1;
    for (i = 0; i < 2355; i = i + 1) {}
  }
}

// this function convert a uint32_t number into a chars
char * fixed_to_string_32(uint32_t num, char * buf) {
  bool neg = false;
  uint32_t n;
  char * tmp;

  // checking if number is negative by checking sign bit if so converting it into positive number
  if (num & 0x80000000) {
    num = ~num + 1;
    neg = true;
  }

  buf += 4;
  n = num >> 16;
  tmp = buf;
  // converting number into respective ascii characters
  do {
    *--tmp = (n % 10) + '0';
    n /= 10;
  } while (n);
  
  // checking if boolean for neg is true if so add negative mark ("-")
  if (neg)
    *--tmp = '-';

  n = num;
  
  // checking if there is no number in integer part if so return temp
  if (!(n & 0xFFFF)) {
    * buf = 0;
    return tmp;
  }
  // adding . mark
  * buf++ = '.';
  // checking fraction part if there is number if so convert it into ascii
  // in out case we only check the first 8 bits even though there exist 16 bits in total
  // we noticed if we increase our check we get to many digits in fraction part
  while ((n &= 0xFF00)) {
    n *= 10;
    * buf++ = (n >> 16) + '0';
  }
  * buf = 0;

  return tmp;
}

// this function calculates the length of string
uint32_t strlenc(char * str) {
  uint32_t n = 0;
  while ( * str++)
    n++;
  return n;
}

// this function clears the display by setting each to empty string
void display_clear(void) {
  display_string(0, "");
  display_string(1, "");
  display_string(2, "");
  display_string(3, "");
}

// this function is used to choose state you give a ptr to state variable
// and pressedBtns which is the currently pressed btns
void choose_state(int pressedBtns, int * state) {
  // checking if a btn is active
  if (pressedBtns) {
    if ((pressedBtns & 0x1) && * state != 4) {
      // kelvin
      * state = 3;
    }

    // check btn 3
    if ((pressedBtns >> 1) & 0x1) {
      // fahrenheit
      * state = 2;
    }

    // checking if btn 4 is acrive
    if ((pressedBtns >> 2) & 0x1) {
      //celsius
      * state = 1;
    }
  }

  if (getbtn1()) {
    if ( * state == 0) {
      // measure temp
      * state = 4;
    } else {
      * state = 0;
    }
  }
}

// this functions display the temperature symbol decided by the state
void display_temp_symbol(char * text, char * ptr, int state) {
  ptr = text + strlenc(text);
  // add space between the number
  * ptr++ = ' ';
  // adding following symbol °
  * ptr++ = 7;
  // here we check the state and assigning correct symbol
  if (state == 1 || state == 4) {
    * ptr++ = 'C';
  } else if (state == 2) {
    * ptr++ = 'F';
  } else if (state == 3) {
    * ptr++ = 'K';
  }

  // finishing the string with null mark
  * ptr++ = 0;

}

// this functions check the timerintervall so it doesn't surpass the threshold
int check_timeintervall(int timerintervall, int sum_to_add) {

  if ((timerintervall + sum_to_add) <= 2000) {
    return 1;
  }

  return 0;

}

// this function is used to handle state it print check if the state is 4 first then it print text
// it also increments timeIntervall by checking which switch was activated and lastly confirms
// changes by pressing btn 3
void handle_state_4(int pressedBtns, int state, int * ready, int * timeIntervall, int * arrayLength) {
  if (state == 4) {
    display_string(0, "current intervall");
    display_string(1, itoaconv( * timeIntervall));
    display_string(2, "choose intervall");
    display_string(3, "with switches");
    display_update();
    * ready = 0;
    PORTE = 0x4;

    // SW1
    if (IFS(0) & 0x80) {
      int sum_to_add = 1;
      * timeIntervall = check_timeintervall( * timeIntervall, sum_to_add) ? * timeIntervall + sum_to_add : * timeIntervall;
      IFSCLR(0) = 0x80;
    }

    // SW2
    if (IFS(0) & 0x800) {
      int sum_to_add = 10;
      * timeIntervall = check_timeintervall( * timeIntervall, sum_to_add) ? * timeIntervall + sum_to_add : * timeIntervall;
      IFSCLR(0) = 0x800;
    }

    // SW3
    if (IFS(0) & 0x8000) {
      int sum_to_add = 100;
      * timeIntervall = check_timeintervall( * timeIntervall, sum_to_add) ? * timeIntervall + sum_to_add : * timeIntervall;
      IFSCLR(0) = 0x8000;
    }

    // SW4
    if (IFS(0) & 0x80000) {
      int sum_to_add = 1000;
      * timeIntervall = check_timeintervall( * timeIntervall, sum_to_add) ? * timeIntervall + sum_to_add : * timeIntervall;
      IFSCLR(0) = 0x80000;
    }

    if (pressedBtns & 0x1 && * timeIntervall != 0) {
      * arrayLength = * timeIntervall;
      * ready = 1;
      display_clear();

    }
  }

}

// this function includes our custom inits for timers and interrupts for timer
void main_init(void) {
  // setting prescaling value to 256
  T2CON = 0x70;
  // setting our time period value it will period of at 100 ms
  PR2 = 31250;
  // setting the count timer 2 to zero
  TMR2 = 0;

  TRISD &= 0xFE0;

  // enable interrupt all SW's
  IEC(0) = 0x88880;
  enable_interrupt();

  // make sure timer is off
  T2CONCLR = 0x8000;
}

// this function just displays our custom text for main menu
void display_main(void) {
  display_string(0, "btn4 for Cels");
  display_string(1, "btn3 for Fahr");
  display_string(2, "btn2 for Kelv");
  display_string(3, "btn1 to interv");
}

int main(void) {
  uint32_t temp;
  char buf[32], * s, * t;

  /* Set up peripheral bus clock */
  OSCCON &= ~0x180000;
  OSCCON |= 0x080000;

  /* Set up output pins */
  AD1PCFG = 0xFFFF;
  ODCE = 0x0;
  TRISECLR = 0xFF;
  PORTE = 0x0;

  /* Output pins for display signals */
  PORTF = 0xFFFF;
  PORTG = (1 << 9);
  ODCF = 0x0;
  ODCG = 0x0;
  TRISFCLR = 0x70;
  TRISGCLR = 0x200;

  /* Set up input pins */
  TRISDSET = (1 << 8);
  TRISFSET = (1 << 1);

  /* Set up SPI as master */
  SPI2CON = 0;
  SPI2BRG = 4;

  /* Clear SPIROV*/
  SPI2STATCLR &= ~0x40;
  /* Set CKP = 1, MSTEN = 1; */
  SPI2CON |= 0x60;

  /* Turn on SPI */
  SPI2CONSET = 0x8000;

  /* Set up i2c */
  I2C1CON = 0x0;
  /* I2C Baud rate should be less than 400 kHz, is generated by dividing
  the 40 MHz peripheral bus clock down */
  I2C1BRG = 0x0C2;
  I2C1STAT = 0x0;
  I2C1CONSET = 1 << 13; //SIDL = 1
  I2C1CONSET = 1 << 15; // ON = 1
  temp = I2C1RCV; //Clear receive buffer

  /* Set up input pins */
  TRISDSET = (1 << 8);
  TRISFSET = (1 << 1);

  main_init();

  display_init();
  display_main();

  display_update();

  PORTE = 0x16;

  int arrayLength = 0;
  int * arrayLength_ptr = & arrayLength;

  int timeIntervall = 0;
  int * timeIntervall_ptr = & timeIntervall;

  float temperature_measurments[2000];
  int index = 0;
  int timeoutcount = 0;

  // 0 menu, 1 celsius, 2 fahrenheit, 3 kelvin, 4 measure temp
  int state = 0;
  int * state_ptr = & state;
  int ready = 1;
  int * ready_ptr = & ready;

  int go = 0;

  while (1) {

    // getting currently pressed btn just btn 2,3,4
    int pressedBtns = getbtns();
    // check current state
    choose_state(pressedBtns, state_ptr);

    // continue if state changes from 0
    if (state > 0) {

      // handle state 4 (will only run if the state is 4)
      handle_state_4(pressedBtns, state, ready_ptr, timeIntervall_ptr, arrayLength_ptr);
      if (ready) {
        // start timer
        T2CONSET = 0x8000;
        go = 1;

        /* Send start condition and address of the temperature sensor with
        write mode (lowest bit = 0) until the temperature sensor sends
        acknowledge condition */
        do {
          i2c_start();
        } while (!i2c_send(TEMP_SENSOR_ADDR << 1));
        /* Send register number we want to access */
        i2c_send(TEMP_SENSOR_REG_CONF);
        /* Set the config register to 0 */
        i2c_send(0x0);
        /* Send stop condition */
        i2c_stop();

        while (go) {

          // gets currently pressed btns (this is used so we can change state from selected state)
          int pressedBtns = getbtns();
          choose_state(pressedBtns, state_ptr);

          // in case the state is 0 we will reset some variables and display main menu screen
          // also stop inner loop
          if (state == 0) {
            index = 0;
            timeoutcount = 0;

            display_main();
            display_update();
            go = 0;

          }

          // making sure state is not 0 it could change by pressing the button
          if (state > 0) {
            display_clear();
            display_string(0, "Temperature:");

            // timer interruption in our case will happen every 100ms
            if (IFS(0) & 0x100) {
              // increase timeoutcount
              timeoutcount++;

              /* Send start condition and address of the temperature sensor with
              write flag (lowest bit = 0) until the temperature sensor sends
              acknowledge condition */
              do {
                i2c_start();
              } while (!i2c_send(TEMP_SENSOR_ADDR << 1));
              /* Send register number we want to access */
              i2c_send(TEMP_SENSOR_REG_TEMP);

              /* Now send another start condition and address of the temperature sensor with
              read mode (lowest bit = 1) until the temperature sensor sends
              acknowledge condition */
              do {
                i2c_start();
              } while (!i2c_send((TEMP_SENSOR_ADDR << 1) | 1));

              /* Now we can start receiving data from the sensor data register */
              // we shift by 16 because our temp is 32 bit floating point number
              // which has 16 bits integer part and 16 bits fraction part
              // temp is also in celsius
              temp = i2c_recv() << 16;
              i2c_ack();
              temp |= i2c_recv();

              /* To stop receiving, send nack and stop */
              i2c_nack();
              i2c_stop();

             // check state this is for fahrenheit and we convert from celsius to fahrenheit
              if (state == 2) {
                // we multiply by 1.8 our temp which is in celcius
                temp = temp * 1.8;
                // we add + 32 which is represented in 32 bits floating point number
                temp += 0x00200000;
                  
              } else if (state == 3) {
                uint32_t kelvin_const = 0x01112666;
                temp += kelvin_const;

              } else if (state == 4) {
                  // check timeout we timeout every 500 ms and decrease timeIntervall
                if (timeoutcount == 5) {
                    PORTE = 0x6;
                    timeoutcount = 0;

                  if (timeIntervall > 0) {
                    timeIntervall -= 1;
                    temperature_measurments[index] = (float) temp;
                    index++;
                  }
                }
                // now we need to display the result from our measurements
                if (timeIntervall <= 0) {
                  PORTE = 0x8;
                  
                  // create necessary variables
                  int i = 0;
                  float temp_average = 0;
                  float temp_max = temperature_measurments[0];
                  float temp_min = temperature_measurments[0];
                  char buf_max[32], buf_min[32], * avg, * max, * min, * max_symbol, * min_symbol, * avg_symbol;
                
                  // we loop through the array which contains our temperature measurement
                  // we save the current temperature every second, we calculate sum of temp.
                  // and get max and min temperature
                  for (i = 0; i < arrayLength; i++) {
                    temp_average += temperature_measurments[i];

                    if (temperature_measurments[i] > temp_max) {
                      temp_max = temperature_measurments[i];
                    }

                    if (temperature_measurments[i] < temp_min) {
                      temp_min = temperature_measurments[i];
                    }

                  }
                  // calculate average temp
                  temp_average = temp_average / arrayLength;
                  
                  // converts given numbers into string
                  avg = fixed_to_string_32(temp_average, buf);
                  min = fixed_to_string_32(temp_min, buf_min);
                  max = fixed_to_string_32(temp_max, buf_max);

                  // adding the appropriate temp. symbol
                  display_temp_symbol(avg, avg_symbol, state);
                  display_temp_symbol(min, min_symbol, state);
                  display_temp_symbol(max, max_symbol, state);

                  // displaying the values 1 row is avg. temp.,  2 row is max, 3 row is min
                  display_string(0, "avg/max/min");
                  display_string(1, avg);
                  display_string(2, max);
                  display_string(3, min);
                  display_update();
                    
                  // 10 seconds delay before going back to menu screen (state 0)
                  delay_ms(10000);

                  state = 0;
                }
              }
                
              // here we convert the temp to string
              s = fixed_to_string_32(temp, buf);
              // assigning the correct temp. symbol.
              display_temp_symbol(s, t, state);

              //PORTE++;
              // displaying the temperature
              display_string(1, s);
              display_update();

              IFSCLR(0) = 0x100;
            }
          }
        }
      }
    }
  }

  return 0;
}
