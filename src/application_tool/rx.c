#include <stdio.h>
#include <stdlib.h>   // For rand() and srand()
#include <time.h>     // For time()
#include <unistd.h>   // For sleep()

int main(int argc, char** argv) {
    while (0==0){
    if (argc > 1) {
        printf("%s\n", argv[1]);  // Correctly print the first argument
    } else {
        printf("No input provided.\n");
    }

    // Seed the random number generator
    srand(time(NULL));

    // Generate a random number between 0 and 9999
    int random_number = rand() % 10000;

    printf("Random number: %d\n", random_number);

    printf("RX code paused\n");
    sleep(3);
    fflush(stdout);
    printf("RX code finishing\n");
}
    return 0;
}
