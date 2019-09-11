#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

// By passing in a pointer to this struct when we create the thread
// we can get around only being able to pass in one argument to the
// thread.
struct newRow_runner_struct
{
    int x;
    int dimension;
    double *readArr;
    double *writeArr;
    double precision;
    int completed;
};

// Creates the memory space for a 1D array to be used in a 2D manner.
double *createSqArr(int dimension)
{
    double *arr = malloc(dimension * dimension * sizeof(double));
    return arr;
}

// Prints out input array in a readable format.
void printArr(double *arr, int d)
{
    printf("Array: \n");
    for (int x = 0; x < d; x++)
    {
        for (int y = 0; y < d; y++)
        {
            printf("%f ", arr[(x * d) + y]);
        }
        printf("\n");
    }
}

// Returns an incomplete X*Y array where the inner elements 
// (elements to average) are 0's.
// By using this as the input array, we are easily able to test whether the
// relaxation technique is working.
double *getXYArr(int dimension)
{
    double *arr = createSqArr(dimension);
    for (int x = 0; x < dimension; x++)
    {
        for (int y = 0; y < dimension; y++)
        {
            arr[(x * dimension) + y] = x * y;
        }
    }
    for (int x = 1; x < dimension - 1; x++)
    {
        for (int y = 1; y < dimension - 1; y++)
        {
            arr[(x * dimension) + y] = 0;
        }
    }
    return arr;
}

// Replaces the inner values (i.e. not the border values) in the readArr
// with the values in the writeArr.
void updateReadArr(double *readArr, double *writeArr, int dimension)
{
    for (int x = 1; x < dimension - 1; x++)
    {
        for (int y = 1; y < dimension - 1; y++)
        {
            readArr[(x * dimension) + y] = writeArr[(x * dimension) + y];
        }
    }
}

// Thread function to calculate new average of all elements in one row
// of the readArr.
void *newRow_runner(void *arg)
{
    // Make sure the input arg is of type newRow_runner_struct
    struct newRow_runner_struct *arg_struct =
        (struct newRow_runner_struct *)arg;

    int x = arg_struct->x;
    int d = arg_struct->dimension;

    // Loop through each y/column the row the thread is working on and
    // calculate the average. If we have reached desired precision,
    // increase the completed counter.
    for (int y = 1; y < (arg_struct->dimension - 1); y++)
    {
        arg_struct->writeArr[(x * d) + y] = 
            (arg_struct->readArr[((x - 1) * d) + y] +
            arg_struct->readArr[(x * d) + (y + 1)] +
            arg_struct->readArr[((x + 1) * d) + y] +
            arg_struct->readArr[(x * d) + (y - 1)]) / 4;

        if (fabs(arg_struct->writeArr[(x * d) + y] -
                 arg_struct->readArr[(x * d) + y]) <= arg_struct->precision)
        {
            arg_struct->completed = arg_struct->completed + 1;
        }
    }
    pthread_exit(0);
}

// Relaxation technique solver. Loops through the rows of the given array
// and assigns one thread to each row. This continues until all rows have
// been averaged once. If all values have reached desired precision return
// the solved array. If not, restart and average all values again.
double *solveArr(double *readArr, double *writeArr, int dimension,
                 int numOfThreads, double precision)
{
    // Total number of rows of we need to calculate i.e. not including top and
    // bottom row.
    int rowsToCalculate = dimension - 2; 
    // Total number of rows whos y values have all reached desired precision. 
    int completed = 0;
    // Set up my array of structs to be used by the threads. 
    struct newRow_runner_struct threads[numOfThreads];

    // Thread IDs
    pthread_t tids[numOfThreads];

    // While all inner values haven't reached the required precision.
    while (completed < rowsToCalculate) 
    {
        // Number of rows we've calculated the average for on current run.
        int rowsCalculated = 0;

        // Reset number of fully precise rows that have reached desired 
        // precision on each run.
        completed = 0;       
        int threadsCreated = 0;

        // Loop through each row in the given array.
        for (int x = 1; x < dimension - 1; x++)
        {
            // Only create a thread if rows calculated is less than the 
            // total rows required to average.
            if (rowsCalculated < rowsToCalculate) 
            {
                threads[threadsCreated].x = x;
                threads[threadsCreated].dimension = dimension;
                threads[threadsCreated].readArr = readArr;
                threads[threadsCreated].writeArr = writeArr;
                threads[threadsCreated].precision = precision;
                threads[threadsCreated].completed = 0;

                // Create thread attributes then initialise
                // - controls how the thread functions
                pthread_attr_t attr;
                pthread_attr_init(&attr);
                // Start thread
                pthread_create(&tids[threadsCreated], &attr, newRow_runner,
                               &threads[threadsCreated]);
                threadsCreated = threadsCreated + 1;
                rowsCalculated = rowsCalculated + 1;
            }
            if ((threadsCreated == numOfThreads) || (rowsCalculated == rowsToCalculate))
            {
                // Wait until threads are finished
                for (int i = 0; i < threadsCreated; i++)
                {
                    pthread_join(tids[i], NULL);
                    if (threads[i].completed == (dimension - 2))
                    { // if all values in row are to right precision
                        completed = completed + 1;
                    }
                }
                updateReadArr(readArr, writeArr, dimension);
                threadsCreated = 0;
            }
        }
    }
    return readArr;
}

int main(int argc, char **argv)
{
    int dimension = atoi(argv[1]);
    double precision = atof(argv[2]);
    int numOfThreads = atoi(argv[3]);
    double *readArr = getXYArr(dimension);
    double *writeArr = getXYArr(dimension);
    clock_t begin = clock();
    printArr(readArr, dimension);
    solveArr(readArr, writeArr, dimension, numOfThreads, precision);
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Run time: %f\n", time_spent);
    printArr(readArr, dimension);
    return 0;
}