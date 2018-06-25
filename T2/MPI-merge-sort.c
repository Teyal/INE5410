// V. Freitas [2018] @ ECL-UFSC
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <mpi.h>

/***
 * Todas as Macros pré-definidas devem ser recebidas como parâmetros de
 * execução da sua implementação paralela!!
 ***/
#ifndef DEBUG // to use DEBUG add -DDEBUG=m on compiler
#define DEBUG 0
#endif

#ifndef NELEMENTS
#define NELEMENTS 100
#endif

#ifndef MAXVAL
#define MAXVAL 255
#endif // MAX_VAL

void debug(const char* msg, ...) {
	if (DEBUG > 2) {
		va_list args;
		va_start(args, msg);
		vprintf(msg, args);
		va_end(args);
	}
}

void print_array(int array[], int size) {
	printf("Printing Array:\n");
	for (int i = 0; i < size; ++i) {
		printf("%d. ", array[i]);
	}
	printf("\n");
}

// VER SE TEM MANEIRA MELHOR DE FAZER ESSE MERGE SÓ COM O PONTEIRO
/*void merge(int numbers[], int begin, int middle, int end, int sorted[]) {
	int i, j;
	i = begin; j = middle;
	debug("Merging. Begin: %d, Middle: %d, End: %d\n", begin, middle, end);
	print_array(numbers, end);
	for (int k = begin; k < end; ++k) {
		debug("LHS[%d]: %d, RHS[%d]: %d\n", i, numbers[i], j, numbers[j]);
		if (i < middle && (j >= end || numbers[i] < numbers[j])) {
			sorted[k] = numbers[i];
			i++;
		} else {
			sorted[k] = numbers[j];
			j++;
		}
	}
}*/
void merge (int * a, int size, int * temp) {
//void merge (int a[], int size, int temp[]) {
  int i1 = 0;
  int i2 = size / 2;
  int tempi = 0;

  while (i1 < size / 2 && i2 < size)
    {
      if (a[i1] < a[i2])
	{
	  temp[tempi] = a[i1];
	  i1++;
	}
      else
	{
	  temp[tempi] = a[i2];
	  i2++;
	}
      tempi++;
    }
  while (i1 < size / 2)
    {
      temp[tempi] = a[i1];
      i1++;
      tempi++;
    }
  while (i2 < size)
    {
      temp[tempi] = a[i2];
      i2++;
      tempi++;
    }
  // Copy sorted temp array into main array, a
	print_array(a, size);
  memcpy (a, temp, size * sizeof (int));
}

void recursive_merge_sort(int* tmp, int begin, int end, int* numbers) {
// void recursive_merge_sort(int tmp[], int begin, int end, int numbers[]) {
	if (end - begin < 2)
		return;
	else {
		printf("DOIN STUFFFSSSSSSSSSSSSS\n");
		int middle = (begin + end)/2;
		recursive_merge_sort(numbers, begin, middle, tmp);
		recursive_merge_sort(numbers, middle, end, tmp);
		//merge(tmp, begin, middle, end, numbers);
		merge(tmp, end, numbers);
	}
}

void mpi_merge_sort(int* numbers, int size, int* tmp, int step,
// void mpi_merge_sort(int numbers[], int size, int tmp[], int step,
										int my_rank, int max_rank, MPI_Comm comm) {
	int power = pow(2,step);
	//printf("The step is: %d\n", power);
	int friend_rank = my_rank + power; // determs the rank of the process that will help
	if(friend_rank > max_rank){
		printf("IS SOMEONE HERE?\n");
		recursive_merge_sort(tmp, 0, size, numbers);
	} else {
		//printf("Process %d has helper %d\n", my_rank, friend_rank);
		// pode ser usado request com Isend para melhor performance
		MPI_Request request;
		MPI_Status status;

		int* meio = numbers+size/2;										//it was with max_rank
		MPI_Isend(meio, size-size/2, MPI_INT, friend_rank, step, comm, &request);

		print_array(meio, size-size/2);
		printf("Process %d, before mpi recusivity\n", my_rank);
		mpi_merge_sort(numbers, size/2, tmp, step+1, my_rank, max_rank, comm);
		printf("Process %d, after mpi recusivity\n", my_rank);
		MPI_Request_free (&request);

		MPI_Recv (meio, size-size/2, MPI_INT, friend_rank, MPI_ANY_TAG, comm, &status);
		printf("%d RECEIVED\n", my_rank);
		int middle = (size-1)/2;
		//merge (numbers, 0, middle, size, tmp);
		merge (numbers, size, tmp);
	}
}

void friend_merge_sort(int my_rank, int max_rank, MPI_Comm comm){
	MPI_Status status;
  int size;
	printf("hey friend %d here\n", my_rank);
  MPI_Probe (MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status);
  MPI_Get_count (&status, MPI_INT, &size);
	int parent_rank = status.MPI_SOURCE;
	int step = status.MPI_TAG;
	int *a = malloc (sizeof (int) * size);
	int *tmp = malloc (sizeof (int) * size);
  MPI_Recv (a, size, MPI_INT, parent_rank, MPI_ANY_TAG, comm, &status);
	mpi_merge_sort(a, size, tmp, step+1, my_rank, max_rank, comm);
  // Send sorted array to parent process
	printf("%d is possibly trying to send\n", my_rank);
  MPI_Send (a, size, MPI_INT, parent_rank, 123, comm);
	printf("%d SENDED \n", my_rank);
	free(a);
	free(tmp);
	return;
}

