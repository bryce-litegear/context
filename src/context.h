#ifndef CONTEXT_H
#define CONTEXT_H
#include <stddef.h>
#include <stdint.h>


typedef struct context_blk_t context_blk_t;

typedef void (*context_func_t)(context_blk_t *context);

typedef struct context_blk_t
{
    const context_func_t target_func;
    const size_t size;
    const size_t useable;
    uint64_t user_context[];  // location of copied data
} context_blk_t;

context_blk_t *package_context(context_func_t func, void *user_context, size_t uc_size, size_t workspace);
size_t free_context_blk(context_blk_t *blk);
void run_context(context_blk_t *blk);
void run_context_and_free(context_blk_t *blk);

#endif // CONTEXT_H
