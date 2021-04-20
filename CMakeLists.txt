cmake_minimum_required(VERSION 2.8)

project(dataset-tools)


find_package( OpenCV REQUIRED )

set(DEPENDENCIES ${OpenCV_LIBS})

add_executable(dataset-cropper "src/dataset-cropper.cpp")
target_link_libraries(dataset-cropper stdc++fs ${DEPENDENCIES})

add_executable(dataset-selector "src/dataset-selector.cpp")
target_link_libraries(dataset-selector stdc++fs ${DEPENDENCIES})
