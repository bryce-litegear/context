#include "context.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#ifndef USE_MALLOC

#define BLKS 64
#define BLK_SIZE (sizeof(uint64_t) * 32)
typedef uint64_t space_blk_t[32];

static space_blk_t space[BLKS];
static uint8_t in_use[BLKS];

static context_blk_t *allocate_space(size_t *needed)
{
    ldiv_t blks = ldiv(*needed, sizeof(space_blk_t));
    if (blks.rem)
    {
        blks.quot++;
    }
    *needed = blks.quot * sizeof(space_blk_t) ;
    for(int i=0; i<BLKS; i++)
    {
        if(in_use[i] == 0)
        {
            in_use[i] = 2;
            for(int j=i+1; j<BLKS; j++)
            {
                if(j == i+blks.quot)
                {
                    memset(&in_use[i], 1, blks.quot);
                    return (context_blk_t*)&space[i];
                }

                if(in_use[j]) 
                {
                    memset(&in_use[i], 0, j-i);
                    i=j;
                    break;
                }
                in_use[j] = 2;
            }
        }
    }
    return NULL;
}

void free_context_blk(context_blk_t *blk)
{
    ptrdiff_t index = (space_blk_t*)blk - space;  // pointer math
    assert(index >= 0);
    if(&space[index] == (space_blk_t*)blk) // validate pointer alignment 
    {
        memset(&in_use[index], 0, blk->size/sizeof(space_blk_t));
    }
    else
    {
        // run time error will lose memory block
        assert(&space[index] == (space_blk_t*)blk);
    }
}

#else
static context_blk_t *allocate_space(size_t *needed)
{
    context_blk_t *blk = malloc(needed);
    return blk;
}

size_t free_context_blk(context_blk_t *blk)
{
    free(blk);
}
#endif

context_blk_t *package_context(context_func_t func, void *user_context, size_t uc_size, size_t workspace)
{
    size_t total = sizeof(context_blk_t) + uc_size + workspace;
    context_blk_t *blk = allocate_space(&total);
    if(blk)
    {
        memcpy(blk, 
            &(context_blk_t){.target_func=func, .size=total, .useable=workspace},
            sizeof(context_blk_t));
        memcpy(blk->user_context, user_context, uc_size);
        memset(&((uint8_t*)blk->user_context)[uc_size], 0, workspace);
    }
    return blk;
}

void run_context(context_blk_t *blk)
{
    if(blk->target_func)
    {
        blk->target_func(blk);
    }
}

void run_context_and_free(context_blk_t *blk)
{
    if(blk->target_func)
    {
        blk->target_func(blk);
    }
    free_context_blk(blk);
}