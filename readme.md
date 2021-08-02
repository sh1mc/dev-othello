# dev-othello Linux kernel module

## How to build
1. Set your environment variable as following:
```
export KERN_DIR=/path/to/kernel/src/root
```
2. Build the module with `make`.
3. Insert the module with `insmod`.

## How to play
- The path of the othello device is /dev/othello.
- It returns an ascii art of current othello board when it readed.
- You can put a disk wrighting `O` or `@` with proper offset.
- `@` IS A DARK DISK.

## Tips
- For example, to put a light disk on 20th (3-E) position, execute:
```
yes " " | head -n 19 | tr -d '\n' | sed -e 's/$/O\n/' > /dev/othello
```

