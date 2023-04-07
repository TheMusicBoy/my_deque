set(INCROOT ${PROJECT_SOURCE_DIR}/include/)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/)

set(SRC
    ${INCROOT}/values.hpp
    ${INCROOT}/allocator.hpp
)

add_library(deque-lib ${SRC})
target_include_directories(deque-lib PUBLIC ${INCROOT})
set_target_properties(deque-lib PROPERTIES LINKER_LANGUAGE CXX)