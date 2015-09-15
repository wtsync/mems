# mems

this is a tool to easily change kernel space memory. 

## usage

```bash
mems 0x101100c0 0x08002000
```

write kernel address `0x101100c0` to value `0x08002000`

## build 

```bash
mipsel-openwrt-linux-gcc -g mems.c -o mems
```
