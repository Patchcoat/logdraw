#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>// command line options
#include <string.h>
#include <time.h>// datapoint time value
#include <stdint.h>// uint32_t for defining color
#include <regex.h>// keep in mind this is POSIX, and linux only
#include "mergesort.h"

// TODO
// this should be settable by the user, but for now it's just a variable
#define MAXLINES 1024
#define MAX_ERROR_MSG 0x1000
// these are each the regex that can be used to identify certain values
#define INT_REGEX "\\(-\\{0,1\\}[0-9]\\{1,2\\}\\)"
#define FLOAT_REGEX "\\(-\\{0,1\\}[0-9]\\{1,\\}\\.\\{0,1\\}[0-9]*\\)"
#define POSITIVE_INT_REGEX "\\([0-9]\\{1,2\\}\\)"

// vectors
typedef struct {
    float x;
    float y;
} v2d;
typedef struct {
    float x;
    float y;
    float z;
} v3d;
typedef struct {
    float x;
    float y;
    float z;
    float w;
} v4d;

// data union
union dt {
    long int i; // integer
    float f; // float
    char str[32]; // string// TODO handle arbitrary length strings
    v2d vec2d; // 2d vector
    v3d vec3d; // 3d vector
    v4d vec4d; // 4d vector
    uint32_t clr; // rbga color
};

// time format struct
enum timefmt {
    Epoch,// number of ms from the epoch
    Date,// year month day
    Time,// hour minute second millisecond
    DateTime// year month day hour minute second millisecond
};
// data format struct
enum datafmt{
    Int,
    Float,
    String,
    Vec2d,
    Vec3d,
    Vec4d,
    ColorRGB, // 0 to 255
    ColorHex // 0 to FF
};


// time datapoint
// if date is true, it's seconds from Jan 1, 1970. If it's false it's seconds
// from the start of the day
typedef struct {
    time_t time;
    unsigned short ml;// milliseconds
    int date;// functions as a bool. Is the date information in time?
    // TODO there's got to be a smaller way to store this data
} tmpt;
// datapoint struct
typedef struct {
    unsigned int id;
    tmpt time;
    union dt data;
} pt;

// dynamic array
typedef struct {
    pt* array;
    size_t used;
    size_t size;
} ptarray;

// datagroup struct
typedef struct {
    char* name; // name of the datagroup
    unsigned int id;
    regex_t idext; // identifier extractor
    enum timefmt tmfmt;// time format
    char* timestr; // time extractor string
    unsigned short tmordr[7];
    regex_t timeext; // time extractor
    enum datafmt dtfmt;// data format
    char* dtstr; // data extractor string
    unsigned short dtordr[7];
    regex_t dtext; // data extractor
    ptarray dt; // all of the data points
} grp;
// dtordr and tmordr are in a class of varible called datapoint order
// this is used to map the order in the regex to the expected order.
// For example the data in the regex may be ordered Z,X,Y, but the expected
// order for the data is X,Y,Z. So 0 (X) would be 1, 1 (Y) would be 2, and 2 (Z)
// would be 0. There are enough here to hold DateTime, the element with the
// greatest number of elements.

// init the datapoint array
void initdparray(ptarray* a, size_t initsize) {
    a->array = (pt*)malloc(initsize * sizeof(pt));
    a->used = 0;
    a->size = initsize;
}
// insert an element into the datapoint array
void indparray(ptarray* a, pt element) {
    if (a->used == a->size) {
        a->size *= 2;
        a->array = (pt*)realloc(a->array, a->size * sizeof(pt));
    }
    a->array[a->used++] = element;
}
// free the datapoint array
void freedparray(ptarray* a) {
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}

