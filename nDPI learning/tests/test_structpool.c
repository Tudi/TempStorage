#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include "../src/buffer_pool.h"

#pragma pack(push, 1)
typedef struct __attribute__((packed)) mydummystruct {
    void* prev, * next;
    uint8_t val;
}mydummystruct;
#pragma pack(pop)

static void test_struct_pool_create() {
    StructPool* sp = struct_pool_create(sizeof(mydummystruct), 0);
    assert_non_null(sp);
    struct_pool_destroy(sp);
}

static void test_struct_pool_alloc() {
    StructPool* sp = struct_pool_create(sizeof(mydummystruct), 0);
    assert_non_null(sp);
    mydummystruct* elem = struct_pool_alloc(sp);
    assert_non_null(elem);
    struct_pool_free(sp, elem);
    struct_pool_destroy(sp);
}

static void test_struct_pool_alloc_many() {
    StructPool* sp = struct_pool_create(sizeof(mydummystruct), 0);
    assert_non_null(sp);
    for (size_t i = 0; i < 10000; i++) {
        mydummystruct* elem = struct_pool_alloc(sp);
        assert_non_null(elem);
        struct_pool_free(sp, elem);
    }
    struct_pool_destroy(sp);
}

static void test_struct_pool_alloc_alligned() {
    StructPool* sp = struct_pool_create(sizeof(mydummystruct), 0);
    assert_non_null(sp);
    for (size_t i = 0; i < 10000; i++) {
        mydummystruct* elem = struct_pool_alloc(sp);
        assert_non_null(elem);
        assert_int_equal(((uint64_t)elem) % USE_ALLIGNMENT_TO, 0);
        struct_pool_free(sp, elem);
    }
    struct_pool_destroy(sp);
}

static void test_struct_pool_bounds_checker() {
    StructPool* sp = struct_pool_create(sizeof(mydummystruct), 0);
    assert_non_null(sp);
    for (size_t i = 0; i < 10; i++) {
        mydummystruct* elem = struct_pool_alloc(sp);
        assert_non_null(elem);
        uint8_t* rawpointer = (uint8_t*)elem;
        rawpointer[-1] = 0xFF;
        // check we found the corruption
        assert_int_equal(struct_pool_free(sp, elem), 1);
    }
    for (size_t i = 0; i < 10; i++) {
        mydummystruct* elem = struct_pool_alloc(sp);
        assert_non_null(elem);
        uint8_t* rawpointer = (uint8_t*)elem;
        rawpointer[sizeof(mydummystruct) + 1] = 0xFF;
       // check we found the corruption
        assert_int_equal(struct_pool_free(sp, elem), 1);
    }

    // check a non allocated block
    assert_int_equal(struct_pool_free(sp, (void*)0xBADBAD), -1);

    struct_pool_destroy(sp);
}

void run_structpool_tests()
{
    init_pool_manager();
    test_struct_pool_create();
    test_struct_pool_alloc();
    test_struct_pool_alloc_many();
    test_struct_pool_alloc_alligned();
    test_struct_pool_bounds_checker();
    destroy_pool_manager();
}