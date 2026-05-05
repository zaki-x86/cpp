# Usage: target_apply_coverage(my_target)
# Then run: cmake --build . --target coverage
function(target_apply_coverage target)
  if(ENABLE_COVERAGE)
    target_compile_options(${target} PRIVATE --coverage -O0 -g)
    target_link_options(${target} PRIVATE --coverage)
  endif()
endfunction()

function(add_coverage_target)
  if(ENABLE_COVERAGE)
    find_program(LCOV lcov)
    find_program(GENHTML genhtml)
    if(LCOV AND GENHTML)
      add_custom_target(coverage
        COMMAND ${LCOV} --capture --directory . --output-file coverage.info
        COMMAND ${LCOV} --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage.info
        COMMAND ${GENHTML} coverage.info --output-directory coverage_report
        COMMAND ${CMAKE_COMMAND} -E echo "Coverage report: coverage_report/index.html"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating coverage report"
      )
    endif()
  endif()
endfunction()
