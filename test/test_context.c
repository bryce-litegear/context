
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
    TEST_ASSERT_EQUAL(context->useable, 56);
    TEST_ASSERT_EQUAL(4, my_struct->u1);
}


void test_package_context(void)
{
    struct my_s_t my_struct = {4,3,2};

    context_blk_t *blk = package_context(ctx_func, &my_struct, sizeof my_struct, 56);
    TEST_ASSERT_EQUAL(56, blk->useable);
    TEST_ASSERT_EQUAL(3, ((struct my_s_t*)&blk->user_context)->u2);
    run_context_and_free(blk);
}

