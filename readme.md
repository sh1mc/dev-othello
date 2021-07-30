# dev-othello kernel module for Linux

## How to build
1. Set your environment variable as following:
```
export KERN_DIR=/path/to/kernel/src/root
```
1. Build the module with `make`.
1. Insert the module with `insmod`.

## How to play
- The path of the othello device is /dev/othello.
- It returns an ascii art of current othello board when it readed.
- You can put a disk wrighting `O` or `@` with proper offset.
- `@` IS A DARK DISK.