// *****************************************************************************
// The Extractor
//
// Each extractor grabs a section of the line and moves the data into a
// datapoint structure.
//
// As an example let's take this.
// [08:24:13] Vec3(1,5,7)
//
// Time
// the time format would be set to time, since there's no date information
// this exposes the $H, $M, and $S tags, each of which are equivalent to \d{1,2}
// The finished extractor looks like
// \[$H:$M:$S\]
//
// Identifier
// There are no special tags for an identifier. It just uses regex to see if a
// line contains a match. The extractor would look like
// Vec3
//
// Data
// The data format is used to extract the data. In this case we're using a 3D
// vector, so we'll use the Vec3d type. This exposes $X, $Y, and $Z, which are
// equivalent to \d+\.?\d+
// The finished extractor looks like
// \($X, $Y, $Z\)
// *****************************************************************************

// load the log file
unsigned long int ldlog(char* filename, char** lines) {
    for (int i = 12+strlen(filename); i > 0; i--) 
        putchar('-');
    putchar('\n');
    printf("|Load File %s|\n", filename);
    for (int i = 12+strlen(filename); i > 0; i--) 
        putchar('-');
    putchar('\n');
   
    FILE *file;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    file = fopen(filename, "r");
    if (file == NULL)
        exit(EXIT_FAILURE);

    unsigned long int lncount = 0;
    while ((read = getline(&line, &len, file)) != -1) {
        char* rdline = (char *)malloc(sizeof(char)*strlen(line)+1);
        strcpy(rdline, line);
        lines[lncount] = rdline;
        lncount++;
        // if going over the max number of lines, break out of the loop
        // because the array is indexed at 0 add 1 to lncount
        if (lncount+1 > MAXLINES)
            break;
    }

    fclose(file);
    if (line)
        free(line);

    return lncount;
}

void regexcompile(regex_t* r, char* regex) {
    int reti = regcomp(r, regex, 0);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        exit(1);
    }
}

