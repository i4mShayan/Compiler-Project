add_executable (ARK
  ARK.cpp
  CodeGen.cpp
  Lexer.cpp
  Parser.cpp
  Sema.cpp
  Optimizer.cpp
  )
target_link_libraries(ARK PRIVATE ${llvm_libs})
