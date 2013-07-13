#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <ftw.h>
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

int is_printable_utf8(const char *s) {
    int togo = 0;
    while(*s) {
        uchar c = (uchar) *s;
        if(togo == 0) {
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
                    return 0;
            }
            if( ! is_printable( c & 127 ) ) {
                return 0;
            }
        } else {
            if( count_highbits(c) != 1 ) {
                return 0;
            }
            togo--;
        }
        s++;
    }
    return togo==0;
}

int walker(const char *fn, const struct stat *st, int t, struct FTW *ftw) {
    if( ! is_printable_utf8(fn) ) {
        fprintf(stderr,"no printable utf-8-filename:");
        while(*fn) {
            uchar c = (uchar) *fn;
            if(is_printable(c)) {
                fprintf(stderr,"%c",c);
            } else {
                fprintf(stderr,"#x%x",c);
            }
            fn++;
        }
        fprintf(stderr,"\n");
    }
    return 0;
}

int main(int argc, char** argv) {
    if(argc != 2) {
        fprintf(stderr,"no argument\n");
        return -1;
    }
    char *arg = argv[1];
    nftw (arg, &walker, 64, 0);
    return 0;
};
