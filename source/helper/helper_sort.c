#include "helper_sort.h"

static void swap(int* a, int* b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

// bubble sort function
void bubbleSort(int array[], int n) 
{ 
    int i, j;
    for (i = 0; i < n-1; i++)
        for (j = 0; j < n-i-1; j++)
            if (array[j] > array[j+1])
                swap(&array[j], &array[j+1]);
}