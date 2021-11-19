/*
* 
* 
* 
*/
// ----------------------------------------------
#include <sys/wait.h> 
#include <sys/types.h>
#include <unsitd.h>
#include <signal.h>

#define ARR_SIZE 1000
#define NUM_OF_CHILDREN 3

int end = 0;

void valid_status(pid_t status);
void do_parent(pid_t status);
void do_child();
int is_prime(int number);
void open_pipe(int pipe_id);
int count_appear(int &appearance, int num); // todo
// -----------------------------------------------

int main()
{
    int arr[ARR_SIZE],                      // for array of primes
        pipe_descs[2],                      // for all childs & parent: child 
        pipe_personal[NUM_OF_CHILDREN][2];  // uniq pipe to each child with his parent
    pid_t status;                                           

    open_pipe(pipe_descs);                  // open the general pipe

    for (i = 0; i < NUM_OF_CHILDREN; ++i) { // create the childs (maybe we should do it not in loop)
        status = fork();
        valid_status(status);               
        open_pipe(pipe_personal[i]);        // open the uniq pipe

        if (status == 0)
            do_child(pipe_descs, pipe_personal[i]);
        else if (status > 0)
            do_parent(pipe_descs, pipe_personal);
    }

    return EXIT_SUCCESS;
}
// -----------------------------------------------

void do_parent(pid_t status, int arr)
{
    int count_new_prime = 0,                // to count new value in array
        appearance = 0,                     // to count appearance of some value in array
        nbytes                              
        i,
        childs_status;                      // not using yet

    close(pipe_descs[1]);                   // close writh path in pipe descs
    for (i = 0; i < NUM_OF_CHILDREN; ++i)
        close(pipe_personal[i][0]);         // close read path in personal descs

    for (i = 0; i < ARR_SIZE; ++i)
    {
        nbytes = read(pipe_descs[0], arr[i], sizeof(int));          // read prime number from pipe (that child sent)
        if (count_appear(appearance, arr[i]) == 0)                  
            ++count_new_prime;
        // sent appearance to child
        write(pipe_personal[1], appearance, sizeof(int));           // writh to child the appearance of the num he sent
    }
    
    kill_childs(childs_status);             // kill the children - todo: think how to pass the status of all the childs

    printf("Sun of diffrent value in array: %d\n" count_new_prime);     
    close(pipe_descs[0]);                   
    for (i = 0; i < NUM_OF_CHILDREN; ++i)
        close(pipe_personal[i][1]);

    return EXIT_SUCCESS;
}
// -----------------------------------------------

// this function send to children they need to end
void kill_childs(int childs_status)
{
    end = 1;
    //for (i = 0; i < NUM_OF_CHILDREN; ++i)
    //  kill(childs_status[i], SIGUSR1);
}

/*
void catch_sigusr1(int sig_num) {
    signal(SIGUSR1, catch_sigusr1);

    printf(" process %d caught signal SIGUSR1\n", getpid());
}
// -----------------------------------------------
*/

void do_child() // pipe_descs: writh   pipe_personal: read
{
    int count_new_prime = 0,            // to count new value that child give to his parent
        num;                            // for random num and to read msg from parent
    srand(17);                          // for rand

    close(pipe_descs[0]);               // close read path in pipe descs
    close(pipe_personal[1]);            // close writh path in pipe_personal 

    while (!end)
    {
        num = rand()%999+2;             // get random val
        if (is_prime(num))              // if is prime
            write(pipe_descs[1], num, sizeof(int); // + getpid()  // sent num && id to parent

        read(pipe_descs[0], num, sizeof(int));     // read from pipe the appearence of the num in array      
        if (num == 0) ++count_new_prime;
    }

    // things that need to happend when parent sent "end"
    printf("Sun of diffrent value in child 1: %d\n" count_new_prime);
    close(pipe_descs[1]);
    close(pipe_personal[0]);

    return(EXIT_SUCCESS);
}
// -----------------------------------------------

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

void open_pipe(int pipe_id)
{
    if (pipe(pipe_id) == -1) {
        perror("cannot open pipe");
        exit(EXIT_FAILURE);
    }
}
// ----------------------------------------------------------------------------

void valid_status(pid_t status)
{
    if (status < 0)
    {
        perror("can not fork");
        exit(EXIT_FAILURE);
    }
}
// -----------------------------------------------