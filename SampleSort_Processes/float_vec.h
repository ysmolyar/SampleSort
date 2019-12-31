// Author: Nat Tuck
// CS3650 starter code

#ifndef FLOAT_VEC_H
#define FLOAT_VEC_H

typedef struct floats {
    long size;
    long cap;
    float* data;
} floats;

floats* make_floats();
void floats_push(floats* xs, float xx);
void free_floats(floats* xs);
void floats_print(floats* xs);
int floats_cmp(const void* e1, const void* e2);

#endif