int regexexec(regex_t* r, char* text) {
    char msgbuf[100];
    int reti = regexec(r, text, 0, NULL, 0);
    if (!reti) {
        return 1;
    } else if (reti == REG_NOMATCH) {
        return 0;
    } else {
        regerror(reti, r, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        exit(1);
    }
    return 0;
}

// text is the input text, ftext is the text to find, rtext is the text to replace
// constflag should be 1 if text is a string literal, 1 when it's not
char* replace(char* text, char* ftext, char* rtext, int constflag) {
    char* ch;
    if(NULL == (ch = strstr(text,ftext)))
        return text;

    // create a buffer big enough to hold the text plus the text that's going into the the
    // string, minus the text being replaced
    char buffer[strlen(text)+strlen(rtext)-strlen(ftext)];
    strncpy(buffer, text, ch-text);//copy the text to the buffer up to the first occurance
    buffer[ch-text] = 0;
    sprintf(buffer+(ch-text), "%s%s", rtext, ch+strlen(ftext));

    if (text && !constflag)
        free(text);
    size_t len = strlen(buffer)+1;
    char* otext = (char*) malloc(len);
    strcpy(otext, buffer);

    return otext;
}

// takes in a string, a matchptr, and return the number at that location
int mtchstoi(char* str, regmatch_t matchptr) {
    int n = 0;
    int neg = 0;
    if (str[matchptr.rm_so] == '-')
        neg = 1;
    for (int i = matchptr.rm_so+neg; i < matchptr.rm_eo; i++) {
        n *= 10;
        n += str[i] - '0';
    }
    if (neg)
        n *= -1;
    return n;
}
// as above but with a float
float mtchstof(char* str, regmatch_t matchptr) {
    float n = 0;
    int neg = 0;
    int dot = 0;
    if (str[matchptr.rm_so] == '-')
        neg = 1;
    // integer component
    for (int i = matchptr.rm_so+neg; i < matchptr.rm_eo; i++) {
        if (str[i] == '.') {
            dot = i+1;
            // add 1 so that if the dot is at point 0, "if (dot)" is still true
            // this has the added benefit that when the decimal for loop starts
            // it starts after the dot.
            break;
        }
        n *= 10;
        n += str[i] - '0';
    }
    // decimals
    if (dot) {
        float mult = 0.1;
        for (int i = dot; i < matchptr.rm_eo; i++) {
            n += (str[i] - '0') * mult;
            mult *= 0.1;
        }
    }
    if (neg)
        n *= -1;
    return n;
}
// as above but with a hex value
// this is laser focused on the clr value in the data value of a datapoint
char toup(char c) {
    if (c >= 'a')
        return c-0x20;
    else
        return c;
}
int isdgt(char c) {
    if (c >= '0' && c <= '9')
        return 1;
    else
        return 0;
}
uint32_t mtchstox(char* str, regmatch_t matchptr) {
    uint32_t n = 0;
    for (int i = matchptr.rm_so; i < matchptr.rm_eo; i++) {
        n *= 0x10;
        if (isdgt(str[i]))
            n += str[i] - '0';
        else
            n += toup(str[i]) - 'A' + 0xA;
    }
    return n;
}

// create the regex for time
void timeformat(grp* group, char* text) {
    char* regtxt;
    char* regorder[7];
    // the behavior depends on the format of time
    switch(group->tmfmt){
      case Epoch:;
        regorder[0] = strstr(text, "$E"); // epoch time
        regorder[1] = strstr(text, "$m"); // milliseconds
        for (int i = 0; i < 2; i++)
            group->tmordr[i] = i;
        mrgsrt(regorder, group->tmordr, 2);
        regtxt = replace(text, "$E", POSITIVE_INT_REGEX, 1);
        regtxt = replace(text, "$m", POSITIVE_INT_REGEX, 0);
        break;
      case Date:
        regorder[0] = strstr(text, "$Y");
        regorder[1] = strstr(text, "$o"); //month
        regorder[2] = strstr(text, "$D");
        for (int i = 0; i < 3; i++)
            group->tmordr[i] = i;
        mrgsrt(regorder, group->tmordr, 3);
        regtxt = replace(text, "$Y", POSITIVE_INT_REGEX, 1);
        regtxt = replace(regtxt, "$o", POSITIVE_INT_REGEX, 0);
        regtxt = replace(regtxt, "$D", POSITIVE_INT_REGEX, 0);
        break;
      case Time:;//empty statement to avoid compile errors
        // extract the order of elements from the text
        regorder[0] = strstr(text, "$H");
        regorder[1] = strstr(text, "$M"); // minutes
        regorder[2] = strstr(text, "$S");
        regorder[3] = strstr(text, "$m"); // milliseconds
        // fill the tmordr array
        for (int i = 0; i < 4; i++)
            group->tmordr[i] = i;
        // sort the tmordr array using the regorder array
        mrgsrt(regorder, group->tmordr, 4);
        // create the regex from the user input
        regtxt = replace(text, "$H", POSITIVE_INT_REGEX, 1);
        regtxt = replace(regtxt, "$M", POSITIVE_INT_REGEX, 0);
        regtxt = replace(regtxt, "$S", POSITIVE_INT_REGEX, 0);
        regtxt = replace(regtxt, "$m", POSITIVE_INT_REGEX, 0);
        break;
      case DateTime:
        regorder[0] = strstr(text, "$Y");
        regorder[1] = strstr(text, "$o"); //month
        regorder[2] = strstr(text, "$D");
        regorder[3] = strstr(text, "$H");
        regorder[4] = strstr(text, "$M"); // minutes
        regorder[5] = strstr(text, "$S");
        regorder[6] = strstr(text, "$m"); // milliseconds
        for (int i = 0; i < 7; i++)
            group->tmordr[i] = i;
        mrgsrt(regorder, group->tmordr, 7);
        regtxt = replace(text, "$Y", POSITIVE_INT_REGEX, 1);
        regtxt = replace(regtxt, "$o", POSITIVE_INT_REGEX, 0);
        regtxt = replace(regtxt, "$D", POSITIVE_INT_REGEX, 0);
        regtxt = replace(regtxt, "$H", POSITIVE_INT_REGEX, 0);
        regtxt = replace(regtxt, "$M", POSITIVE_INT_REGEX, 0);
        regtxt = replace(regtxt, "$S", POSITIVE_INT_REGEX, 0);
        regtxt = replace(regtxt, "$m", POSITIVE_INT_REGEX, 0);
        break;
      default:
        break;
    }
    regexcompile(&group->timeext, regtxt);
    free(regtxt);
}
// create the regex for data
void dataformat(grp* group, char* text) {
    char* regtxt;
    switch(group->dtfmt) {
      case Int:
        break;
      case Float:
        break;
      case String:
        break;
      case Vec2d:
        break;
      case Vec3d:;//empty statement to avoid compile errors
        // extract the order of elements from the text
        char* regorder[3] = {
            strstr(text, "$X"),
            strstr(text, "$Y"),
            strstr(text, "$Z")};
        // fill the tmordr array
        for (int i = 0; i < 3; i++)
            group->dtordr[i] = i;
        // sort the tmordr array using the regorder array
        mrgsrt(regorder, group->dtordr, 3);
        // create the regex from the user input
        regtxt = replace(text, "$X", FLOAT_REGEX, 1);
        regtxt = replace(regtxt, "$Y", FLOAT_REGEX, 0);
        regtxt = replace(regtxt, "$Z", FLOAT_REGEX, 0);
        break;
      case Vec4d:
        break;
      case ColorRGB:
        break;
      case ColorHex:
        break;
      default:
        break;
    }
    regexcompile(&group->dtext, regtxt);
    free(regtxt);
}

// load time information
void loadtm(char* line, regmatch_t* matchptr, grp* dtgrp, pt* point) {
    struct tm t;
    t.tm_sec = 0;
    t.tm_min = 0;
    t.tm_hour = 0;
    t.tm_mday = 1;
    t.tm_mon = 0;
    t.tm_year = 0;
    t.tm_isdst = -1;
    int mil = -1;
    switch (dtgrp->tmfmt) {
      case Epoch:
        point->time.date = 1;
        regmatch_t match = matchptr[dtgrp->tmordr[1]];
        if (match.rm_eo < 0)
            return;
        break;
      case Date:
        point->time.date = 1;
        point->time.ml = 0;
        for (int i = 0; i < 3; i++) {
            regmatch_t match = matchptr[dtgrp->tmordr[i]+1];
            if (match.rm_eo < 0)
                continue;
            switch (i) {
              case 0:
                t.tm_year = mtchstoi(line, match) - 1900;
                break;
              case 1:
                t.tm_mon = mtchstoi(line, match);
                break;
              case 2:
                t.tm_mday = mtchstoi(line, match);
                break;
            }
        }
        point->time.time = mktime(&t);
        break;
      case Time:
        for (int i = 0; i < 4; i++) {
            regmatch_t match = matchptr[dtgrp->tmordr[i]+1];
            if (match.rm_eo < 0)
                continue;
            switch (i) {
              case 0:
                t.tm_hour = mtchstoi(line, match);
                break;
              case 1:
                t.tm_min = mtchstoi(line, match);
                break;
              case 2:
                t.tm_sec = mtchstoi(line, match);
                break;
              case 3:
                mil = mtchstoi(line, match);
                break;
            }
        }
        point->time.ml = (mil == -1);
        point->time.date = 0;
        point->time.time = mktime(&t);
        break;
      case DateTime:
        point->time.date = 1;
        for (int i = 0; i < 7; i++) {
            regmatch_t match = matchptr[dtgrp->tmordr[i]+1];
            if (match.rm_eo < 0)
                continue;
            switch(i) {
              case 0:
                t.tm_year = mtchstoi(line, match) - 1900;
                break;
              case 1:
                t.tm_mon = mtchstoi(line, match);
                break;
              case 2:
                t.tm_mday = mtchstoi(line, match);
                break;
              case 3:
                t.tm_hour = mtchstoi(line, match);
                break;
              case 4:
                t.tm_min = mtchstoi(line, match);
                break;
              case 5:
                t.tm_sec = mtchstoi(line, match);
                break;
              case 6:
                mil = mtchstoi(line, match);
                break;
            }
        }
        point->time.ml = (mil == -1);
        point->time.date = 0;
        point->time.time = mktime(&t);
        break;
      default:
        break;
    }
}

// load data
void loaddt(char* line, regmatch_t* matchptr, grp* dtgrp, pt* point) {
    regmatch_t match = matchptr[dtgrp->dtordr[1]];
    switch (dtgrp->dtfmt) {
        case Int:
          if (match.rm_eo < 0)
              return;
          point->data.i = mtchstoi(line, match);
          break;
        case Float:
          if (match.rm_eo < 0)
              return;
          point->data.f = mtchstof(line, match);
          break;
        case String:;
          if (match.rm_eo < 0)
              return;
          for (int i = match.rm_so; i < match.rm_eo; i++) { 
              if (i - match.rm_so > 32)// TODO handle arbitrary length strings
                  break;
              point->data.str[i] = line[i];
          }
          break;
        case Vec2d:
          for (int i = 0; i < 2; i++) {
              regmatch_t match = matchptr[dtgrp->dtordr[i]+1];
              if (match.rm_eo < 0)
                  continue;
              switch (i) {
                case 0:
                  point->data.vec2d.x = mtchstof(line, match);
                  break;
                case 1:
                  point->data.vec2d.y = mtchstof(line, match);
                  break;
              }
          }
          break;
        case Vec3d:
          for (int i = 0; i < 3; i++) {
              regmatch_t match = matchptr[dtgrp->dtordr[i]+1];
              if (match.rm_eo < 0)
                  continue;
              switch (i) {
                case 0:
                  point->data.vec3d.x = mtchstof(line, match);
                  break;
                case 1:
                  point->data.vec3d.y = mtchstof(line, match);
                  break;
                case 2:
                  point->data.vec3d.z = mtchstof(line, match);
                  break;
              }
          }
          break;
        case Vec4d:
          for (int i = 0; i < 4; i++) {
              regmatch_t match = matchptr[dtgrp->dtordr[i]+1];
              if (match.rm_eo < 0)
                  continue;
              switch (i) {
                case 0:
                  point->data.vec4d.x = mtchstof(line, match);
                  break;
                case 1:
                  point->data.vec4d.y = mtchstof(line, match);
                  break;
                case 2:
                  point->data.vec4d.z = mtchstof(line, match);
                  break;
                case 3:
                  point->data.vec4d.w = mtchstof(line, match);
              }
          }
          break;
        case ColorRGB:;
          unsigned char r, g, b, a = 0;
          for (int i = 0; i < 4; i ++) {
              regmatch_t match = matchptr[dtgrp->dtordr[i]+1];
              if (match.rm_eo < 0)
                  continue;
              switch (i) {
                case 0:
                  r = (char)mtchstoi(line, match);
                  break;
                case 1:
                  g = (char)mtchstoi(line, match);
                  break;
                case 2:
                  b = (char)mtchstoi(line, match);
                  break;
                case 3:
                  a = (char)mtchstoi(line, match);
                  break;
              }
          }
          // insert each color value into the correct place in the clr variable
          point->data.clr = (r * 0x1000000) + (g * 0x10000) + (b * 0x100) + a;
          break;
        case ColorHex:
          if (match.rm_eo < 0)
              return;
          point->data.clr = mtchstox(line, match);
          break;
        default:
          break;
    }
}

int main(int argc, char** argv) {
    int c;
    char* filename;
    int fnflag = 0;// filename flag

    // read command line options
    while (1) {
        static struct option long_options[] = {
            {"help",    no_argument,       0, 'h'},
            {"version", no_argument,       0, 'v'},
            {"filename",    required_argument, 0, 'f'},
            {0,0,0,0}
        };

        int option_index = 0;

        // get the options, the ':' after the f means an argument is required
        c = getopt_long (argc, argv, "hvf:",
                long_options, &option_index);

        if (c == -1)
            break;

        switch (c) {
          case 0: // error checking
            if (long_options[option_index].flag != 0)
                break;
            printf ("option %s", long_options[option_index].name);
            if (optarg)
                printf (" with arg %s", optarg);
            printf("\n");
            break;
          case 'h': // print help
            puts ("Usage: logdraw [options]");
            puts ("-h, --help                   Display this information");
            puts ("-v, --version                Display version information");
            puts ("-f, --filename <file name>   Load a log file");
            return 0;
          case 'v': // print version
            puts ("logdraw 0.0.0.1");
            puts ("\nWritten by c1user");
            return 0;
          case 'f': // get the specified filename
            fnflag = 1;// set the filename flag
            // copy optarg into the filename variable
            filename = malloc(strlen(optarg)+1);
            strcpy(filename, optarg);
            break;
          default:
            abort();
        }
    }

    if (optind < argc) {
        printf ("non-option ARGV-elements: ");
        while (optind < argc)
            printf ("%s ", argv[optind++]);
        putchar ('\n');
    }

    // Create datagroup arrays
    // TODO load datagroups from a file
    size_t grpcnt = 1;
    grp* dtgrps[grpcnt];

    // Create datagroup structures.

    grp vec3dg;
    dtgrps[0] = &vec3dg;
    vec3dg.tmfmt = Time;
    vec3dg.dtfmt = Vec3d;
    vec3dg.name = "Vec3";
    timeformat(&vec3dg, "\\[$H:$M:$S\\]");
    regexcompile(&vec3dg.idext, "Vec3");
    dataformat(&vec3dg, "($X,$Y,$Z)");
    initdparray(&vec3dg.dt, 32);// TODO figure out a good starting size
    // Here's an idea. init the arrays at the beginning of the data groups for loop
    // Assume that each data group takes up about the same space, so divide the
    // lncount by the grpcnt and get the size of the init datapoint array

    // End Create datagroup structures.

    if (fnflag == 0) {
        printf("Please select a file using the -f tag\n");
        return 0;
    } else {
        // create an array of lines
        char** lines = (char**)malloc(MAXLINES*sizeof(char*));
        unsigned long int lncount = ldlog(filename, lines);
        printf("Line count: %u\n", (unsigned int) lncount);
        // loop through each line
        for (int i = 0; i < lncount; i++) {
            printf("%s",lines[i]);
            // for every line, loop through every group
            for (int j = 0; j < grpcnt; j++) {
                int reti = regexexec(&dtgrps[j]->idext, lines[i]);
                // check if the line belongs to the group
                if (reti) {
                    pt point;
                    regmatch_t* matchptr = malloc(sizeof(regmatch_t)*8);
                    reti = regexec(&dtgrps[j]->timeext, lines[i], 8, matchptr, 0);
                    // load the time from the line into the data structure
                    if (!reti) {
                        loadtm(lines[i], matchptr, dtgrps[j], &point);
                    } else {
                        printf("Time formatting error\n");
                    }
                    free(matchptr);
                    matchptr = malloc(sizeof(regmatch_t)*5);
                    reti = regexec(&dtgrps[j]->dtext, lines[i], 5, matchptr, 0);
                    // load the data from the line into the data structure
                    if (!reti) {
                        loaddt(lines[i], matchptr, dtgrps[j], &point);
                    } else {
                        printf("Data formatting error\n");
                    }
                    // insert the datapoint into the array
                    indparray(&dtgrps[j]->dt, point);
                    free(matchptr);
                }
            }
            free(lines[i]);
        }
        free(lines);
    }

    for (int i = 0; i < grpcnt; i++) {
        printf("%d\n",(int)dtgrps[i]->dt.used);
        regfree(&dtgrps[i]->timeext);
        regfree(&dtgrps[i]->idext);
        regfree(&dtgrps[i]->dtext);
        freedparray(&dtgrps[i]->dt);
        // regex free
    }

    if (filename)
        free(filename);

    return 0;
}
