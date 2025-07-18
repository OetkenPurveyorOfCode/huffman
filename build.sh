mkdir build
pushd build
gcc ../compress.c -o compress
gcc ../decompress.c -o decompress 
popd
