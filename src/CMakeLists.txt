target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        main.cpp
        verbosity.h
        separator_factory.h
        separator_factory.cpp
        complete_graph.h
        complete_graph.cpp
        run_config.h
        run_config.cpp
        separators/abstract_separator.h
        separators/cubic_triangle_separator.h
        separators/cubic_triangle_separator.cpp
        )
