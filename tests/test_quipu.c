#define _POSIX_C_SOURCE 200809L
#include "../src/quipu.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void test_encode_decode() {
    Knot knots[16];
    int count = encode_number(135, knots, 16);
    assert(count == 3);
    assert(knots[0].value == 1);
    assert(knots[1].value == 3);
    assert(knots[2].value == 5);
    int decoded = decode_number(knots, count);
    assert(decoded == 135);
    printf("  PASS: encode_decode\n");
}

void test_encode_zero() {
    Knot knots[16];
    int count = encode_number(0, knots, 16);
    assert(count == 1);
    assert(knots[0].type == KNOT_EMPTY);
    printf("  PASS: encode_zero\n");
}

void test_encode_single_digit() {
    Knot knots[16];
    int count = encode_number(7, knots, 16);
    assert(count == 1);
    assert(knots[0].value == 7);
    assert(knots[0].type == KNOT_LONG); /* last digit > 1 */
    printf("  PASS: encode_single_digit\n");
}

void test_checksum() {
    Knot knots[4] = {{KNOT_FIGURE_EIGHT, 3, 0}, {KNOT_SINGLE, 5, 1}, {KNOT_LONG, 2, 2}};
    assert(checksum(knots, 3) == 0); /* 3+5+2=10, 10%10=0 */
    printf("  PASS: checksum\n");
}

void test_cord_basic() {
    Cord c = cord_create(1, 10);
    assert(c.knot_count == 0);
    cord_add_knot(&c, KNOT_FIGURE_EIGHT, 4);
    cord_add_knot(&c, KNOT_SINGLE, 2);
    assert(c.knot_count == 2);
    
    size_t len;
    char *ser = cord_serialize(&c, &len);
    assert(ser != NULL);
    assert(strstr(ser, "C1:") != NULL);
    free(ser);
    cord_destroy(&c);
    printf("  PASS: cord_basic\n");
}

void test_quipu_create() {
    Quipu q = quipu_create(8);
    assert(q.pendant_count == 0);
    assert(q.pendant_capacity == 8);
    quipu_destroy(&q);
    printf("  PASS: quipu_create\n");
}

void test_quipu_pendants() {
    Quipu q = quipu_create(8);
    int p1 = quipu_add_pendant(&q, 1);
    int p2 = quipu_add_pendant(&q, 2);
    assert(p1 == 0);
    assert(p2 == 1);
    assert(q.pendant_count == 2);
    
    Cord *found = quipu_find_pendant(&q, 0);
    assert(found != NULL);
    assert(found->color == 1);
    
    assert(quipu_total_knots(&q) == 0);
    quipu_destroy(&q);
    printf("  PASS: quipu_pendants\n");
}

void test_quipu_arithmetic() {
    Quipu a = quipu_create(4);
    quipu_add_pendant(&a, 1);
    encode_number(25, a.pendants[0].knots, a.pendants[0].knot_capacity);
    a.pendants[0].knot_count = 2;
    a.pendants[0].knots[0] = (Knot){KNOT_FIGURE_EIGHT, 2, 0};
    a.pendants[0].knots[1] = (Knot){KNOT_LONG, 5, 1};
    
    Quipu b = quipu_create(4);
    quipu_add_pendant(&b, 1);
    b.pendants[0].knot_count = 1;
    b.pendants[0].knots[0] = (Knot){KNOT_SINGLE, 3, 0};
    
    Quipu sum = quipu_add(&a, &b);
    assert(sum.pendant_count == 1);
    int val = decode_number(sum.pendants[0].knots, sum.pendants[0].knot_count);
    assert(val == 28); /* 25 + 3 */
    
    quipu_destroy(&a);
    quipu_destroy(&b);
    quipu_destroy(&sum);
    printf("  PASS: quipu_arithmetic\n");
}

void test_quipu_subtract() {
    Quipu a = quipu_create(4);
    quipu_add_pendant(&a, 1);
    a.pendants[0].knot_count = 2;
    a.pendants[0].knots[0] = (Knot){KNOT_FIGURE_EIGHT, 5, 0};
    a.pendants[0].knots[1] = (Knot){KNOT_LONG, 0, 1};
    
    Quipu b = quipu_create(4);
    quipu_add_pendant(&b, 1);
    b.pendants[0].knot_count = 1;
    b.pendants[0].knots[0] = (Knot){KNOT_SINGLE, 2, 0};
    
    Quipu diff = quipu_subtract(&a, &b);
    int val = decode_number(diff.pendants[0].knots, diff.pendants[0].knot_count);
    assert(val == 48); /* 50 - 2 */
    
    quipu_destroy(&a);
    quipu_destroy(&b);
    quipu_destroy(&diff);
    printf("  PASS: quipu_subtract\n");
}

void test_corruption_check() {
    Quipu orig = quipu_create(4);
    quipu_add_pendant(&orig, 1);
    orig.pendants[0].knot_count = 2;
    orig.pendants[0].knots[0] = (Knot){KNOT_FIGURE_EIGHT, 3, 0};
    orig.pendants[0].knots[1] = (Knot){KNOT_LONG, 7, 1};
    
    Quipu copy = quipu_create(4);
    quipu_add_pendant(&copy, 1);
    copy.pendants[0].knot_count = 2;
    copy.pendants[0].knots[0] = (Knot){KNOT_FIGURE_EIGHT, 3, 0};
    copy.pendants[0].knots[1] = (Knot){KNOT_LONG, 7, 1};
    
    CorruptionCheck ok = quipu_check_corruption(&orig, &copy);
    assert(ok.value_ok);
    assert(ok.count_ok);
    assert(ok.parity_ok);
    
    /* Corrupt */
    copy.pendants[0].knots[1].value = 9;
    CorruptionCheck bad = quipu_check_corruption(&orig, &copy);
    assert(!bad.parity_ok);
    
    quipu_destroy(&orig);
    quipu_destroy(&copy);
    printf("  PASS: corruption_check\n");
}

void test_weave_unweave() {
    long woven = weave_values(42, 17);
    int v1, v2;
    unweave_values(woven, &v1, &v2);
    assert(v1 == 42);
    assert(v2 == 17);
    printf("  PASS: weave_unweave\n");
}

void test_encode_decode_roundtrip() {
    for (int n = 0; n <= 999; n += 37) {
        Knot knots[16];
        int count = encode_number(n, knots, 16);
        int decoded = decode_number(knots, count);
        assert(decoded == n);
    }
    printf("  PASS: roundtrip\n");
}

int main() {
    printf("Running quipu-math-c tests:\n");
    test_encode_decode();
    test_encode_zero();
    test_encode_single_digit();
    test_checksum();
    test_cord_basic();
    test_quipu_create();
    test_quipu_pendants();
    test_quipu_arithmetic();
    test_quipu_subtract();
    test_corruption_check();
    test_weave_unweave();
    test_encode_decode_roundtrip();
    printf("\n12 passed, 0 failed\n");
    return 0;
}
