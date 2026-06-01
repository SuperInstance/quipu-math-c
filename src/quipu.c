#include "quipu.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Encode a number as knots (base-10 positional) */
int encode_number(int n, Knot *out, size_t max_knots) {
    if (n < 0 || max_knots == 0) return 0;
    if (n == 0) { out[0].type = KNOT_EMPTY; out[0].value = 0; out[0].position = 0; return 1; }
    
    int digits[20];
    int count = 0;
    int temp = n;
    while (temp > 0 && count < 20) { digits[count++] = temp % 10; temp /= 10; }
    
    if ((size_t)count > max_knots) return 0;
    
    /* Reverse: most significant first */
    for (int i = 0; i < count; i++) {
        int d = digits[count - 1 - i];
        out[i].value = d;
        out[i].position = i;
        if (i == count - 1) {
            /* Units: use long knot for d>1, single for 1 */
            out[i].type = (d == 1) ? KNOT_SINGLE : KNOT_LONG;
        } else if (d == 1) {
            out[i].type = KNOT_SINGLE;
        } else if (d >= 2) {
            out[i].type = KNOT_FIGURE_EIGHT;
        } else {
            out[i].type = KNOT_EMPTY;
        }
    }
    return count;
}

/* Decode knots back to number */
int decode_number(const Knot *knots, size_t count) {
    int result = 0;
    for (size_t i = 0; i < count; i++) {
        result = result * 10 + knots[i].value;
    }
    return result;
}

/* Checksum: sum of values mod 10 */
int checksum(const Knot *knots, size_t count) {
    int sum = 0;
    for (size_t i = 0; i < count; i++) sum += knots[i].value;
    return sum % 10;
}

/* Cord lifecycle */
Cord cord_create(int color, size_t capacity) {
    Cord c;
    c.color = color;
    c.knots = (Knot *)calloc(capacity, sizeof(Knot));
    c.knot_count = 0;
    c.knot_capacity = capacity;
    c.position = 0;
    return c;
}

void cord_destroy(Cord *c) {
    free(c->knots);
    c->knots = NULL;
    c->knot_count = 0;
}

int cord_add_knot(Cord *c, KnotType type, int value) {
    if (c->knot_count >= c->knot_capacity) return -1;
    c->knots[c->knot_count].type = type;
    c->knots[c->knot_count].value = value;
    c->knots[c->knot_count].position = (int)c->knot_count;
    c->knot_count++;
    return 0;
}

char *cord_serialize(const Cord *c, size_t *out_len) {
    *out_len = c->knot_count * 12 + 32;
    char *buf = (char *)malloc(*out_len);
    int pos = 0;
    pos += sprintf(buf + pos, "C%d:", c->color);
    for (size_t i = 0; i < c->knot_count; i++) {
        pos += sprintf(buf + pos, "%d%d", c->knots[i].type, c->knots[i].value);
        if (i < c->knot_count - 1) pos += sprintf(buf + pos, ",");
    }
    *out_len = (size_t)pos;
    return buf;
}

/* Quipu lifecycle */
Quipu quipu_create(size_t pendant_capacity) {
    Quipu q;
    q.main_cord = cord_create(0, 32);
    q.pendants = (Cord *)calloc(pendant_capacity, sizeof(Cord));
    q.pendant_count = 0;
    q.pendant_capacity = pendant_capacity;
    return q;
}

void quipu_destroy(Quipu *q) {
    cord_destroy(&q->main_cord);
    for (size_t i = 0; i < q->pendant_count; i++) {
        cord_destroy(&q->pendants[i]);
    }
    free(q->pendants);
    q->pendants = NULL;
    q->pendant_count = 0;
}

int quipu_add_pendant(Quipu *q, int color) {
    if (q->pendant_count >= q->pendant_capacity) return -1;
    q->pendants[q->pendant_count] = cord_create(color, 16);
    q->pendants[q->pendant_count].position = (int)q->pendant_count;
    q->pendant_count++;
    return (int)(q->pendant_count - 1);
}

Cord *quipu_find_pendant(const Quipu *q, int position) {
    for (size_t i = 0; i < q->pendant_count; i++) {
        if (q->pendants[i].position == position) return &q->pendants[i];
    }
    return NULL;
}

size_t quipu_total_knots(const Quipu *q) {
    size_t total = q->main_cord.knot_count;
    for (size_t i = 0; i < q->pendant_count; i++) {
        total += q->pendants[i].knot_count;
    }
    return total;
}

/* Arithmetic: element-wise pendant operations */
Quipu quipu_add(const Quipu *a, const Quipu *b) {
    size_t n = a->pendant_count < b->pendant_count ? a->pendant_count : b->pendant_count;
    Quipu result = quipu_create(n);
    for (size_t i = 0; i < n; i++) {
        int va = decode_number(a->pendants[i].knots, a->pendants[i].knot_count);
        int vb = decode_number(b->pendants[i].knots, b->pendants[i].knot_count);
        quipu_add_pendant(&result, a->pendants[i].color);
        Knot knots[16];
        int count = encode_number(va + vb, knots, 16);
        for (int j = 0; j < count; j++) {
            cord_add_knot(&result.pendants[i], knots[j].type, knots[j].value);
        }
    }
    return result;
}

Quipu quipu_subtract(const Quipu *a, const Quipu *b) {
    size_t n = a->pendant_count < b->pendant_count ? a->pendant_count : b->pendant_count;
    Quipu result = quipu_create(n);
    for (size_t i = 0; i < n; i++) {
        int va = decode_number(a->pendants[i].knots, a->pendants[i].knot_count);
        int vb = decode_number(b->pendants[i].knots, b->pendants[i].knot_count);
        int diff = va - vb;
        if (diff < 0) diff = 0;
        quipu_add_pendant(&result, a->pendants[i].color);
        Knot knots[16];
        int count = encode_number(diff, knots, 16);
        for (int j = 0; j < count; j++) {
            cord_add_knot(&result.pendants[i], knots[j].type, knots[j].value);
        }
    }
    return result;
}

/* Corruption check */
CorruptionCheck quipu_check_corruption(const Quipu *original, const Quipu *copy) {
    CorruptionCheck check = {1, 1, 1};
    
    if (original->pendant_count != copy->pendant_count) {
        check.count_ok = 0;
        return check;
    }
    
    for (size_t i = 0; i < original->pendant_count; i++) {
        if (original->pendants[i].knot_count != copy->pendants[i].knot_count) {
            check.count_ok = 0;
        }
        if (original->pendants[i].color != copy->pendants[i].color) {
            check.value_ok = 0;
        }
        int cs_orig = checksum(original->pendants[i].knots, original->pendants[i].knot_count);
        int cs_copy = checksum(copy->pendants[i].knots, copy->pendants[i].knot_count);
        if (cs_orig != cs_copy) check.parity_ok = 0;
    }
    return check;
}

/* Weaving (categorical product encoding) */
long weave_values(int v1, int v2) {
    return (long)v1 * 10000L + v2;
}

void unweave_values(long woven, int *v1, int *v2) {
    *v2 = (int)(woven % 10000);
    *v1 = (int)(woven / 10000);
}
