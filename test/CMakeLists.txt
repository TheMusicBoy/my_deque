set(SRC 

)

add_executable(deque-tests ${SRC})
target_link_libraries(deque-tests PUBLIC deque-lib)
set_target_properties(deque-tests PROPERTY LINKER_LANGUAGE CXX)