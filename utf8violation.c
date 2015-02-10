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

#define _GNU_SOURCE
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <ftw.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
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

const char *find_violation(const char *s, int escape) {
    int togo = 0; /* bytes left in multibyte character */
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
            } else if (escape && wc == '#') {
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

char *get_escaped_name(const char *fn) {
    const char *end = fn + strlen(fn);
    char *escaped_name = 0;
    if( find_violation(fn,0) != end) {
        escaped_name = (char *) malloc(sizeof(char) * (1 + strlen(fn) * strlen("#(000)")));
        char *escp = escaped_name;
        const char *ep = fn;
        const char *last;
        while ( end != ep) {
            last = ep;
            ep = find_violation(last,1);
            strncpy(escp,last,ep-last);
            escp += ep-last;
            *escp = 0;
            if ( ep == end) 
                break;
            sprintf(escp,"#(%d)",(uchar) *ep);
            escp += strlen(escp);
            ep++;

        }
    } 
    return escaped_name;
}

#define MODEAUTO 2
#define MODEINTERACTIVE 1
#define MODEREPORT 0

int mode = MODEREPORT;

char new_name[256];

const char *bassname(const char *path) {
    const char *p = path + strlen(path);
    while(p >= path && (*p) != '/')
        --p;
    return p+1;
}

int walker(const char *fn, const struct stat *st, int t, struct FTW *ftw) {
    char *escaped_name = get_escaped_name(fn);

    if (escaped_name == 0)
        return FTW_CONTINUE;


    int do_repair = 0;

    switch (mode) {
        case MODEREPORT:
            printf("%s\n",escaped_name);
            free(escaped_name);
            return FTW_CONTINUE;
        case MODEINTERACTIVE:
modeinteractive:
            printf("%s\n",escaped_name);
            printf("0) ignore\n");
            printf("1) replace with default\n");
            printf("2) input new filename\n");

            if(0 == fgets(new_name,256,stdin)) {
                free(escaped_name);
                return FTW_STOP;
            }
            switch(new_name[0]) {
                case '0':
                    free(escaped_name);
                    return FTW_SKIP_SUBTREE;
                case '1':
                    strcpy(new_name,bassname(escaped_name));
                    break;
                case '2':
                    printf("new filename: ");
                    if(0 == fgets(new_name,256,stdin)) {
                        free(escaped_name);
                        return FTW_STOP;
                    }
                    *(new_name+strlen(new_name)-1) = 0; /* remove newline */
                    for (int i = 0; i < strlen(new_name); i++) {
                        if(new_name[i] == '/') {
                            printf("filename may not contain '/'\n");
                            goto modeinteractive;
                        }
                    }
                    break;
                default:
                    free(escaped_name);
                    return FTW_STOP;
            }
            break;
        case MODEAUTO:
            strcpy(new_name,bassname(escaped_name));
            printf("renaming %s\n",escaped_name);
            do_repair = 1;
            break;
    }

    strcpy(escaped_name,fn);
    strcpy((char *) bassname(escaped_name),new_name);

    if(0 != rename(fn,escaped_name)) {
        perror("rename failed");
    } else {
        printf("new name is %s \n\n",escaped_name);
        nftw (escaped_name, &walker, 64, FTW_ACTIONRETVAL);
    }
    free(escaped_name);
    return FTW_SKIP_SUBTREE;
}

int main(int argc, char** argv) {
    char *arg;
    if(argc < 2)
        arg = ".";
    else {
        if (argc < 3)
            arg = argv[1];
        else {
            if (strlen(argv[1]) < 2)
                goto usage;
            else {
                switch(argv[1][1]) {
                    case 'i':
                        mode = MODEINTERACTIVE;
                        break;
                    case 'a':
                        mode = MODEAUTO;
                        break;
                    default:
                        goto usage;
                }
            }
            arg = argv[2];
        }   
    }
    return nftw (arg, &walker, 64, FTW_ACTIONRETVAL);

usage:
    fprintf(stderr,"USAGE: %s (-i|-a)? DIRNAME?",argv[0]);
    return EXIT_FAILURE;
};
