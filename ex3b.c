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

struct my_msg {
    pid_t status;
    int prime;
};

int end = 0;

// ----------------------------------------------------------------------------
void valid_status(pid_t status);
void do_parent(pid_t children_status[], int arr[], int pipe_descs[], 
               int pipe_personal[][2]);
void do_child(int pipe_descs[], int pipe_personal[]);
int is_prime(int number);
void open_pipe(int pipe_id[]);
int count_appear(int arr[], int num);
void initialize_arr(int arr[]);
void create_children(int arr[], pid_t* status, pid_t children_status[], 
                     int pipe_descs[], int pipe_personal[][2]);
void close_parent_pipe(int pipe_descs[], int path_in_descs, 
                       int pipe_personal[][2], int path_in_personal);
int child_num(pid_t children_status[], pid_t status);
void catch_signal();
// ----------------------------------------------------------------------------

// -------------------------------main-----------------------------------------
int main()
{
    int arr[ARR_SIZE],   // for array of primes
        pipe_descs[2],                      // for all childs & parent: child 
        pipe_personal[NUM_OF_CHILDREN][2];  // pipe to each child with parent
    pid_t children_status[NUM_OF_CHILDREN],
        status;
    singal(SIGTERM ,catch_signal);

    initialize_arr(arr);
    open_pipe(pipe_descs);                  // open the general pipe
    create_children(arr, &status, children_status, pipe_descs, pipe_personal);

    return EXIT_SUCCESS;
}
// ----------------------------------------------------------------------------

// this function create all NUM_OF_CHILDREN and open personal pipe for each one
void create_children(int arr[], pid_t* status, pid_t children_status[],
    int pipe_descs[], int pipe_personal[][2])
{
    int i;              // for loop

    for (i = 0; i < NUM_OF_CHILDREN; ++i) {
        open_pipe(pipe_personal[i]);
        *status = fork();
        valid_status(*status);

        if (*status == 0)
            do_child(pipe_descs, pipe_personal[0]); // todo
            exit(EXIT_SUCCESS);

        children_status[i] = *status;        
    }

    do_parent(children_status, arr, pipe_descs, pipe_personal);
}
// ----------------------------------------------------------------------------

// this function is for parenth
// it read prime from child, check appearance of this prime in array and write 
// it to child. kill all children and print num of diffrent value in array
void do_parent(pid_t children_status[], int arr[], int pipe_descs[], 
               int pipe_personal[][2])
{
    int count_new_prime = 0,             // to count new value in array
        appearance = 0,                  // to count appearance of num in array
        i,                               // for loop
        msg[2];                          // { child_pid, prime }

    close_parent_pipe(pipe_descs, 1, pipe_personal, 0);

    for (i = 0; i < ARR_SIZE; ++i)
    {
        // read prime number from pipe (that child sent)
        read(pipe_descs[0], &msg, sizeof(int)*2);          
        arr[i] = msg.[1];
        appearance = count_appear(arr, msg.[1]);
        if (appearance == 0)
            ++count_new_prime;
        // writh to child the appearance of the num he sent
        write(pipe_personal[child_num(children_status, msg.[0])][1], 
              &appearance, sizeof(int));
    }

    kill_children(children_status);
    printf("Sun of diffrent value in array: %d\n", count_new_prime);
    close_parent_pipe(pipe_descs, 0, pipe_personal, 1);

    exit(EXIT_SUCCESS);
}
// ----------------------------------------------------------------------------

// this function is for children. in while loop it gets random num,
// check if it prime and if it is send the num to parent, read from parent the
// appearance of prime and when need to end it prints num of new prime it gave
void do_child(int pipe_descs[], int pipe_personal[]) 
{
    int count_new_prime = 0,            // for count new prime from this child
        num,                            // for rand num
        msg[2];                         // { child_pid, prime }

    srand(17);                          // for rand
    close(pipe_descs[0]);               // close pipe for reading
    close(pipe_personal[1]);            // close pipe for writeing
    
    while (!end)
    {
        num = rand() % 999 + 2;         // get random val
        if (is_prime(num))              // if is prime
        {
            msg[0] = getpid();
            msg[1] = num;            
            // sent to parent my id and prime
            write(pipe_descs[1], &msg, sizeof(int)*2);  
        }
        // read from pipe the appearence of the prime num in array      
        read(pipe_personal[0], &num, sizeof(int));     
        if (num == 0) ++count_new_prime;
    }

    printf("Sum of diffrent value in child %id: %d\n", getpid(), 
           count_new_prime);
    close(pipe_descs[1]);               // close pipe for writeing
    close(pipe_personal[0]);            // close pipe for reading
    exit(EXIT_SUCCESS);
}
// ----------------------------------------------------------------------------

// this function is a signal hendler
void catch_signal()
{
    end = 1;
}
// ----------------------------------------------------------------------------

// this function kill the children
void kill_children(pid_t children_status[])
{
    int i;

    for (i = 0; i < NUM_OF_CHILDREN; ++i)
    {
        kill(children_status[i], SIGTERM);
    }
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
        if (number % i == 0 && i != number)
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

// this function check if status us valid
void valid_status(pid_t status)
{
    if (status < 0)
    {
        perror("can not fork");
        exit(EXIT_FAILURE);
    }
}
// ----------------------------------------------------------------------------

// this function initialize arr value to 0
void initialize_arr(int arr[])
{
    int i;

    for (i = 0; i < ARR_SIZE; ++i)
    {
        arr[i] = 0;
    }
}
// ----------------------------------------------------------------------------

// this function close pipe in perenth
void close_parent_pipe(int pipe_descs[], int path_in_descs, 
                       int pipe_personal[][2], int path_in_personal)
{
    int i;

    close(pipe_descs[path_in_descs]);                      
    for (i = 0; i < NUM_OF_CHILDREN; ++i)
    {
        close(pipe_personal[i][path_in_personal]);         
    }
}
// ----------------------------------------------------------------------------

// this function count appearance of num in array and returns the result
int count_appear(int arr[], int num)
{
    int i,
        counter = 0;

    for (i = 0; i < ARR_SIZE && i != 0; ++i)
    {
        if (arr[i] == num)
            ++counter;
    }

    return counter;
}
// ----------------------------------------------------------------------------

// this function find status in children_status arr and returns child index
int child_num(pid_t children_status[], pid_t status)
{
    int i;

    for (i = 0; i < NUM_OF_CHILDREN; ++i)
    {
        if (children_status[i] == status)
            return i;
    }
    return 0;
}
// ----------------------------------------------------------------------------