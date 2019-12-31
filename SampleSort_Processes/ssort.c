#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <float.h>
#include <string.h>

#include "float_vec.h"
#include "barrier.h"
#include "utils.h"

void
qsort_floats(floats* xs)
{
    // TODO: call qsort to sort the array
    // see "man 3 qsort" for details
    qsort(xs->data, xs->size, sizeof(float), floats_cmp);
}

floats*
sample(float* data, long size, int P)
{

    //- Randomly select 3*(P-1) items from the array
    floats* base_sample = make_floats();
    for (int i = 0; i < (3*(P-1)); i++) {
        int rand_idx = rand() % size;

        floats_push(base_sample, *((float*)data + rand_idx));
    }

    //- Sort those items
    qsort(base_sample->data, base_sample->size, sizeof(float), floats_cmp);
    floats* sample = make_floats();
    //add min float to the start of vector
    floats_push(sample, FLT_MIN);
    //Take the median of each group of 3 in the sorted array, producing
    //   an array (samples) of size P-1
    for (int i = 1; i < base_sample->size; i+=3) {
        floats_push(sample, base_sample->data[i]);
    } 
    //add max float to the end of vector
    floats_push(sample, FLT_MAX);

    free_floats(base_sample);
    return sample;
}

void
sort_worker(int pnum, float* data, long size, int P, floats* samps, long* sizes, barrier* bb)
{
    floats* xs = make_floats(); //local array of values to sort

    float lo = samps->data[pnum];
    float hi = samps->data[pnum+1];

    // select the floats to be sorted by this worker
    for (int i = 0; i < size; ++i) {  
        float val = data[i];  
        if (val <= hi && val > lo) {
            floats_push(xs, val);
        }
    }
    
    printf("%d: start %.04f, count %ld\n", pnum, samps->data[pnum], xs->size);
    //copy size of local array to its position in sizes array
    memcpy(sizes + pnum, &(xs->size), sizeof(long));
    //sort local array
    qsort_floats(xs);
    //barrier to avoid data race
    barrier_wait(bb);

    //calculate start and end of this partition relative to input
    long start = 0;
    long end = 0;
    if (pnum != 0) {
        for (long i = 0; i < pnum; i++) {
             start += sizes[i];
        }
    }

    end = start + sizes[pnum];

    //replace values in input with sorted values
    long j = 0;

    for (long i = start; i < end; i++) {
         data[i] = xs->data[j];
         ++j;
    }

    free_floats(xs);

    //terminate child process
    //exiting munmaps sizes,
    exit(0); 
}

void
run_sort_workers(float* data, long size, int P, floats* samps, long* sizes, barrier* bb)
{
    pid_t kids[P];
    (void) kids; // suppress unused warning

    // spawn P processes, each running sort_worker
    for (int pp = 0; pp < P; ++pp) {
        if ((kids[pp] = fork())) {

        } else {
            //call sort worker for each process
            sort_worker(pp, data, size, P, samps, sizes, bb);
             
        }
    }

    for (int ii = 0; ii < P; ++ii) {
        int rv = waitpid(kids[ii], 0, 0);
        check_rv(rv);
    }
}

void
sample_sort(float* data, long size, int P, long* sizes, barrier* bb)
{
    floats* samps = sample(data, size, P);
    run_sort_workers(data, size, P, samps, sizes, bb);
    free_floats(samps);
}

int
main(int argc, char* argv[])
{
    alarm(120);

    if (argc != 3) {
        printf("Usage:\n");
        printf("\t%s P data.dat\n", argv[0]);
        return 1;
    }

    const int P = atoi(argv[1]);
    const char* fname = argv[2];

    seed_rng();

    int rv;
    struct stat st;
    rv = stat(fname, &st);
    check_rv(rv);

    const int fsize = st.st_size;
    if (fsize < 8) {
        printf("File too small.\n");
        return 1;
    }

    int fd = open(fname, O_RDWR);
    check_rv(fd);
    //load file with mmap
    void* file = mmap(0, fsize, PROT_READ | PROT_WRITE,
                      MAP_SHARED, fd, 0); 
    (void) file; // suppress unused warning.

    //exit program if error on mmap
    if (*((long *) file) == -1) {
        printf("Error on mmap!");
        exit(1);
    }

    long count = *((long *) file);
    //mmap data so it can be accessed by processes
    float* data = mmap(0, count * sizeof(float), PROT_READ | PROT_WRITE, 
                       MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    //temporary pointer of mmap file to fill malloc data with float values
    void* ptr = file;
    ptr += 8; //skip first long value 

    //fill mmap'd data with values from file
    for (int i = 0; i < count; i++) {
        data[i] = *((float*) ptr);
        ptr +=4;
    }

    long sizes_bytes = P * sizeof(long);
    long* sizes = mmap(0, sizes_bytes, PROT_READ | PROT_WRITE,
                      MAP_SHARED| MAP_ANONYMOUS, -1, 0); 

    barrier* bb = make_barrier(P);

    sample_sort(data, count, P, sizes, bb);
    
    //replace file data with sorted data
    ptr = file;
    ptr += 8;
    for (int i = 0; i < count; i++) {
        *((float*) ptr) = data[i];
        ptr+=4;

    }    

    free_barrier(bb);
    
    //munmap your mmaps
      munmap(data, count);
      munmap(file, fsize);
      munmap(sizes, sizes_bytes);

    return 0;
}

