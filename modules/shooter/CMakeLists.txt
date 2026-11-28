zephyr_library_named(breeze_shooter)

file(GLOB SHOOTER_SOURCE CONFIGURE_DEPENDS
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
zephyr_library_sources(${SHOOTER_SOURCE})

target_include_directories(breeze_shooter
    PUBLIC
        "inc"
	PRIVATE
		"src"
)
target_link_libraries(breeze_shooter
    PRIVATE
        ramenx
        robotpilots::algorithm
        breeze::module::models
)
