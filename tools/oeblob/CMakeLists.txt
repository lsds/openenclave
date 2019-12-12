# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

add_executable(oeblob main.c)

target_link_libraries(oeblob oehost)

# assemble into proper collector dir
set_property(TARGET oeblob PROPERTY RUNTIME_OUTPUT_DIRECTORY ${OE_BINDIR})

# install rule
install(TARGETS oeblob EXPORT openenclave-targets DESTINATION ${CMAKE_INSTALL_BINDIR})
