#include <stdio.h>

int main(int argc, char** argv) {
    printf("tx out:\n");

    if (argc < 2) {
        printf("  No input arguments provided.\n");
    } else {
        for (int i = 1; i < argc; i++) {
            printf("  Arg %d: %s\n", i, argv[i]);
        }
    }


    return 0;
}