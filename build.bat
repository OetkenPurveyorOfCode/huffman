mkdir build
pushd build
call clang -g -fsanitize=address,undefined ..\compress.c -o compress.exe 
call clang -g -fsanitize=address,undefined ..\decompress.c -o decompress.exe 
popd
