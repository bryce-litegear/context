/**
 * @file context.h
 * @author Bryce Deary (bryce.deary@litegear.com)
 * @brief Imagining a closure in C that can be passed around via a pointer.
 * @date 2024-11-26
 * 
 * @copyright Copyright LightGear (2024)
 * @details The idea for this module is to allow a function to be enclosed
 *  with a memory chunk and parameters in a data structure. The closure can be 
 *  rerun with the prior state retained. 
 * 
 * Example: Say you have an existing function you want to make portable so it 
 * can be queued for execution via a message queue.  
 * 
 * int my_found_function(int parm1, void *my_buffer, size_t buf_size);
 * 
 * so make a structure to support the parameters
 * 
 * typedef struct my_found_support_t
 * {
 *      int parm1;
 *      void *buffer;
 *      size_t buf_size;
 *      int ret_val;
 * } my_found_support_t;
 * 
 * // this is our wrapper function
 * void my_found_function_wrapper(context_blk_t *context)
 * {
 *      my_found_support_t *parms = (my_found_support_t *)context->user_context;    
 *      // use special allocated  buffer
 *      parms->buffer = context->workspace;
 *      parms->buf_size = context->workspace_size;
 *      parms->ret_val = my_found_function(parms->parm1, parms->buffer, parms->buf_size);
 * }
 * 
 * void main()
 * {
 *      my_found_support_t original = {.parm1 = 42};
 *      my_found_support_t *managed; 
 * 
 *      // assume we want a buffer of 64
 *      context_blk_t *blk = package_context(my_found_function_wrapper, &original, sizeof original, 64);
 *      assert(blk != NULL);
 * 
 *      // To run just
 *      run_context(blk);
 *  
 *      // data is at
 *      managed = (my_found_support_t*)blk->user_context;
 *      
 *      // now managed->ret_val will contain returned value each time you do run_context
 *      // managed->buffer is a pointer to the buffer or blk->workspace is also accessible
 *           
 * }
 * 
 */
#ifndef CONTEXT_H
#define CONTEXT_H
#include <stddef.h>
#include <stdint.h>

// fwd ref
typedef struct context_blk_t context_blk_t;
// wrapper prototype
typedef void (*context_func_t)(context_blk_t *context);
// the wrapper structure
typedef struct context_blk_t
{
    const context_func_t target_func;    // the function
    const void * const original_context; // pointer to original parameters
    void * const workspace;              // pointer to workspace
    const size_t size;                   // size of the parameters
    const size_t workspace_size;         // extra usable space
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
context_blk_t *package_context(context_func_t func, void const *user_context, size_t uc_size, size_t workspace);

/**
 * @brief free an allocated closure structure
 * 
 * @param blk pointer to the closure structure
 */
void free_context_blk(context_blk_t *blk);

/**
 * @brief run the closure, run the enclosed function with the data originally copied 
 *  and the allocated memory. Any changes to the original data or the workspace
 *  will be preserved on exit so if function is rerun it may continue processing
 *  based on state (user is responsible for the behaviour) 
 * 
 * @param blk 
 */
void run_context(context_blk_t *blk);

/**
 * @brief run the closure then free it.  Useful for sending
 *      closures through message queues.
 * 
 * @param blk pointer to closure structure.
 */
void run_context_and_free(context_blk_t *blk);


/**
 * @brief reset the context to original state using original pointer to user_context,
 *  and clear the workspace back to zero.
 *  This assumes that the original user_context was constant and still exists, or 
 *  is at least still meaningful. 
 * 
 * @param blk pointer to previous created context blk
 */
void reset_and_clear_context(context_blk_t *blk);
// same as above but does not clear the workspace
void reset_context(context_blk_t *blk);

/**
 * @brief update teh user context with a new set of values.  This does not
 *  reset the pointer to the original dataset, and does not clear the workspace.
 * 
 * @param blk - pointer to previously created context
 * @param user_context pointer to new set of data or original size
 */
void refresh_context(context_blk_t *blk, void const *user_context);
// same as above but also clear the workspace
void refresh_and_clear_context(context_blk_t *blk, void const *user_context);

/**
 * @brief reset the context to original state using original pointer to user_context and
 *  run the function. This assumes that the original user_context was constant 
 *  and still exists, or is at least still meaningful. 
 * 
 * @param blk 
 */
void reset_and_run_context(context_blk_t *blk);



#endif // CONTEXT_H
