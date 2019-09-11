#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

double** createArr(int m, int n){
    double* values = calloc(m*n, sizeof(double));
    double** rows = malloc(n*sizeof(double*));
    for (int i=0; i<n; ++i) {
        rows[i] = values + i*m;
    }
    return rows;
}

double** solveArr(double** arr, int dimension, int threads, int precision) {
    double** solvedArr = createArr(dimension,dimension);
    int completed = 0; 
    while (completed < (dimension-2)*(dimension-2)) {
        for (int i = 1; i < dimension-1; i++) {
            for (int j = 1; j < dimension-1; j++) {
                double newValue = (arr[i-1][j] + arr[i][j-1] + arr[i+1][j] + arr[i][j+1]) / 4;
                if (fabs(newValue - arr[i][j]) > precision) {
                    arr[i][j] = newValue;
                } else {
                    completed = completed + 1;
                }
            }
        }
    }
    return arr;
} 

void outputArr(double** arr, int d) {
    printf("Array: \n");
    for (int i=0; i<d; i++) {
        for (int j=0; j<d; j++) {
            printf("%f ", arr[i][j]);
        }
        printf("\n");
    }
}

double** getXYArr(int dimension) {
    double** arr = createArr(dimension,dimension);    
    for (int i=0; i<dimension; i++) {
        for (int j=0; j<dimension; j++) {
            arr[i][j] = i*j;
        }
    }
    for (int i = 1; i < dimension-1; i++) {
        for (int j = 1; j < dimension-1; j++) {
            arr[i][j] = 3;
        }
    }
    return arr;
}

int main() {
    double** inputArr = getXYArr(1000);
    // outputArr(inputArr, 1000);
    double** solvedArr = createArr(1000,1000);
    clock_t begin = clock();
    solvedArr = solveArr(inputArr, 1000, 1, 0.001);
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Run time: %f", time_spent);
    // outputArr(solvedArr, 1000);
    return 0;
}