function(set_target_defaults target)
  set_target_properties(${target} PROPERTIES
    LINKER_LANGUAGE CXX
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
  )

  target_compile_options(${target} PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
  )

  # Prevent weird windows bug name clash thingie with fbs ???
  target_compile_definitions(${target} PRIVATE NOMINMAX)
endfunction()
