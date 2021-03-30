cmake_minimum_required(VERSION 2.8)

project(dataset-cropper)


find_package( OpenCV REQUIRED )

set(DEPENDENCIES ${OpenCV_LIBS})

add_executable(${PROJECT_NAME} "main.cpp")
target_link_libraries(${PROJECT_NAME} stdc++fs ${DEPENDENCIES})
