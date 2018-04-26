#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {

    srand(time(NULL));

    long n;
    long count = 0;

    printf("N = ");
    scanf("%ld", &n);

    long *array = (long *) malloc(sizeof(int) * n);

    for (long i = 0; i < n; i++) {
        array[i] = (rand() % 2);
    }

    for (long i = 0; i < n; i++) {
        if (array[i] == 1)
            count++;
    }

    printf("Count: %ld\n", count);
    
    return EXIT_SUCCESS;
}
