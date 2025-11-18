cmake_minimum_required(VERSION 3.20)

# Simple RAG example
add_executable(simple_rag simple_rag.cpp)
target_link_libraries(simple_rag PRIVATE realm)

# Document retrieval example
add_executable(document_retrieval document_retrieval.cpp)
target_link_libraries(document_retrieval PRIVATE realm)

# Training example
add_executable(train_rag train_rag.cpp)
target_link_libraries(train_rag PRIVATE realm)

# MIPS benchmark
add_executable(mips_benchmark mips_benchmark.cpp)
target_link_libraries(mips_benchmark PRIVATE realm)
