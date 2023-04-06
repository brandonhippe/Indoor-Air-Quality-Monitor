#ifndef CRC
#define CRC


// Include necessary libraries
#include <stdint.h>


// Function prototypes
uint8_t calculate_crc(void *input);
int check_crc(uint8_t data[]);


#endif
