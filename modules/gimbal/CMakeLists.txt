# Copyright (c) 2026 RobotPilots-SZU
# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(breeze_module_gimbal)
file(GLOB_RECURSE GIMBAL_SOURCES CONFIGURE_DEPENDS
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "src/*.cpp"
)
zephyr_library_sources(${GIMBAL_SOURCES})
target_include_directories(breeze_module_gimbal
    PUBLIC
        "inc"
	PRIVATE
		"src"
)

target_link_libraries(breeze_module_gimbal
    PRIVATE
		ramenx
		breeze::module::models
		breeze::module::common
)

add_library(breeze::module::gimbal ALIAS breeze_module_gimbal)
