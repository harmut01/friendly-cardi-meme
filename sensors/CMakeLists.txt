# Copyright (c) 2021 Harrison Mutai. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

add_library(max30102)

target_sources(max30102
    INTERFACE
        max30102.cpp
)

target_include_directories(max30102
    INTERFACE
        .
)