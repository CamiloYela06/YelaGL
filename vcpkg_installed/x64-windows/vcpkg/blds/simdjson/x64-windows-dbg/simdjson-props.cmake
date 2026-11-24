target_include_directories("${target}" ${public} [==[$<BUILD_INTERFACE:C:/Users/yelac/Downloads/YelaGL/vcpkg_installed/x64-windows/vcpkg/blds/simdjson/src/v4.6.4-5f3085481b.clean/include>]==] ${private} [==[$<BUILD_INTERFACE:C:/Users/yelac/Downloads/YelaGL/vcpkg_installed/x64-windows/vcpkg/blds/simdjson/src/v4.6.4-5f3085481b.clean/src>]==])
target_compile_features("${target}" ${public} [==[cxx_std_11]==])
target_link_libraries("${target}" ${public} [==[Threads::Threads]==])
target_compile_definitions("${target}" ${public} [==[SIMDJSON_THREADS_ENABLED=1]==])
