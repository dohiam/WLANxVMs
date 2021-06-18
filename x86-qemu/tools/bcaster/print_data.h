#ifndef PRINT_DATA_H
#define PRINT_DATA_H

#include <stdint.h>
#include <stdbool.h>

#define PRINT_QUEUE_SIZE 1024

//Note: enum not used because we want a specific order and to cycle through the colors in order
#define GREEN  0
#define CYAN   1
#define RED    2
#define YELLOW 3
#define PURPLE 4
#define BLUE   5
#define WHITE  6

#define FIRST_COLOR GREEN
#define LAST_COLOR BLUE
#define DEFAULT_COLOR WHITE

void change_color(int color);

void print_data_init(bool ignore_beacons, bool print_data_bytes);

void print_data_add_pkt(int client, uint8_t *pkt, uint32_t len, int color);

#endif // PRINT_DATA_H
