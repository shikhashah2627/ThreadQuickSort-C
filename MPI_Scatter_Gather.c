#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

#define max_rows 100000

int array[max_rows];
int array2[max_rows];
int finalarray[max_rows];
int remaining[max_rows];

void swap(int *a, int *b)
{
	int temp;
	temp = *a;
	*a = *b;
	*b = temp;
}

int partion(int arr[], int p, int r)
{
	//int pivotIndex = p + rand()%(r - p + 1); //generates a random number as a pivot
	// printf("%d\n", pivotIndex);
	int i = p - 1;
	int pivot = arr[r];
	int j;
	// r-1 is needed for j-1
	for (j = p; j <= r - 1; j++)
	{
		if (arr[j] <= pivot)
		{
			i++;
			swap(&arr[i], &arr[j]); //values are swapped to fill and get the 1st array with elements less than pivot value
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
		j = partion(arr, p, q);
		//printf("%d %d %d\n", j, p, q);
		quick_sort(arr, p, j-1);
		quick_sort(arr, j+1, q);
	}
}

int main(int argc, char **argv)
{
	long int sum, partial_sum;
	MPI_Status status;
	int my_id, root_process, ierr, i, num_rows, num_procs,
		an_id, num_rows_to_receive, avg_rows_per_process,remainingvalues,
		sender, num_rows_received, start_row, end_row, num_rows_to_send;
	ierr = MPI_Init(&argc, &argv);
	root_process = 0;

	ierr = MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
	ierr = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

	num_rows = atoi(argv[1]);
	avg_rows_per_process = num_rows / num_procs;
	remainingvalues = num_rows % num_procs;
	// if ( remainingvalues == 0 )
	// 	printf("Entered values will be equally scattered \n");
	// else {
	// 	printf("%d values are left out for scatter \n" , remainingvalues);
	// }

	if(my_id == root_process) {

		for(i = 0; i < num_rows; i++) {
			array[i] = rand()%10000;
			printf("%d \n",array[i]);
		}
	}

	int totalValuesSorted = avg_rows_per_process*num_procs;

	MPI_Scatter(array,avg_rows_per_process,MPI_INT,array2,avg_rows_per_process,MPI_INT,0,MPI_COMM_WORLD);
	printf("avg_rows_per_process : %d \n" ,avg_rows_per_process );
	// for (i = 0; i < avg_rows_per_process; i++) {
	// 	printf("before quicksort scattered result PID : %d array el: %d \n", my_id, array2[i]);
	// }

	quick_sort(array2,0,avg_rows_per_process - 1 );

	// for (i = 0; i < avg_rows_per_process; i++) {
	// 	printf("after quicksort scattered result PID : %d array el: %d \n", my_id, array2[i]);
	// }
	MPI_Gather(array2,avg_rows_per_process,MPI_INT,finalarray,avg_rows_per_process,MPI_INT,0,MPI_COMM_WORLD);
	//for (i = 0; i < (avg_rows_per_process*num_procs) ; i++) {
		//printf("Gathering result PID : %d array el: %d \n", my_id, finalarray[i]);
	//}
	if(my_id == root_process && totalValuesSorted == num_rows) {
		quick_sort(finalarray,0,num_rows-1);
		for (i = 0; i < num_rows ; i++) {
			printf("After final sort result PID : %d array el: %d \n", my_id, finalarray[i]);
 }
} else {
		if (my_id == root_process && totalValuesSorted < num_rows) {
			for (i = 0; i < remainingvalues; i++) {
				finalarray[totalValuesSorted] = array[num_rows-1];
				num_rows--;
				totalValuesSorted++;
				//printf("Addition to final array: %d \n" , finalarray[totalValuesSorted] );
			}
			int finalcount = atoi(argv[1]);
			for(i = 0; i < finalcount ; i++ )
				printf("Final array being sent for quicksort with finalarray[%d] : %d \n " , i , finalarray[i]);
			quick_sort(finalarray,0,finalcount - 1);
			for (i = 0; i < finalcount ; i++) {
					printf("After final sort array el of finalarray[%d] : %d \n", i,finalarray[i]);
			}
		}
}

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	return 0;
	}