int * main_merge_sort(int * numbersA, int sizeA, int* numbersB, int sizeB, int* tmp, int max_rank, MPI_Comm comm) {
// int * main_merge_sort(int numbersA[], int sizeA, int numbersB[], int sizeB, int tmp[], int max_rank, MPI_Comm comm) {
	//recursive_merge_sort(numbersA, 0, sizeA, tmp);
	mpi_merge_sort(numbersA, sizeA, tmp, 0, 0, max_rank, comm);
	//VOLTAR AQUI NÃO TERMINOU AINDA
	//recursive_merge_sort(numbersB, 0, sizeB, tmp+sizeA);
	printf("WARNING!!\n");
	MPI_Barrier(MPI_COMM_WORLD);
	mpi_merge_sort(numbersB, sizeB, tmp+sizeA, 0, 0, max_rank, comm);
	int * result = malloc((sizeA+sizeB)*sizeof(int));
	// merge(tmp, 0, sizeA, sizeA+sizeB, result);

	merge(tmp, sizeA+sizeB, result);//merge(tmp, sizeA+sizeB, result);
	return result;
}

void populate_array(int* array, int size, int max) {
// void populate_array(int array[], int size, int max) {
	int m = max+1;
	for (int i = 0; i < size; ++i) {
		array[i] = rand()%m;
	}
}

int main (int argc, char ** argv) {
	int rank, size;
	MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (rank==0){
		int seed, max_val;
		int * sortable1, * sortable2;
		int * tmp ;
		size_t arr_size1, arr_size2;

		// Basic MERGE unit test
		/*
		if (DEBUG > 1) {
			int * a = (int*)malloc(8*sizeof(int));
			a[0] = 1; a[1] = 3; a[2] = 4; a[3] = 7;
			a[4] = 0; a[5] = 2; a[6] = 5; a[7] = 6;
			int * values = (int*)malloc(8*sizeof(int));
			merge(a, 0, 4, 8, values);
			free (a);
			print_array(values, 8);
			free(values);
	        return 2;
		}

		// Basic MERGE-SORT unit test
		if (DEBUG > 0) {
			int * a = (int*)malloc(8*sizeof(int));
			int * b = (int*)malloc(8*sizeof(int));
			a[0] = 7; a[1] = 6; a[2] = 5; a[3] = 4;
			a[4] = 3; a[5] = 2; a[6] = 1; a[7] = 0;

			b = memcpy(b, a, 8*sizeof(int));
			merge_sort(a, 8, b);
			print_array(b, 8);

			free(a);
			free(b);

			a = (int*)malloc(9*sizeof(int));
			b = (int*)malloc(9*sizeof(int));
			a[0] = 3; a[1] = 2; a[2] = 1;
			a[3] = 10; a[4] = 11; a[5] = 12;
			a[6] = 0; a[7] = 1; a[8] = 1;

			b = memcpy(b, a, 9*sizeof(int));
			print_array(b, 9);

			merge_sort(a, 9, b);
			print_array(a, 9);

			free(a);
			free(b);
			printf("\n");
	        return 1;
		}
		*/
		switch (argc) {
			case 1:
				seed = time(NULL);
				arr_size1 = NELEMENTS;
				arr_size2 = NELEMENTS;
				max_val = MAXVAL;
				break;
			case 2:
				seed = atoi(argv[1]);
				arr_size1 = NELEMENTS;
				arr_size2 = NELEMENTS;
				max_val = MAXVAL;
				break;
			case 3:
				seed = atoi(argv[1]);
				arr_size1 = atoi(argv[2]);
				arr_size2 = NELEMENTS;
				max_val = MAXVAL;
				break;
			case 4:
				seed = atoi(argv[1]);
				arr_size1 = atoi(argv[2]);
				arr_size2 = atoi(argv[3]);
				max_val = MAXVAL;
				break;
			case 5:
				seed = atoi(argv[1]);
				arr_size1 = atoi(argv[2]);
				arr_size2 = atoi(argv[3]);
				max_val = atoi(argv[4]);
				break;
			default:
				printf("Too many arguments\n");
				break;
		}
		srand(seed);
		sortable1 = malloc(arr_size1*sizeof(int));
	  sortable2 = malloc(arr_size2*sizeof(int));
		tmp 	  = malloc((arr_size1+arr_size2)*sizeof(int));

		populate_array(sortable1, arr_size1, max_val);
		srand(seed+1);
		populate_array(sortable2, arr_size2, max_val);

		memcpy(tmp, sortable1, arr_size1*sizeof(int));
		memcpy(tmp + arr_size1, sortable2, arr_size2*sizeof(int));

		print_array(sortable1, arr_size1);
		print_array(sortable2, arr_size2);

		//MPI_Send(void* buf, int count, MPI_INT, int dest, int tag, MPI_Commcomm);
		int * result = main_merge_sort(sortable1, arr_size1,sortable2, arr_size2, tmp, size-1, MPI_COMM_WORLD);
		print_array(result, (arr_size1+arr_size2));

		free(sortable1);
		free(sortable2);
		free(tmp);
		free(result);
	} else {
		friend_merge_sort(rank, size -1, MPI_COMM_WORLD);
		MPI_Barrier(MPI_COMM_WORLD);
		printf("WARNING2\n");
		friend_merge_sort(rank, size - 1, MPI_COMM_WORLD); //para o segundo array (verificar se esta correto)
	}

	MPI_Finalize();
	return 0;
}
