zephyr_library_named(breeze_module_b2b)

target_include_directories(breeze_module_b2b
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/inc"
)

target_sources(breeze_module_b2b PRIVATE
	"./src/transmit.cpp"
)

target_link_libraries(breeze_module_b2b PRIVATE
	ramenx
)

add_library(breeze::module::b2b ALIAS breeze_module_b2b)
