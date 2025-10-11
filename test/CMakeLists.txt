set(TEST_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

set (cpptests
      legalactions undoactions other
    )
set (legalactions_parts 1 2)
set (undoactions_parts 1)
set (other_parts 1 2 3 4 5 6 7)

foreach(cpptest ${cpptests})
  set(cpptestsrc ${cpptestsrc} ${cpptest}test.cpp)
endforeach()

create_test_sourcelist(srclist test_runner.cpp ${cpptestsrc})
add_executable(test_runner ${srclist})
target_link_libraries(test_runner PRIVATE nichess)

foreach(cpptest ${cpptests})
  foreach(part ${${cpptest}_parts})
    add_test(test_${cpptest}_${part}
      ${TEST_PATH}/test_runner ${cpptest}test ${part})
    set_tests_properties(test_${cpptest}_${part} PROPERTIES
      FAIL_REGULAR_EXPRESSION "ERROR;FAIL;Test failed")
  endforeach()
endforeach()
