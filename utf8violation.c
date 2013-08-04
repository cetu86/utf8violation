#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <ftw.h>
#include <unistd.h>
#define uchar unsigned char

int count_highbits(const uchar c) {
    int bit = 8;
    while( ( (1 << (bit-1) ) & c ) && ( bit > 0 ) ) {
        bit--;
    }
    return 8-bit;
};

int is_printable(const uchar c) {
    return ( c > 31 && c < 127 );
};

const char *find_violation(const char *s) {
    int togo = 0;
    const char *mbstart = s;
    while(*s) {
        uchar c = (uchar) *s;
        if(togo == 0) {
            mbstart = s;
            switch (count_highbits(c)) {
                case 0:
                    break;
                case 2:
                    togo = 1;
                    break;
                case 3:
                    togo = 2;
                    break;
                case 4:
                    togo = 3;
                    break;
                default:
                    return mbstart;
            }
            if( ! is_printable( c & 127 ) ) {
                return mbstart;
            }
        } else {
            if( count_highbits(c) != 1 ) {
                return mbstart;
            }
            togo--;
        }
        s++;
    }
    if(togo == 0) {
        return s;
    }
    return mbstart;
}

int walker(const char *fn, const struct stat *st, int t, struct FTW *ftw) {
    const char *ep = find_violation(fn);
    const char *end = fn +strlen(fn);
    if( end != ep) {
        fprintf(stderr,"no printable utf-8-filename:");
        write(2,fn,ep-fn);
        fprintf(stderr,"#(%d)",(uchar) *ep);
        ep++;
        const char *last;
        do {
            last = ep;
            ep = find_violation(last);
            write(2,last,ep-last);
            if(ep == end) {
                break;
            }
            fprintf(stderr,"#(%d)",(uchar) *ep);
            ep++;
        } while(end != ep);

        fprintf(stderr,"\n");
    }
    return 0;
}

int main(int argc, char** argv) {
    char *arg;
    if(argc != 2) {
        arg = ".";
    } else {
        arg = argv[1];
    }
    nftw (arg, &walker, 64, 0);
    return 0;
};
