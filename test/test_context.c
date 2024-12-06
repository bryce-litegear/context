
#include "unity.h"

#include "context.h"

void setUp(void)
{
}

void tearDown(void)
{
}

struct my_s_t{
    uint32_t u1;
    int32_t u2;
    uint16_t u3;
};

void ctx_func(context_blk_t *context)
{
    struct my_s_t *my_struct = (struct my_s_t *)context->user_context;
    TEST_ASSERT_GREATER_THAN(56, context->workspace_size);
    TEST_ASSERT_EQUAL(4, my_struct->u1);
    my_struct->u2++;
}


void test_package_context(void)
{
    struct my_s_t my_struct = {4,3,2};

    context_blk_t *blk = package_context(ctx_func, &my_struct, sizeof my_struct, 56);
    #ifdef USE_MALLOC
    TEST_ASSERT_EQUAL(56, blk->useable);
    #else
    TEST_ASSERT_EQUAL(256 - (sizeof(context_blk_t)+sizeof my_struct), blk->workspace_size);
    #endif
    TEST_ASSERT_EQUAL(3, ((struct my_s_t*)&blk->user_context)->u2);
    run_context_and_free(blk);
}


void test_context_reuse(void)
{
    struct my_s_t my_struct = {4,3,2};

    context_blk_t *blk = package_context(ctx_func, &my_struct, sizeof my_struct, 56);
    TEST_ASSERT_GREATER_THAN(56, blk->workspace_size);
    TEST_ASSERT_EQUAL(3, ((struct my_s_t*)&blk->user_context)->u2);
    run_context(blk);
    TEST_ASSERT_EQUAL(4,((struct my_s_t *)blk->user_context)->u2);
    run_context(blk);
    TEST_ASSERT_EQUAL(5,((struct my_s_t *)blk->user_context)->u2);    
    free_context_blk(blk);
}

