#pragma once

#define ASSERT(__expression)                                  \
    do {                                                      \
        if (!(__expression)) {                                \
            return Error::from_string_literal("ASSERT \"" #__expression "\" failed"); \
        }                                                     \
    } while (0)
