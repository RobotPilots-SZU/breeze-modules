add_library(gimbal_test OBJECT
	./test.cpp
)
target_link_libraries(gimbal_test
	PRIVATE
		ramenx
		breeze::module::models
		breeze_module_gimbal
)

target_link_libraries(app PRIVATE
    gimbal_test
)
