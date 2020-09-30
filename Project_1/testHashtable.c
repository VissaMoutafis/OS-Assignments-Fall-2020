#include "acutest.h"
#include "Types.h"
#include "HT.h"
#include <stdlib.h>
Pointer create_int(int i)
{
    int *p = malloc(sizeof(int));
    *p = i;
    return (Pointer) p;
}

int compare(Pointer a, Pointer b)
{
    return *(int*)a - *(int*)b;
}

size_t hash(Pointer i)
{
    return (*(int*)i) * 25 + 2;
}

void test100(void)
{
    HT h = ht_create(compare, hash, free, 100);

    for (size_t i=0; i < 100; ++i)
    {
        ht_insert(h, create_int(i));
    }
    
    for (size_t i = 0; i < 100; ++i)
    {
        Pointer p = create_int(i), pk;
        TEST_CHECK(ht_contains(h, p, &pk) == true && compare(pk, p) == 0);
        free(p);
    }

    for (size_t i = 25; i < 50; ++i)
    {
        Pointer p = create_int(i), pk;
        TEST_CHECK(ht_contains(h, p, &pk) == true && compare(pk, p) == 0);
        ht_delete(h, p, true, &pk);
        TEST_CHECK(ht_contains(h, p, &pk) == false);
        free(p);
    }
    ht_destroy(h);
}

TEST_LIST =
{
    {"Testing 100 inserts", test100},
    {NULL, NULL}
};