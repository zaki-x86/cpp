# Sets C++23 as default with fallback detection
# Usage: require_cpp23(my_target)
function(require_cpp23 target)
  target_compile_features(${target} PRIVATE cxx_std_23)
  set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
endfunction()

function(require_cpp20 target)
  target_compile_features(${target} PRIVATE cxx_std_20)
  set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF)
endfunction()
