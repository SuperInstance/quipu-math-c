#ifndef QUIPU_MATH_H
#define QUIPU_MATH_H

#include <stddef.h>

/* Knot types */
typedef enum { KNOT_EMPTY = 0, KNOT_SINGLE = 1, KNOT_FIGURE_EIGHT = 2, KNOT_LONG = 3 } KnotType;

/* A single knot */
typedef struct {
    KnotType type;
    int value;       /* 0-9 encoded value */
    int position;    /* Position on cord */
} Knot;

/* A cord with knots */
typedef struct {
    int color;       /* Color index */
    Knot *knots;
    size_t knot_count;
    size_t knot_capacity;
    int position;    /* Pendant position */
} Cord;

/* A quipu (cord tree) */
typedef struct {
    Cord main_cord;
    Cord *pendants;
    size_t pendant_count;
    size_t pendant_capacity;
} Quipu;

/* Knot operations */
int encode_number(int n, Knot *out, size_t max_knots);
int decode_number(const Knot *knots, size_t count);
int checksum(const Knot *knots, size_t count);

/* Cord operations */
Cord cord_create(int color, size_t capacity);
void cord_destroy(Cord *c);
int cord_add_knot(Cord *c, KnotType type, int value);
char *cord_serialize(const Cord *c, size_t *out_len);

/* Quipu operations */
Quipu quipu_create(size_t pendant_capacity);
void quipu_destroy(Quipu *q);
int quipu_add_pendant(Quipu *q, int color);
Cord *quipu_find_pendant(const Quipu *q, int position);
size_t quipu_total_knots(const Quipu *q);

/* Arithmetic */
Quipu quipu_add(const Quipu *a, const Quipu *b);
Quipu quipu_subtract(const Quipu *a, const Quipu *b);

/* Error detection */
typedef struct { int value_ok; int count_ok; int parity_ok; } CorruptionCheck;
CorruptionCheck quipu_check_corruption(const Quipu *original, const Quipu *copy);

/* Weaving (categorical product) */
long weave_values(int v1, int v2);
void unweave_values(long woven, int *v1, int *v2);
#endif
