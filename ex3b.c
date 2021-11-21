/* ex3b.c
===============================================================================
Written by: Meytal Abrahamian  login: meytalben  id: 211369939
            Tomer Akrish              tomerak        315224295
===============================================================================
*/

// --------------------------------includes------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h> 
#include <sys/types.h>
#include <unistd.h>

// -------------------------------global variables-----------------------------
#define ARR_SIZE 1000
#define NUM_OF_CHILDREN 3
#define SEED 17

struct my_msg {
    pid_t status;
    int prime;
};

int count_new_prime = 0;

// ----------------------------------------------------------------------------
void do_parent(pid_t children_pid[], int pipe_descs[], int pipe_personal[][2]);
void do_child(int pipe_descs[], int pipe_personal[]);
void create_children(int pipe_descs[], int pipe_personal[][2]);
void close_parent_pipe(int pipe_descs[], int descs_path, 
    int pipe_personal[][2], int personal_path);
void kill_children(pid_t children_pid[]);
void catch_signal(int term);
void open_pipe(int pipe_id[]);
int get_prime();
int count_appear(int arr[], int num, int index);
int is_prime(int number);
int child_num(pid_t children_pid[], pid_t status);

// -------------------------------main-----------------------------------------
int main()
{
    int pipe_descs[2],                      // for all childs & parent: child 
        pipe_personal[NUM_OF_CHILDREN][2];  // pipe to each child with parent

    open_pipe(pipe_descs);                  // open the general pipe
    create_children(pipe_descs, pipe_personal);

    return EXIT_SUCCESS;
}
// ----------------------------------------------------------------------------

// this function create all NUM_OF_CHILDREN and open personal pipe for each one
void create_children(int pipe_descs[], int pipe_personal[][2])
{
    pid_t children_pid[NUM_OF_CHILDREN];
    pid_t status;
    int i;

    for (i = 0; i < NUM_OF_CHILDREN; ++i)
    {
        open_pipe(pipe_personal[i]);
        status = fork();
        if (status < 0)
        {
            perror("can not fork");
            exit(EXIT_FAILURE);
        }
        else if (status == 0)
            do_child(pipe_descs, pipe_personal[i]);

        children_pid[i] = status;
    }

    do_parent(children_pid, pipe_descs, pipe_personal);
}
// ----------------------------------------------------------------------------

// this function is for parenth
// it read prime from child, check appearance of this prime in array and write 
// it to child. kill all children and print num of diffrent value in array
void do_parent(pid_t children_pid[], int pipe_descs[], int pipe_personal[][2])
{
    int count_new_prime = 0,             // to count new value in array
        appearance = 0,                  // to count appearance of num in array
        i;                               // for loop
    int arr[ARR_SIZE];
    struct my_msg msg;                   // { child_pid, prime }

    close_parent_pipe(pipe_descs, 1, pipe_personal, 0);

    for (i = 0; i < ARR_SIZE; ++i)
    {
        // read prime number from pipe (that child sent)
        if (read(pipe_descs[0], &msg, sizeof(struct my_msg)) < 0)
            exit(EXIT_FAILURE);
        arr[i] = msg.prime;
        appearance = count_appear(arr, msg.prime, i);
        if (appearance == 0)
            ++count_new_prime;
        // writh to child the appearance of the num he sent
        if (write(pipe_personal[child_num(children_pid, msg.status)][1],
            &appearance, sizeof(int)) < 0)
            exit(EXIT_FAILURE);
    }

    kill_children(children_pid);
    printf("Sum of diffrent value in array: %d\n", count_new_prime);
    close_parent_pipe(pipe_descs, 0, pipe_personal, 1);
}
// ----------------------------------------------------------------------------

// this function is for children. in while loop it gets random num,
// check if it prime and if it is send the num to parent, read from parent the
// appearance of prime and when need to end it prints num of new prime it gave
void do_child(int pipe_descs[], int pipe_personal[])
{
    int num;			// for count new prime from this child
    signal(SIGTERM, catch_signal);
    srand(SEED);                        // for rand
    close(pipe_descs[0]);               // close pipe for reading
    close(pipe_personal[1]);            // close pipe for writing
    struct my_msg msg;

    while (1)
    {
        num = get_prime();
        msg.status = getpid();
        msg.prime = num;
        // sent to parent my id and prime
        if (write(pipe_descs[1], &msg, sizeof(struct my_msg)) < 0)
            exit(EXIT_FAILURE);

        // read from pipe the appearence of the prime num in array      
        if (read(pipe_personal[0], &num, sizeof(int)) < 0)
            exit(EXIT_FAILURE);
        if (num == 0)
            ++count_new_prime;
    }
}
// ----------------------------------------------------------------------------

int get_prime()
{
    int num;
    while (!is_prime(num = rand() % 999 + 2)) {}

    return num;
}
// ----------------------------------------------------------------------------

// this function is a signal hendler
void catch_signal(int term)
{
    printf("Sum of diffrent value:%d\tpid: %d\n", count_new_prime, getpid());
    exit(EXIT_SUCCESS);
}
// ----------------------------------------------------------------------------

// this function kill the children
void kill_children(pid_t children_pid[])
{
    int i;

    for (i = 0; i < NUM_OF_CHILDREN; ++i)
        kill(children_pid[i], SIGTERM);
}
// ----------------------------------------------------------------------------

// this function gets number and return 1 if its a prime number and 0 if not
int is_prime(int number)
{
    int i;

    if (number < 2)
        return 0;

    for (i = 2; i * i < number; ++i)
    {
        if (number % i == 0)
            return 0;
    }
    return 1;
}
// ----------------------------------------------------------------------------

// this function open pipe using pipe id
void open_pipe(int pipe_id[])
{
    if (pipe(pipe_id) == -1)
    {
        perror("cannot open pipe");
        exit(EXIT_FAILURE);
    }
}
// ----------------------------------------------------------------------------

// this function close pipe in perenth
void close_parent_pipe(int pipe_descs[], int descs_path,
    int pipe_personal[][2], int personal_path)
{
    int i;

    close(pipe_descs[descs_path]);
    for (i = 0; i < NUM_OF_CHILDREN; ++i)
    {
        close(pipe_personal[i][personal_path]);
    }
}
// ----------------------------------------------------------------------------

// this function count appearance of num in array and returns the result
int count_appear(int arr[], int num, int index)
{
    int i,
        counter = 0;

    for (i = 0; i < index; ++i)
    {
        if (arr[i] == num)
            ++counter;
    }

    return counter;
}
// ----------------------------------------------------------------------------

// this function find status in children_pid arr and returns child index
int child_num(pid_t children_pid[], pid_t status)
{
    int i;

    for (i = 0; i < NUM_OF_CHILDREN; ++i)
    {
        if (children_pid[i] == status)
            return i;
    }
    return 0;
}
// ----------------------------------------------------------------------------
