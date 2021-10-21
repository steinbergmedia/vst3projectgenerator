cmake_minimum_required(VERSION 3.14.0)

# Check if git is installed and can be found
function(smtg_check_system)
    find_package(Git)
endfunction(smtg_check_system)
