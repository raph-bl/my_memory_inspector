#include <stdio.h>
#include <unistd.h>

int main(void) {
    int counter = 0;

    static char secret[] = "HELLO_FROM_VICTIM"; 

    printf("victim PID = %d\n", getpid());
    fflush(stdout);

    while (1) {
        printf("tick %d\n", counter);
        fflush(stdout);
        counter++;
        sleep(1);
    }

    return 0;
}
