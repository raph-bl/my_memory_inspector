#include <stdio.h>
#include <unistd.h>

int fonction_de_test_int(int nb) {
    nb = 42;
    return nb;
}

char fonction_de_test_char() {
    return 'a';
}

void fonction_de_test_void() {
    printf("helloworld"); 
}

int main(void) {
    int counter = 0;

    static char secret[] = "HELLO_FROM_VICTIM"; 

    static int x = 42;
    static size_t y = 42;

    fonction_de_test_void();
    fonction_de_test_int(12);
    fonction_de_test_char();

    printf("victim PID = %d\n", getpid());
    printf("Buffer address: %p\n", (void *)secret); 
    printf("Buffer size: %zu\n", sizeof(secret)); 
    fflush(stdout);

    while (1) {
        printf("tick %d\n", counter);
        fflush(stdout);
        counter++;
        sleep(1);
    }

    return 0;
}
