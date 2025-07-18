# Huffman Coding


## Build

Invoke `build.bat` or `build.sh`.

Manual:
```sh
gcc compress.c -o compress.exe
gcc decompress.c -o decompress.exe
```

## Usage Example

To compress a file `huffman.h` to a compressed `huffman.z`:
```sh
compress.exe huffman.h huffman.z
```
To decompress a compressed file `huffman.z` to an uncompressed file `recovered.h`:
```sh
decompress.exe huffman.z recovered.h
```
