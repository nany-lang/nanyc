set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(nany_compilation_database_folder "${CMAKE_CURRENT_BINARY_DIR}")

configure_file("cmake/you-complete-me.vim.template.cmake" ".ycm_extra_conf.py")
