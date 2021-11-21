/* ex1ba1.c
===============================================================================
Written by: Meytal Abrahamian  login: meytalben  id: 211369939
			Tomer Akrish              tomerak        315224295
===============================================================================

This program receives using argument vector:
	File name and natural number

The program repeats the following task 50 times:
it cerate produces two children:
	Child A will define an array, insert random data into it, and sort the
	   array by bubbles sort.
	Child B is the same but will perform a quick sort.

Each child add information to the file:
A. Which sort is realized: b or q
B. How long did it take him to sort the array.
The two data will appear one after the other, with a space between them,
in one line. The program is wait for the two children to finish

Now the parent goes through the file, computer and displays:
A. How long on average did it take to sort bubbles of the data,
   and how long did it take to quickly sort.
B. What is the shortest time a child has run a bubble sort
   and what is the shortest time a child has run a quick sort.
C. What is the most time a child who has sorted A has run compared to
   a child who has sorted B.

The program delete the using file at the end of the program and print another
informatin how long did it take to him to run since the program start until
it end.
*/

//------------------------------includes and defines---------------------------
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>


#define NUM_OF_RUNS 50
#define ARR_SIZE 50000

struct Data_time {
	double _average;
	double _fastest;
	double _slowest;
};

typedef void (*sort_func)(int[], int[]);

//-----------------------------prototype----------------------------------------
void sort_and_time(struct Data_time* bubble_data, struct Data_time* quick_data);
void randomize_arr(int arr[]);
void main_process(struct Data_time* data, int pipe_sort[], int rand_arr[],
	sort_func sorting);

void bubble_sort(int arr[], int bubble_pipe[]);
void swap(int* a, int* b);

void quick_sort_process(int arr[], int quick_pipe[]);
void quick_sort(int arr[], int low, int high);
int partition(int arr[], int low, int high);

void print_run_time(struct Data_time* bubble_data, struct Data_time* quick_data);
void calc_data(struct Data_time* bubble_data, double run_time);

//----------------------------------main----------------------------------------
int main(int argc, char* argv[])
{
	if (argc != 2)
		exit(EXIT_FAILURE);

	struct Data_time bubble_data = { 0, 100, 0 },	//average, fastest, slowest
		quick_data = { 0, 100, 0 };					//average, fastest, slowest


	srand(atoi(argv[1]));
	struct timeval t0, t1;

	gettimeofday(&t0, NULL);

	sort_and_time(&bubble_data, &quick_data);
	print_run_time(&bubble_data, &quick_data);

	gettimeofday(&t1, NULL);
	printf("parent run time %lf\n", (double)(t1.tv_usec - t0.tv_usec) / 1000000
		+ (double)(t1.tv_sec - t0.tv_sec));

	return EXIT_SUCCESS;
}

//------------------------------------------------------------------------------
// this function create chiled to quick sort and bubble sort
void sort_and_time(struct Data_time* bubble_data, struct Data_time* quick_data)
{
	int bubble_pipe[2], quick_pipe[2], arr[ARR_SIZE];
	int i;

	if (pipe(bubble_pipe) == -1 || pipe(quick_pipe) == -1)
	{
		perror("cannot open pipe\n");
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < NUM_OF_RUNS; ++i)
	{
		randomize_arr(arr);
		main_process(bubble_data, bubble_pipe, arr, bubble_sort);
		main_process(quick_data, quick_pipe, arr, quick_sort_process);
	}
	close(bubble_pipe[0]);//end of dad use in pipe
	close(bubble_pipe[1]);
	close(quick_pipe[0]);
	close(quick_pipe[1]);
}

//------------------------------------------------------------------------------
void randomize_arr(int arr[])
{
	int i;

	for (i = 0; i < ARR_SIZE; ++i)
		arr[i] = rand() % 1000;
}

//------------------------------------------------------------------------------
void main_process(struct Data_time* data, int pipe_sort[], int rand_arr[],
	sort_func sorting)
{
	double time;
	char ignore;

	pid_t status = fork();
	if (0 > status)
		exit(EXIT_FAILURE);
	else if (0 == status)
	{
		close(pipe_sort[0]);
		dup2(pipe_sort[1], STDOUT_FILENO);
		sorting(rand_arr, pipe_sort);
	}

	wait(NULL);

	dup2(pipe_sort[0], STDIN_FILENO);
	scanf("%c%c%lf", &ignore, &ignore, &time);
	calc_data(data, time);
}

//-----------------------------bubble sort--------------------------------------
void bubble_sort(int arr[], int bubble_pipe[])
{
	int i, j;
	struct timeval t0, t1;

	gettimeofday(&t0, NULL);

	for (i = 0; i < (ARR_SIZE)-1; ++i)
	{
		for (j = 0; j < ARR_SIZE - i - 1; ++j)
			if (arr[j] > arr[j + 1])
				swap(&arr[j], &arr[j + 1]);
	}

	gettimeofday(&t1, NULL);

	printf("b %lf ", ((double)t1.tv_usec - t0.tv_usec) / 1000000 +
		(double)(t1.tv_sec - t0.tv_sec));
	fflush(stdout);
	close(bubble_pipe[1]);
	exit(EXIT_SUCCESS);
}

//------------------------------------------------------------------------------
void swap(int* a, int* b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}

//------------------------------quick sort--------------------------------------
void quick_sort_process(int arr[], int quick_pipe[])
{
	struct timeval t0, t1;

	gettimeofday(&t0, NULL);

	quick_sort(arr, 0, ARR_SIZE);

	gettimeofday(&t1, NULL);
	printf("q %lf ", (double)(t1.tv_usec - t0.tv_usec) / 1000000 +
		(double)(t1.tv_sec - t0.tv_sec));
	fflush(stdout);
	close(quick_pipe[1]);
	exit(EXIT_SUCCESS);
}

//------------------------------------------------------------------------------
void quick_sort(int arr[], int low, int high)
{
	int index;
	if (low < high)
	{
		index = partition(arr, low, high);

		quick_sort(arr, low, index - 1);
		quick_sort(arr, index + 1, high);
	}
}

//------------------------------------------------------------------------------
int partition(int arr[], int low, int high)
{
	int pivot = arr[high];
	int i = (low - 1);

	for (int j = low; j <= high - 1; j++)
	{
		if (arr[j] < pivot)
		{
			++i;
			swap(&arr[i], &arr[j]);
		}
	}
	swap(&arr[i + 1], &arr[high]);
	return (i + 1);
}

//------------------------------------------------------------------------------
// this function find run time of bubble sort and quick sort
void print_run_time(struct Data_time* bubble_data, struct Data_time* quick_data)
{
	bubble_data->_average /= NUM_OF_RUNS;
	quick_data->_average /= NUM_OF_RUNS;

	printf("bubble sort average: %lf\tquick sort average: %lf\n",
		bubble_data->_average, quick_data->_average);
	printf("bubble sort fastest: %lf\tquick sort fastest: %lf\n",
		bubble_data->_fastest, quick_data->_fastest);
	printf("bubble sort slowest: %lf\tquick sort slowest: %lf\n",
		bubble_data->_slowest, quick_data->_slowest);
}

//------------------------------------------------------------------------------
//this func calc average, fastest time and slowest time of sort
void calc_data(struct Data_time* data, double run_time)
{
	data->_average += run_time;

	if (run_time < data->_fastest)				//find fastest run time
		data->_fastest = run_time;
	else if (run_time > data->_slowest)			//find slowest run time
		data->_slowest = run_time;
}
