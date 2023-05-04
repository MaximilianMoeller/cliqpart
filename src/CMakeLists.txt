# list source files here
set(sources main.cpp)

add_executable(${CMAKE_PROJECT_NAME} ${sources})

# Link Gurobi
if(CXX)
    target_link_libraries(${CMAKE_PROJECT_NAME} optimized ${GUROBI_CXX_LIBRARY}
            debug ${GUROBI_CXX_DEBUG_LIBRARY})
endif()
target_link_libraries(${CMAKE_PROJECT_NAME} ${GUROBI_LIBRARY})

