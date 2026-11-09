zephyr_library_named(breeze_module_communicator)

target_include_directories(breeze_module_communicator
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/inc/public"
	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/inc/private"
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)

target_link_libraries(breeze_module_communicator
	PRIVATE
        robotpilots::algorithm
		ramenx
		breeze::module::models
		rp_proto
)

zephyr_library_sources(${SOURCES})

add_library(breeze::module::communicator ALIAS breeze_module_communicator)
