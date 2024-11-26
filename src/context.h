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
    uint64_t user_context[];  // location of copied data and requested workspace
} context_blk_t;

/*
    Typical user context structure may contain a open block at the end for the 
    workspace.

    typedef struct user_struture_t
    {
        // the user data structure copied into the context_blk


        // the last element may be an open block as array definition
        uint8_t my_workspace[];
    } user_struture_t;

    In user function
    void my_function(context_blk_t *context)
    {
        // access to your copied data
        user_struture_t *my_struct = (user_struture_t *)context->user_context;
        // access to you requested workspace
        uint8_t *workspace = (uint8_t*)my_struct->my_workspace;
    
    
    }

*/

/**
 * @brief build a closure structure for a function and some data
 * 
 * @param func      function to be wrapped
 * @param user_context data to be copied into the closure (copied not ref)
 * @param uc_size   the size of the copied data
 * @param workspace   any additional workspace requested
 * @return context_blk_t* NULL if memory error or pointer to the closure.
 */
context_blk_t *package_context(context_func_t func, void *user_context, size_t uc_size, size_t workspace);

/**
 * @brief free an allocated closure structure
 * 
 * @param blk pointer to the closure structure
 */
void free_context_blk(context_blk_t *blk);

/**
 * @brief run the closure, run the enclosed function with the data originally copied 
 *      and the allocated memory. 
 * 
 * @param blk 
 */
void run_context(context_blk_t *blk);

/**
 * @brief run the closure once then free it.  Useful for sending
 *      closures through message queues.
 * 
 * @param blk pointer to closure structure.
 */
void run_context_and_free(context_blk_t *blk);

#endif // CONTEXT_H
