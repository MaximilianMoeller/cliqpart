target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        main.cpp
        run_config.h
        run_config.cpp
        data.h
        data.cpp
        clique_part_model.h
        clique_part_model.cpp
        separator_factory.h
        separator_factory.cpp
        separators/abstract_separator.h
        separators/triangle_separator.h
        separators/triangle_separator.cpp
        separators/st_separator.h
        separators/st_separator.cpp
        ilp_callback.cpp
        ilp_callback.h
        )
