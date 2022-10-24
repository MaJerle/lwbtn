#include <stdio.h>
#include <string.h>
#include "lwbtn/lwbtn.h"
#include "windows.h"

const int keys[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
int state[sizeof(keys) / sizeof(keys[0])];
int state_prev[sizeof(keys) / sizeof(keys[0])];

int
main(void) {
    printf("Application running\r\n");

    while (1) {
        /* Simple simulator */
        for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
            state[i] = GetAsyncKeyState(keys[i]) < 0;
            if (state[i] != state_prev[i]) {
                printf("State changed for key %c to: %d\r\n", keys[i], state[i]);
            }
            state_prev[i] = state[i];
        }
    }
}
