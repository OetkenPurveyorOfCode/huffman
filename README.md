# Huffman Coding


## Build

```sh
gcc compress.c -o compress.exe
gcc decompress.c -o decompress.exe
```

## Usage Example

To compress a file `main.c` to a compressed `main.c.z`:
```sh
compress.exe main.c main.c.z
```
To decompress a compressed file `main.c.z` to an uncompressed file `main_recovered.c`:
```sh
decompress.exe main.c.z main_recovered.c
```
