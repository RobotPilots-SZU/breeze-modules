add_library(b2b_test OBJECT
	./test.cpp
)
target_link_libraries(b2b_test
	PRIVATE
		breeze_module_b2b
)

target_link_libraries(app PRIVATE b2b_test)
