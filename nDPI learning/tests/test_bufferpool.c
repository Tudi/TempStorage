#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "../src/buffer_pool.h"

static void test_create_pool_manager() {
    PoolManager* mgr = pool_manager_create();
    assert_non_null(mgr);
    pool_manager_destroy(mgr);
}

static void test_create_pool() {
    PoolManager* mgr = pool_manager_create();
    assert_non_null(mgr);
    BufferPool* pool = pool_manager_pop_st(mgr);
    assert_non_null(pool);
    pool_manager_push_st(mgr, pool);
    pool_manager_destroy(mgr);
}

static void test_alloc_from_pool() {
    PoolManager* mgr = pool_manager_create();
    assert_non_null(mgr);
    BufferPool* pool = pool_manager_pop_st(mgr);
    assert_non_null(pool);

    for (size_t i = 0; i < 100; i++) {
        // alloc
        void* buff1 = alloc_pooled(pool, 5);
        // check allocated buffer allignment
        assert_int_equal(((uint64_t)buff1) % USE_ALLIGNMENT_TO, 0);
    }

    // free all allocated memory
    free_all_pooled(pool);

    pool_manager_push_st(mgr, pool);
    pool_manager_destroy(mgr);
}

static void test_bounds_check_pool() {
    PoolManager* mgr = pool_manager_create();
    assert_non_null(mgr);
    BufferPool* pool = pool_manager_pop_st(mgr);
    assert_non_null(pool);

    // create a mem corruption post alloc
    {
        const uint64_t alloc_size = 32;
        // alloc
        uint8_t* buff1 = (uint8_t *)alloc_pooled(pool, alloc_size);
        assert_non_null(buff1);

        // create a mem corruption post alloc
        buff1[alloc_size] = 0;
        assert_int_equal(check_bounds_pooled(pool, buff1), 1);
    }

    // create a mem corruption pre alloc
    {
        const uint64_t alloc_size = 32;
        // alloc
        uint8_t* buff1 = (uint8_t*)alloc_pooled(pool, alloc_size);
        assert_non_null(buff1);

        // create a mem corruption pre alloc
        buff1[-1] = 0xFF;
        assert_int_equal(check_bounds_pooled(pool, buff1), 1);
    }

    // check a non allocated block
    assert_int_equal(check_bounds_pooled(pool, (void*)0xBADBAD), -1);

    // free all allocated memory
    free_all_pooled(pool);

    pool_manager_push_st(mgr, pool);
    pool_manager_destroy(mgr);
}

void run_bufferpool_tests()
{
    init_pool_manager();
    test_create_pool_manager();
    test_create_pool();
    test_alloc_from_pool();
    test_bounds_check_pool();
    destroy_pool_manager();
}