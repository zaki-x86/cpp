# Usage: target_apply_warnings(my_target)
function(target_apply_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /WX)
  else()
    target_compile_options(${target} PRIVATE
      -Wall -Wextra -Wpedantic -Werror
      -Wshadow -Wnon-virtual-dtor -Wold-style-cast
      -Wcast-align -Wunused -Woverloaded-virtual
      -Wconversion -Wsign-conversion -Wnull-dereference
      -Wdouble-promotion -Wformat=2
    )
  endif()
endfunction()
