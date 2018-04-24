#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {

    srand(time(NULL));

    int n;
    int count = 0;

    printf("N = ");
    scanf("%d", &n);

    int *array = (int *) malloc(sizeof(int) * n);
    
    for (int i = 0; i < n; i++) {
        array[i] = (rand() % 2);
    }

    for (int i = 0; i < n; i++) {
        if (array[i] == 1)
            count++;
    }

    printf("Count: %d\n", count);
    
    return EXIT_SUCCESS;
}
