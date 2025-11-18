# Example programs

add_executable(basic_gru basic_gru.cpp)
target_link_libraries(basic_gru PRIVATE gru_lib)

add_executable(sequence_processing sequence_processing.cpp)
target_link_libraries(sequence_processing PRIVATE gru_lib)

add_executable(sentiment_analysis sentiment_analysis.cpp)
target_link_libraries(sentiment_analysis PRIVATE gru_lib)
