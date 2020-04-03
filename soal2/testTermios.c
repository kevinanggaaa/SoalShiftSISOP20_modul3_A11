#include <termios.h>
#include <stdio.h>

void set_keypress(void) {
    struct termios new_settings;
    tcgetattr(0,&stored_settings);
    new_settings = stored_settings;
    /* Disable canonical mode, and set buffer size to 1 byte */
    new_settings.c_lflag &= (~ICANON);
    new_settings.c_cc[VTIME] = 0;
    new_settings.c_cc[VMIN] = 1;
    tcsetattr(0,TCSANOW,&new_settings);
    return;
}

void reset_keypress(void){
    tcsetattr(0,TCSANOW,&stored_settings);
    return;
}

int main() {
    printf("started\n");
    printf("set_keypress\n");
    set_keypress();
    printf("set_keypress done\n");
    while (1) {
        printf("in loop");
        if (getchar()) {

        }
    }
    reset_keypress
    return 0;
}