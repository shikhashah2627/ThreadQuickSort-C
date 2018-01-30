#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct  sortvalues
{
	int *array;
	int  left;
	int  right;
};

// thread function calling quicksort
void* qsthreadfunction(void *init)
{
	struct  sortvalues *start = init;
	quick_sort(start ->array , start ->left , start ->right);
	return  NULL;
}

void swap(int *a, int *b)

{

    int temp;
    temp = *a;
    *a = *b;
    *b = temp;

}

int partion(int arr[], int p, int r)

{

    int pivotIndex = p + rand()%(r - p + 1); //generates a random number as a pivot
    int pivot;
    int i = p - 1;
    int j;

    pivot = arr[pivotIndex];//gets pivot value
	printf("pivot is %d \n %d \n",pivot,arr[r]);

    swap(&arr[pivotIndex], &arr[r]);
	printf("after swapping pivot index is %d and r is %d \n",arr[pivotIndex],arr[r]);
	printf("value of i is %d \n",i);
    for (j = p; j < r; j++)
    {printf("p value, pivot value and r value while entering for loop and i and j are %d,%d,%d,%d,%d \n",p,pivot,r,i,j);
	printf("before swapping i and j values are %d and %d \n",arr[i],arr[j]);
        if (arr[j] < pivot)
        {
           i++;
		printf("%d \n",arr[j]);
		printf("%d \n \n",arr[i]);
           swap(&arr[i], &arr[j]);
		printf("after swapping arr[i] is %d and arr[j] is %d \n",arr[i],arr[j]);
        }

    }
    swap(&arr[i+1], &arr[r]);

    return i + 1;

}

void quick_sort(int arr[], int p, int q)

{

    int j;

    if (p < q)

    {
	pthread_t t1,t2;

       	j = partion(arr, p, q);
	printf("partition performed \n");

	struct  sortvalues arg1 = {arr , p ,j-1};
	struct  sortvalues arg2 = {arr , j+1 ,q};
	//  Create a new  thread  for  one  subsort
	pthread_create (&t1 , NULL , qsthreadfunction , &arg1);
	printf("thread1 is over \n");
	pthread_create (&t2 , NULL, qsthreadfunction , &arg2);
	printf("thread2 is over \n");
	pthread_join(t1,NULL);
	pthread_join(t2,NULL);

       //quick_sort(arr, p, j-1); -- will be called by thread 1
       //quick_sort(arr, j+1, q); -- will be called by thread 2

    }

}



int main()

{

  int i;
	int sizeforsorting;
	int *arr = (int*) malloc(sizeof(int)*sizeforsorting);
	printf("Please enter the size of array for sorting ");
	scanf("%d",&sizeforsorting);


    for (i = 0; i < sizeforsorting; i++)
	{
        arr[i] = rand()%1000;
	printf("%d \n",arr[i]);
	}
    	quick_sort(arr, 0, sizeforsorting-1);//function to sort the elements of array

    for (i = 0; i < sizeforsorting; i++)
         printf("%d \n", arr[i]);

    return 0;

}
