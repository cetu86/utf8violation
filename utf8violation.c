/* 
Copyright (C) 2013 Daniel Schmitz <daniel2.schmitz@tu-dortmund.de>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <ftw.h>
#include <unistd.h>
#define uchar unsigned char
#define ushort unsigned short

int count_highbits(const uchar c) {
    int bit = 8;
    while( ( (1 << (bit-1) ) & c ) && ( bit > 0 ) ) {
        bit--;
    }
    return 8-bit;
};

uchar get_lower_bits(uchar c, int hb) {
    c <<= hb;
    c >>= hb;
    return c;
};

int is_not_printable(const ushort c) {
    return ( c < 32  || (c >= 0x80 && c <= 0x9f ));
};

const char *find_violation(const char *s) {
    int togo = 0;
    const char *mbstart = s;
    ushort wc;
    while(*s) {
        uchar c = (uchar) *s;
        int hb = count_highbits(c);
        if(togo == 0) {
            mbstart = s;
            wc = get_lower_bits(c,hb);
            if(hb > 4 || hb == 1) {
                return mbstart;
            } else if(hb > 0) {
                togo = hb-1;
            }
        } else {
            if( hb != 1 ) {
                return mbstart;
            }
            wc <<= 6;
            wc += get_lower_bits(c,hb);
            togo--;
        }

        if(togo == 0) {
            if( is_not_printable( wc ) ) {
                return mbstart;
            }
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
        //fprintf(stderr,"no printable utf-8-filename:");
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
