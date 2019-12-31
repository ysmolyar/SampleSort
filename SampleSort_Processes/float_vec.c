// Author: Nat Tuck
// CS3650 starter code

#include <stdlib.h>
#include <stdio.h>

#include "float_vec.h"

/*
typedef struct floats {
    long size;
    long cap;
    float* data;
} floats;
*/

floats*
make_floats()
{
    floats* xs = malloc(sizeof(floats));
    xs->size = 0;
    xs->cap  = 2;
    xs->data = malloc(xs->cap * sizeof(float));
    return xs;
}

void
floats_push(floats* xs, float xx)
{
    if (xs->size >= xs->cap) {
        xs->cap *= 2;
        xs->data = realloc(xs->data, xs->cap * sizeof(float));
    }

    xs->data[xs->size] = xx;
    xs->size += 1;
}

void
free_floats(floats* xs)
{
    free(xs->data);
    free(xs);
}

void
floats_print(floats* xs)
{
    for (int ii = 0; ii < xs->size; ++ii) {
        printf("%.04f ", xs->data[ii]);
    }
    printf("\n");
}

int
floats_cmp(const void* e1, const void* e2) 
{
    float f1 = *((float*) e1);
    float f2 = *((float*) e2);

    if (f1 > f2) {
        return 1;
    } else if (f1 < f2) {
        return -1;
    }
    return 0;
}


