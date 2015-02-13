/* 
Copyright (C) 2013-15 Daniel Schmitz <daniel2.schmitz@tu-dortmund.de>

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
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
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

void printtype(mode_t m, int align) {
    switch(m) {
        case S_IFSOCK:
            printf("socket");
            if (align)
                  printf("           ");
            break;
        case S_IFLNK:
            printf("symlink");
            if (align)
                   printf("          ");
            break;
        case S_IFREG:
            printf("file");
            if (align)
                printf("             ");
            break;
        case S_IFBLK:
            printf("block device");
            if (align)
                        printf("     ");
            break;
        case S_IFDIR:
            printf("directory");
            if (align)
                     printf("        ");
            break;
        case S_IFCHR:
            printf("character device");
            if (align)
                            printf(" ");
            break;
        case S_IFIFO:
            printf("fifo");
            if (align)
                printf("             ");
            break;
    }
}

int is_in_violation(const char *s) {
    const char *end = s + strlen(s);
    if( find_violation(s,0) != end)
        return 1;
    return 0;
}

void printescaped(const char *fn) {
    const char *end = fn + strlen(fn);
    if( find_violation(fn,0) != end) {
        const char *ep = fn;
        const char *last;
        while ( end != ep) {
            last = ep;
            ep = find_violation(last,1);
            fwrite(last,sizeof(char),ep-last,stdout);
            if ( ep == end) 
                break;
            printf("#(%d)",(uchar) *ep);
            ep++;

        }
    } else {
            printf("%s",fn);
    }
}

int get_name_from_user(mode_t m,const char *fn, const char* en,char *nn) {
    printtype(m,0);
    printf(" %s\n\n",en);
    if(m == S_IFDIR) {
        printf("0) ignore and skip subtree\n");
    } else {
        printf("0) ignore\n");
    }
    printf("1) (default) replace with escaped name\n");
    printf("2) input new filename\n");
    printf("3) list directory contents and return here\n");
    printf("X) terminate\n");

    if(0 == fgets(nn,256,stdin)) {
        return FTW_STOP;
    }
    printf("\n");

    char *b;
    char o;
    DIR *d;
    struct dirent e;
    struct dirent *r;
    char *fn2;

    switch(nn[0]) {
        case '0':
            return FTW_SKIP_SUBTREE;
        case '\n':
        case '1':
            strcpy(nn,bassname(en));
            return FTW_CONTINUE;
        case '2':
            printf("new filename: ");
            if(0 == fgets(nn,256,stdin)) {
                return FTW_STOP;
            }
            *(nn+strlen(nn)-1) = 0; /* remove newline */
            for (int i = 0; i < strlen(nn); i++) {
                if(nn[i] == '/') {
                    printf("filename may not contain '/'.\n\n");
                    return get_name_from_user(m,fn,en,nn);
                }
            }
            if( find_violation(nn,0) != 
                    nn + strlen(nn)) {
                    printf("please enter valid utf-8.\n\n");
                    return get_name_from_user(m,fn,en,nn);
            }
            return FTW_CONTINUE;
        case '3':
            b = (char *) bassname(fn);
            fn2 = (char *) malloc(b-fn);
            strncpy(fn2,fn,b-fn-1);
            fn2[b-fn-1] = 0;
            d = opendir(fn2);
            printf("content of %s:\n",fn2);
            while(1) {
                readdir_r(d,&e,&r);
                if (r != &e) break;
                if (!strcmp(e.d_name,".")) continue;
                if (!strcmp(e.d_name,"..")) continue;
                if (is_in_violation(e.d_name))
                    printf("* ");
                else
                    printf("  ");

                printescaped(e.d_name);
                printf("\n");
            } 
            closedir(d);
            printf("\n *escaped\n\n");
            free(fn2);
            return get_name_from_user(m,fn,en,nn);
        default:
            return FTW_STOP;
    }
}

int walker(const char *fn, const struct stat *st, int t, struct FTW *ftw) {
    char *escaped_name = get_escaped_name(fn);
    mode_t m = st->st_mode & S_IFMT;

    int retval = FTW_CONTINUE;

    if (escaped_name == 0)
        return FTW_CONTINUE;

    switch (mode) {
        case MODEREPORT:
            printf("%s\n",escaped_name);
            free(escaped_name);
            return FTW_CONTINUE;
        case MODEINTERACTIVE:
            if((retval = get_name_from_user(m,fn,escaped_name,new_name)) !=
                FTW_CONTINUE) {
                    free(escaped_name);
                    return retval;
            }
            break;
        case MODEAUTO:
            strcpy(new_name,bassname(escaped_name));
            printf("renaming %s\n",escaped_name);
            break;
    }

    strcpy(escaped_name,fn);
    strcpy((char *) bassname(escaped_name),new_name);

    if (0 != rename(fn,escaped_name)) {
        perror("rename failed");

        if (mode == MODEINTERACTIVE) {
            free(escaped_name);
            return walker(fn,st,t,ftw);
        }
    }

    nftw (escaped_name, &walker, 64, FTW_ACTIONRETVAL);

    free(escaped_name);
    printf("\n");
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
                    case 'r':
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
    fprintf(stderr,"Usage: %s [MODE] [DIRECTORY]\n",argv[0]);
    fprintf(stderr,"mode can be either \n");
    fprintf(stderr,"  -r  report mode (default): print all violating filenames\n");
    fprintf(stderr,"  -a  auto mode: repair filesystem by escaping all violating filenames\n");
    fprintf(stderr,"  -i  interactive mode: let the user enter replacement filenames\n\n");
    fprintf(stderr,"directory specifies the root for a recursive directory tree walk. \n");
    fprintf(stderr,"directory defaults to the current directory.\n");
    return EXIT_FAILURE;
};
