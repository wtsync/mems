#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define PAGE_SIZE    4096

void hexprint(const unsigned char* s, int size)
{
    int i = 0, n = 0, c = 0, p = 0;

    p += fprintf(stdout, "ADDR    : +0 +1 +2 +3 +4 +5 +6 +7"
        "  +8 +9 +A +B +C +D +E +F    0123456789ABCDEF\n");
    while(1) {
        n = size > 0x10 ? 0x10 : size;
        size -= n;

        p += fprintf(stdout, "%08X: ", c);
        for(i = 0; i < n; i++) {
            if(i == 8) {
                p += fprintf(stdout, " ");
            }
            p += fprintf(stdout, "%02X ", *(s + c + i));
        }
        for(i = n; i < 0x10; i++) {
            if(i == 8) {
                p += fprintf(stdout, " ");
            }
            p += fprintf(stdout, "   ");
        }
        p += fprintf(stdout, "   ");
        for(i = 0; i < n; i++) {
            if(*(s + c + i) >= 0x20 && *(s + c + i) <= 0x7E) {
                p += fprintf(stdout, "%c", *(s + c + i));
            } else {
                p += fprintf(stdout, ".");
            }
        }
        p += fprintf(stdout, "\n");
        if(size <= 0) {
            break;
        }
        c += n;
    }
}

// if c is not a valid char, return 0.
#define HexChar2Int(c) \
    ((c >= '0' && c <= '9') ? (c - '0') : ((c >= 'A' && c <= 'F') ? \
    c - 'A' + 0xA : ((c >= 'a' && c <= 'f') ? c - 'a' + 0xA : 0)))

#define OctChar2Int(c) \
     ((c >= '0' && c <= '7') ? (c - '0') : 0)

#define DecChar2Int(c) \
     ((c >= '0' && c <= '9') ? (c - '0') : 0)


// convert string to unsigned int.
// hex: 0x, oct: 0, bin: ...b, dec: normal.
unsigned int atou(char* s)
{
    char *p = s, *e = s;
    unsigned int r = 0;

    while(*e != '\0')
        e++;

    if(*p == '0' && (*(p + 1) == 'x' || *(p + 1) == 'X')) {
        // hex: skip first 0x or 0X header.
        p += 2;

        while(p != e) {
            r <<= 4;
            r += HexChar2Int(*p);
            p++;
        }
    } else if((e - p) >= 2 && *(e - 1) == 'b') {
        // bin: ignore last b.
        e--;

        while(p != e) {
            r <<= 1;
            r += (*p == '0' ? 0 : 1);
            p++;
        }
    } else if(*p == '0') {
        // oct: skip first 0 header.
        p += 1;

        while(p != e) {
            r <<= 3;
            r += OctChar2Int(*p);
            p++;
        }
    } else {
        // dec: we do not need to deal the number.
        while(p != e) {
            r *= 10;
            r += DecChar2Int(*p);
            p++;
        }
    }

    return r;
}

// get one page, system memory map by page number.
unsigned char *mmap_page(unsigned int page)
{
    int fd = -1;
    unsigned char *mem = NULL;

    fd = open("/dev/mem", O_RDWR);
    if(fd < 0)
        return NULL;

    mem = (unsigned char *)mmap(0, PAGE_SIZE,
        PROT_READ | PROT_WRITE, MAP_SHARED, fd, page * PAGE_SIZE);
    close(fd);

    return mem;
}

int main(int argc, char *argv[])
{
    unsigned char *mem = NULL;
    unsigned int pos = 0, page = 0;
    unsigned int offset = 0, size = 0;

    if(argc != 3) {
        printf("vocore command, used to print 512 byte in core memory.\n");
        printf("usage: memv [offset] [size]\n");
        return 0;
    }

    // offset = 0x10000000 + atou(argv[1]);
    offset = atou(argv[1]);
    size = atou(argv[2]);

    printf("offset: 0x%08X, size: 0x%08X\n", offset, size);
    page = offset / PAGE_SIZE;
    pos = offset % PAGE_SIZE;

    if(size + pos > PAGE_SIZE) {
        size = PAGE_SIZE - pos;
        printf("warning: size exceed one page, resize to %d.\n", size);
    }

    mem = mmap_page(page);
    if(mem == NULL) {
        printf("can not map memory.\n");
        return -1;
    }

    // print request memory area.
    hexprint(mem + pos, size);

    munmap(mem, PAGE_SIZE);
    return 0;
}

