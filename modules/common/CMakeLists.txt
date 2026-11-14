# Copyright (c) 2026 RobotPilots-SZU
# SPDX-License-Identifier: Apache-2.0

zephyr_interface_library_named(breeze_module_common)
target_include_directories(breeze_module_common
    INTERFACE
        "inc"
)

target_link_libraries(breeze_module_common
    INTERFACE
		ramenx
		breeze::module::models
)

add_library(breeze::module::common ALIAS breeze_module_common)
