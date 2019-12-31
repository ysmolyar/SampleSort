#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <assert.h>
#include <pthread.h>

#include "float_vec.h"
#include "barrier.h"
#include "utils.h"

void
qsort_floats(floats* xs)
{
    // TODO: call qsort to sort the floats
}

floats*
sample(floats* input, int P)
{
    int kk = 27; // TODO: Probably wrong.
    floats* samps = make_floats(kk + 2);
    samps->data[0] = 0;
    samps->data[kk + 1] = INFINITY;

    // TODO: Sample some floats.

    return samps;
}

void
sort_worker(int pnum, floats* input, const char* output, int P, floats* samps, long* sizes, barrier* bb)
{
    floats* xs = make_floats(input->size);
    printf("%d: start %.04f, count %ld\n", pnum, samps->data[pnum], xs->size);

    // TODO: Build a local array

    // TODO: Sort it

    // TODO: open(2) the output file

    // TODO: lseek(2) to the right spot

    // TODO: Write your local array with write(2)

    free_floats(xs);
}

void
run_sort_workers(floats* input, const char* output, int P, floats* samps, long* sizes, barrier* bb)
{
    //pthread_t threads[P];

    // TODO: Spawn P threads running sort_worker

    // TODO: wait for all P threads to complete
}

void
sample_sort(floats* input, const char* output, int P, long* sizes, barrier* bb)
{
    floats* samps = sample(input, P);
    run_sort_workers(input, output, P, samps, sizes, bb);
    free_floats(samps);
}

int
main(int argc, char* argv[])
{
    alarm(120);

    if (argc != 4) {
        printf("Usage:\n");
        printf("\t%s P input.dat output.dat\n", argv[0]);
        return 1;
    }

    const int P = atoi(argv[1]);
    const char* iname = argv[2];
    const char* oname = argv[3];

    // TODO: remove this print
    printf("Sort from %s to %s.\n", iname, oname);

    seed_rng();

    floats* input = make_floats(0);
    // TODO: Open the input file and read the data into the input array.

    // TODO: Create the output file, of the same size, with ftruncate(2)
    // TODO: Write the size to the output file.

    int ofd = open(oname, O_CREAT|O_TRUNC|O_WRONLY, 0644);
    check_rv(ofd);

    long  n = 2;
    float x = 35.0;
    float y = 21.7;
    write(ofd, &n, sizeof(n));
    write(ofd, &x, sizeof(x)); // TODO: Don't write junk to the output file.
    write(ofd, &y, sizeof(y));

    int rv = close(ofd);
    check_rv(rv);

    barrier* bb = make_barrier(P);

    long* sizes = malloc(P * sizeof(long));
    sample_sort(input, oname, P, sizes, bb);

    free(sizes);

    free_barrier(bb);
    free_floats(input);

    return 0;
}

