# quipu-math-c

> Incan knotted cord mathematics in C — encode, decode, and compute with quipu numbers.

## What This Does

`quipu-math-c` implements the Incan quipu number system in C. Encode decimal numbers as knot sequences, decode them back, compute checksums, build cord trees, perform arithmetic, and serialize/deserialize quipu data. Use it for educational tools, novel encodings, or embedded systems that need cultural math.

## The Cultural Root

See `quipu-math` (PyPI) for full cultural background. Quipu encode decimal numbers as knotted cords — a tactile positional number system.

## Install

```bash
git clone https://github.com/SuperInstance/quipu-math-c.git
cd quipu-math-c
make
```

## Quick Start

```c
#include "quipu.h"

int main() {
    // Encode a number as knots
    Knot knots[16];
    int count = encode_number(135, knots, 16);

    // Decode back
    int value = decode_number(knots, count);
    printf("Decoded: %d\n", value);  // 135

    // Checksum
    int cs = checksum(knots, count);
    printf("Checksum: %d\n", cs);  // 9

    // Build a quipu (cord tree)
    Quipu *q = quipu_create();
    int cord1 = quipu_add_pendant(q, /*color=*/1);
    int cord2 = quipu_add_pendant(q, /*color=*/2);

    // Weaving
    long woven = weave_values(7, 3);
    int v1, v2;
    unweave_values(woven, &v1, &v2);

    // Serialize
    size_t json_len;
    char *json = quipu_serialize(q, &json_len);
    printf("%s\n", json);

    quipu_destroy(q);
    return 0;
}
```

## API Reference

### Knot Encoding
```c
typedef enum { KNOT_SINGLE, KNOT_FIGURE_EIGHT, KNOT_LONG, KNOT_EMPTY } KnotType;

typedef struct {
    KnotType type;
    int value;
    int position;
    int turns;
} Knot;
```

- `int encode_number(int n, Knot *out, size_t max_knots)` — Encode n as knot sequence, returns count
- `int decode_number(const Knot *knots, size_t count)` — Decode knots to integer
- `int checksum(const Knot *knots, size_t count)` — Sum of digit values

### Cord
- `Cord *cord_create(int color, int position)` — Create a cord
- `void cord_destroy(Cord *c)`
- `int cord_add_knot(Cord *c, KnotType type, int value)` — Add a knot
- `char *cord_serialize(const Cord *c, size_t *out_len)` — JSON serialize

### Quipu (Cord Tree)
- `Quipu *quipu_create()` — Create empty quipu
- `void quipu_destroy(Quipu *q)`
- `int quipu_add_pendant(Quipu *q, int color)` — Add pendant cord, returns index
- `char *quipu_serialize(const Quipu *q, size_t *out_len)` — JSON serialize

### Weaving
- `long weave_values(int v1, int v2)` — Interleave two values
- `void unweave_values(long woven, int *v1, int *v2)` — Split back

## License

MIT
