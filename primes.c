#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MIN 2
#define MAX 35


// intentionally used 4 not size of, everywhere 
void write_to_fd(int pipe_end, int start, int end)
{
    for (int i = start; i < end; ++i)
    {
        write(pipe_end, &i, 4);
    }
}

void read_and_filter(int read_end, int write_end, int min)
{
    int num;
    while (read(read_end, &num, 4) != 0)
    {
        if (num % min)
        {
            write(write_end, &num, 4);
        }
    }
}

int main(int argc, char *argv[])
{
    int left_pipe[2];

    pipe(left_pipe);
    int current_prime = MIN;

    write_to_fd(left_pipe[1], MIN, MAX);
    close(left_pipe[1]);

    while (fork() == 0)
    {
        if (read(left_pipe[0], &current_prime, 4) != 0)
        {
            printf("prime is %d\n", current_prime);
        }
        else
        {
            exit(0);
        }
        int right_pipe[2];
        pipe(right_pipe);
        read_and_filter(left_pipe[0], right_pipe[1], current_prime);
        close(right_pipe[1]);

        //  put the old pipes which were inherited as the   new pipes
        left_pipe[0] = right_pipe[0];
        left_pipe[1] = right_pipe[1];
    }
    close(left_pipe[0]);
    wait(0);
    exit(0);
}
