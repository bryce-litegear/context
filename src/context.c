/**
 * @file context.c
 * @author Bryce Deary (bryce.deary@litegear.com)
 * @brief module to associate a function with a block of memory.
 * @date 2024-12-06
 * 
 * @copyright Copyright LightGear (2024)
 * 
 */
#include "context.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#ifndef USE_MALLOC  // a buffer allocation pool if not using malloc

#define BLKS 64  // allocate 64 possible blocks (16k)
typedef uint64_t space_blk_t[32];   
#define BLK_SIZE (sizeof(space_blk_t))

static space_blk_t space[BLKS];
static uint8_t in_use[BLKS];

/**
 * @brief static allocation function, allocate 1 or more
 *  blocks depending on needed.
 * 
 * @param needed require size in bytes
 * @return context_blk_t* 
 */
static context_blk_t *allocate_space(size_t *needed)
{
    ldiv_t blks = ldiv(*needed, sizeof(space_blk_t));
    if (blks.rem)
    {
        blks.quot++;
    }
    // extend the required to actual 
    *needed = blks.quot * sizeof(space_blk_t) ;
    for(int i=0; i<BLKS; i++)  // scan all available blocks
    {
        if(in_use[i] == 0) // if available
        {
            in_use[i] = 2;  // claim the first
            for(int j=i+1; j<BLKS; j++)
            {
                if(j == i+blks.quot)  // required was available
                {
                    memset(&in_use[i], 1, blks.quot);   // consolidate ownership
                    return (context_blk_t*)&space[i];
                }

                if(in_use[j]) // blocked by allocated block
                {
                    memset(&in_use[i], 0, j-i); // return temp claimed
                    i=j;    // start over
                    break;
                }
                in_use[j] = 2; // else do temp claim
            }
        }
    }
    return NULL; // could not allocate
}

/**
 * @brief take pointer to a blk and free its associated space if valid
 * 
 * @param blk pointer to blk
 */
void free_context_blk(context_blk_t *blk)
{
    // validate the pointer
    ptrdiff_t index = (space_blk_t*)blk - space;  // pointer math
    assert(index >= 0);
    // if pointer is valid, clear the in_use bits
    if(&space[index] == (space_blk_t*)blk) // validate pointer alignment 
    {
        memset(&in_use[index], 0, blk->size/sizeof(space_blk_t));
    }
    else // nothing we can do, just throw an assert
    {
        // run time error will lose memory block
        assert(&space[index] == (space_blk_t*)blk);
    }
}

#else  // we are using malloc
static context_blk_t *allocate_space(size_t *needed)
{
    context_blk_t *blk = malloc(*needed);
    return blk;  // make sure to test for NULL
}

size_t free_context_blk(context_blk_t *blk)
{
    free(blk);
}
#endif

context_blk_t *package_context(context_func_t func, void const *user_context, size_t uc_size, size_t workspace_size)
{
    size_t total = sizeof(context_blk_t) + uc_size + workspace_size;
    context_blk_t *blk = allocate_space(&total);
    workspace_size = total - (sizeof(context_blk_t) + uc_size);
    if(blk)
    {
        memcpy(blk, 
            &(context_blk_t){.target_func=func, 
                             .size=total, 
                             .workspace_size=workspace_size,
                             .original_context=user_context,
                             .workspace=&((uint8_t*)blk->user_context)[uc_size]},
            sizeof(context_blk_t));
        memcpy(blk->user_context, user_context, uc_size);
        memset(&((uint8_t*)blk->user_context)[uc_size], 0, workspace_size);
    }
    return blk;
}

void reset_context(context_blk_t *blk)
{
    memcpy(blk->user_context, blk->original_context, blk->size);
}

void reset_and_clear_context(context_blk_t *blk)
{
    reset_context(blk);
    memset(&((uint8_t*)blk->user_context)[blk->size], 0, blk->workspace_size);
}

void refresh_context(context_blk_t *blk, void const *user_context)
{
    memcpy(blk->user_context, blk->original_context, blk->size);
}

void refresh_and_clear_context(context_blk_t *blk, void const *user_context)
{
    refresh_context(blk,user_context);
    memset(&((uint8_t*)blk->user_context)[blk->size], 0, blk->workspace_size);
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

void reset_and_run_context(context_blk_t *blk)
{
    reset_context(blk);
    run_context(blk);
}