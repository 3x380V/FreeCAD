
target_include_directories(Sketcher_tests_run SYSTEM PUBLIC
    ${EIGEN3_INCLUDE_DIR}
    ${OCC_INCLUDE_DIR}
    ${PYCXX_INCLUDE_DIR}
    ${Python3_INCLUDE_DIRS}
    ${XercesC_INCLUDE_DIRS}
)
target_link_directories(Sketcher_tests_run PUBLIC ${OCC_LIBRARY_DIR})

target_link_libraries(Sketcher_tests_run
    gtest_main
    ${Google_Tests_LIBS}
    Sketcher
)

add_subdirectory(App)
