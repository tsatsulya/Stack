#pragma once

#include <errno.h>
#include <stdio.h>

enum debug_modes {
    ERROR_ = 0,
    WARNING_ = 1, 
    INFO_ = 2, 
    DEBUG_ = 3, 
    ULTRA_MEGA_DEBUG_ = 4,
};


#define DEBUG_MODE DEBUG_
// TODO: maybe debug_exec
#define debug_msg(mode, my_func)                            \
    ({                                                      \
            if (mode <= DEBUG_MODE)                         \
                my_func;                                    \
    })

// TODO: Consider:
// #define debug_printf(mode, ...) debug_msg(mode, printf(__VA_ARGS__))

// For stuff like:
// debug_printf(ULTRA_MEGA_DEBUG, "Error caused by index %d out of bounds [%d, %d]", index, left, right)
