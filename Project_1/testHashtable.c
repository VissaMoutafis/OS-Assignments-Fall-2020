#include "AVL.h"
// #include "acutest.h"
#include <stdio.h>
#include <stdlib.h>

Pointer createInt(int n)
{
    Pointer pn = malloc(sizeof(int));
    *(int *)pn = n;

    return pn;
}

int compare(Pointer a, Pointer b)
{
    return *(int *)a - *(int *)b;
}

void test_insert(void)
{
    AVL t = avl_create(compare, free);
    // TEST_CHECK(avl_count(t) == 0 && avl_empty(t) == true);

    int n = 10;

    for (int i = 0; i < n; i++)
    { printf("Inserting : %d\n", i);
        avl_insert(t, createInt(i));
        // TEST_CHECK(avl_count(t) == i + 1);
        // TEST_CHECK(*(int *)avl_node_get_key(t, avl_find(t, createInt(i))) == i);
    }

    // TEST_CHECK(avl_count(t) == n);
    printf("Destroy...\n");
    avl_destroy(t);
}

void test_delete(void)
{
    AVL t = avl_create(compare, free);
    // TEST_CHECK(avl_count(t) == 0 && avl_empty(t) == true);

    int n = 10;

    for (int i = 0; i < n; i++)
    {
        avl_insert(t, createInt(i));
        // TEST_CHECK(avl_count(t) == i + 1);
        // TEST_CHECK(avl_find(t, createInt(i)) != NULL);
    }

    // TEST_CHECK(avl_count(t) == n);

    for (int i = 0; i < n; i++)
    {
        Pointer old;
        avl_delete(t, createInt(i), true, &old);
        // TEST_CHECK(avl_count(t) == n - 1 - i);
        // TEST_CHECK(avl_find(t, createInt(i)) == NULL);
    }

    avl_destroy(t);
    return;
}

int main() {
    // test_insert();
    test_delete();
}

// TEST_LIST =
//     {
//         {"Insert Function", test_insert},
//         {"Delete Function", test_delete},
//         {NULL, NULL}};