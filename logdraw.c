#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>// command line options
#include <string.h>
#include <time.h>// datapoint time value
#include <stdint.h>// uint32_t for defining color
#include <regex.h>// keep in mind this is POSIX, and linux only
#include "mergesort.h"
#include "datapoint.h"
//#include "gtkUI.h"
#include "draw.h"

// TODO
// this should be settable by the user, but for now it's just a variable
#define MAXLINES 1024
#define MAX_ERROR_MSG 0x1000
// these are each the regex that can be used to identify certain values
#define INT_REGEX "\\(-\\{0,1\\}[0-9]\\{1,2\\}\\)"
#define FLOAT_REGEX "\\(-\\{0,1\\}[0-9]\\{1,\\}\\.\\{0,1\\}[0-9]*\\)"
#define POSITIVE_INT_REGEX "\\([0-9]\\{1,2\\}\\)"
#define STRING_REGEX "\\([[:print:]]\\{1,\\}\\)"
#define HEX_REGEX "\\([A-Za-z0-9]\\{6,8\\}\\)"

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
char* replace(char* text, char* ftext, char* rtext) {
    char* ch;
    if(NULL == (ch = strstr(text,ftext)))
        return text;

    // create a buffer big enough to hold the text plus the text that's going into the the
    // string, minus the text being replaced
    char buffer[strlen(text)+strlen(rtext)-strlen(ftext)];
    strncpy(buffer, text, ch-text);//copy the text to the buffer up to the first occurance
    buffer[ch-text] = 0;
    sprintf(buffer+(ch-text), "%s%s", rtext, ch+strlen(ftext));

    // this is set up in a way that text always needs to be freed
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
    int count = 0;
    for (int i = matchptr.rm_so; i < matchptr.rm_eo; i++) {
        count++;
        n *= 0x10;
        if (isdgt(str[i]))
            n += str[i] - '0';
        else
            n += toup(str[i]) - 'A' + 0xA;
    }
    // if the hex is only 6 or 7 hex digits long, push the previous values to
    // the end and pad with 0xF. Basically if the string was lacking
    // transparency make it opaque.
    while (count < 8) {
        n *= 0x10;
        n += 0xF;
    }
    return n;
}

// create the regex for time
void timeformat(grp* group) {
    char text[strlen(group->timestr)+1];
    strcpy(text, group->timestr);
    if (text[0] == '\0')
        return;
    char* regtxt = (char*)malloc(strlen(text)+1);
    strcpy(regtxt, text);
    char* regorder[7];
    // the behavior depends on the format of time
    switch(group->tmfmt){
      case Epoch:
        regorder[0] = strstr(text, "$E"); // epoch time
        regorder[1] = strstr(text, "$m"); // milliseconds
        for (int i = 0; i < 2; i++)
            group->tmordr[i] = i;
        mrgsrt(regorder, group->tmordr, 2);
        regtxt = replace(regtxt, "$E", POSITIVE_INT_REGEX);
        regtxt = replace(regtxt, "$m", POSITIVE_INT_REGEX);
        break;
      case Date:
        regorder[0] = strstr(text, "$Y");
        regorder[1] = strstr(text, "$o"); //month
        regorder[2] = strstr(text, "$D");
        for (int i = 0; i < 3; i++)
            group->tmordr[i] = i;
        mrgsrt(regorder, group->tmordr, 3);
        regtxt = replace(regtxt, "$Y", POSITIVE_INT_REGEX);
        regtxt = replace(regtxt, "$o", POSITIVE_INT_REGEX);
        regtxt = replace(regtxt, "$D", POSITIVE_INT_REGEX);
        break;
      case Time:
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
        regtxt = replace(regtxt, "$H", POSITIVE_INT_REGEX);
        regtxt = replace(regtxt, "$M", POSITIVE_INT_REGEX);
        regtxt = replace(regtxt, "$S", POSITIVE_INT_REGEX);
        regtxt = replace(regtxt, "$m", POSITIVE_INT_REGEX);
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
        regtxt = replace(regtxt, "$Y", POSITIVE_INT_REGEX);
        regtxt = replace(regtxt, "$o", POSITIVE_INT_REGEX);
        regtxt = replace(regtxt, "$D", POSITIVE_INT_REGEX);
        regtxt = replace(regtxt, "$H", POSITIVE_INT_REGEX);
        regtxt = replace(regtxt, "$M", POSITIVE_INT_REGEX);
        regtxt = replace(regtxt, "$S", POSITIVE_INT_REGEX);
        regtxt = replace(regtxt, "$m", POSITIVE_INT_REGEX);
        break;
      default:
        break;
    }
    regexcompile(&group->timeext, regtxt);
    free(regtxt);
}
// create the regex for data
void dataformat(grp* group) {
    char text[strlen(group->dtstr)+1];
    strcpy(text, group->dtstr);
    if (text[0] == '\0')
        return;
    char* regtxt = (char*)malloc(strlen(text)+1);
    strcpy(regtxt, text);
    char* regorder[4];
    switch(group->dtfmt) {
      case Int:
        group->dtordr[0] = 0;
        regtxt = replace(regtxt, "$I", INT_REGEX);
        break;
      case Float:
        group->dtordr[0] = 0;
        regtxt = replace(regtxt, "$F", FLOAT_REGEX);
        break;
      case String:
        group->dtordr[0] = 0;
        regtxt = replace(regtxt, "$S", STRING_REGEX);
        break;
      case Vec2d:
        regorder[0] = strstr(text, "$X");
        regorder[1] = strstr(text, "$Y");
        for (int i = 0; i < 2; i++)
            group->dtordr[i] = i;
        mrgsrt(regorder, group->dtordr, 2);
        regtxt = replace(regtxt, "$X", FLOAT_REGEX);
        regtxt = replace(regtxt, "$Y", FLOAT_REGEX);
        break;
      case Vec3d:;//empty statement to avoid compile errors
        // extract the order of elements from the text
        regorder[0] = strstr(text, "$X");
        regorder[1] = strstr(text, "$Y");
        regorder[2] = strstr(text, "$Z");
        // fill the tmordr array
        for (int i = 0; i < 3; i++)
            group->dtordr[i] = i;
        // sort the tmordr array using the regorder array
        mrgsrt(regorder, group->dtordr, 3);
        // create the regex from the user input
        regtxt = replace(regtxt, "$X", FLOAT_REGEX);
        regtxt = replace(regtxt, "$Y", FLOAT_REGEX);
        regtxt = replace(regtxt, "$Z", FLOAT_REGEX);
        break;
      case Vec4d:
        regorder[0] = strstr(text, "$X");
        regorder[1] = strstr(text, "$Y");
        regorder[2] = strstr(text, "$Z");
        regorder[3] = strstr(text, "$W");
        for (int i = 0; i < 4; i++)
            group->dtordr[i] = i;
        mrgsrt(regorder, group->dtordr, 4);
        regtxt = replace(regtxt, "$X", FLOAT_REGEX);
        regtxt = replace(regtxt, "$Y", FLOAT_REGEX);
        regtxt = replace(regtxt, "$Z", FLOAT_REGEX);
        regtxt = replace(regtxt, "$W", FLOAT_REGEX);
        break;
      case ColorRGB:
        regorder[0] = strstr(text, "$R");
        regorder[1] = strstr(text, "$G");
        regorder[2] = strstr(text, "$B");
        regorder[3] = strstr(text, "$A");
        // fill the tmordr array
        for (int i = 0; i < 4; i++)
            group->dtordr[i] = i;
        // sort the tmordr array using the regorder array
        mrgsrt(regorder, group->dtordr, 4);
        // create the regex from the user input
        regtxt = replace(regtxt, "$R", POSITIVE_INT_REGEX);
        regtxt = replace(regtxt, "$G", POSITIVE_INT_REGEX);
        regtxt = replace(regtxt, "$B", POSITIVE_INT_REGEX);
        regtxt = replace(regtxt, "$A", POSITIVE_INT_REGEX);
        break;
      case ColorHex:
        regtxt = replace(regtxt, "$X", HEX_REGEX);
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
    regmatch_t match = matchptr[dtgrp->dtordr[0]];
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

// takes in a string and returns the associated time format
enum timefmt strtotmft(char* str) {
    if (strcmp(str, "Epoch")) {
        return Epoch;
    } else if (strcmp(str, "Date")) {
        return Date;
    } else if (strcmp(str, "Time")) {
        return Time;
    } else if (strcmp(str, "DateTime")) {
        return DateTime;
    }
    return Epoch;
}
// as above but for the data format
enum timefmt strtodtft(char* str) {
    if (strcmp(str, "Int")) {
        return Int;
    } else if (strcmp(str, "Float")) {
        return Float;
    } else if (strcmp(str, "String")) {
        return String;
    } else if (strcmp(str, "Vec2d")) {
        return Vec2d;
    } else if (strcmp(str, "Vec3d")) {
        return Vec3d;
    } else if (strcmp(str, "Vec4d")) {
        return Vec4d;
    } else if (strcmp(str, "ColorRGB")) {
        return ColorRGB;
    } else if (strcmp(str, "ColorHex")) {
        return ColorHex;
    }
    return Int;
}

// read the log file config file
grp* readconfig(char* filein, size_t *grpcnt) {
    char* filename = (char *) malloc(strlen(filein)+6);
    // the config file for a given filename is named "filename.lgd"
    strcpy(filename, ".");
    strcat(filename, filein);
    strcat(filename, ".lgd");
    // notify user that the file is being read
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
    
    grp* dtgrps = (grp*)malloc(sizeof(grp));

    size_t grpnum = -1;
    unsigned long int lncount = 0;
    while ((read = getline(&line, &len, file)) != -1) {
        size_t linelen = strlen(line);
        //printf("%s", rdline);
        char startchar = line[0];
        size_t colon = 0;
        // find the colon in the line
        for (int i = 0; i < linelen; i++) {
            if (line[i] == ':') {
                colon = i;
                break;
            }
        }
        // get rdline to be just the data portion of the line
        char* rdline = (char *)malloc(sizeof(char)*linelen+1);
        strcpy(rdline, &line[colon+1]);
        rdline[linelen-(colon+2)] = '\0';
        //printf("data%d=\"%s\"\n",(int)grpnum,rdline);
        // different behaviors based on the label
        // in the log file the name must be first, since it's within the 'n'
        // case that the grp is set up.
        switch(startchar){
          case 'n':
            grpnum++;
            dtgrps = (grp*)realloc(dtgrps, sizeof(grp)*(grpnum+1));
            dtgrps[grpnum].name = (char*)malloc(strlen(rdline)+1);
            strcpy(dtgrps[grpnum].name, rdline);
            initdparray(&dtgrps[grpnum].dt, 32);// TODO figure out a good starting size
            //printf("name\n");
            break;
          case 't':
            if (line[1] == 'm') {
                enum timefmt tmft = strtotmft(rdline);
                dtgrps[grpnum].tmfmt = tmft;
            } else {
                dtgrps[grpnum].timestr = (char*)malloc(strlen(rdline)+1);
                if (strlen(rdline)) {
                    strcpy(dtgrps[grpnum].timestr, rdline);
                } else {
                    strcpy(dtgrps[grpnum].timestr, "");
                }
                timeformat(&dtgrps[grpnum]);
            }
            //printf("time\n");
            break;
          case 'i':
            dtgrps[grpnum].idstr = (char*)malloc(strlen(rdline)+1);
            strcpy(dtgrps[grpnum].idstr, rdline);
            regexcompile(&dtgrps[grpnum].idext, dtgrps[grpnum].idstr);
            //printf("id\n");
            break;
          case 'd':
            if (line[1] == 't') {
                enum datafmt dtft = strtodtft(rdline);
                dtgrps[grpnum].dtfmt = dtft;
            } else {
                dtgrps[grpnum].dtstr = (char*)malloc(strlen(rdline)+1);
                strcpy(dtgrps[grpnum].dtstr, rdline);
                dataformat(&dtgrps[grpnum]);
            }
            //printf("data\n");
            break;
          default:
            break;
        }
        free(rdline);
        lncount++;
        // if going over the max number of lines, break out of the loop
        // because the array is indexed at 0 add 1 to lncount
        if (lncount+1 > MAXLINES)
            break;
    }

    fclose(file);
    if (line)
        free(line);

    free(filename);
    
    //for (int i = 0; i <= grpnum; i++) {
    //    if (dtgrps[i].timestr[0] != '\0')
    //        regfree(&dtgrps[i].timeext);
    //    regfree(&dtgrps[i].idext);
    //    regfree(&dtgrps[i].dtext);
    //    free(dtgrps[i].name);
    //    free(dtgrps[i].timestr);
    //    free(dtgrps[i].dtstr);
    //    free(dtgrps[i].idstr);
    //    //freedparray(&dtgrps[i]->dt);
    //}
    //free(dtgrps);

    *grpcnt = grpnum+1;
    return dtgrps;
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
    size_t grpcnt;
    grp* dtgrps = readconfig(filename, &grpcnt);
   
    //check grpcnt
    for (int i = 0; i < grpcnt; i++) {
        printf("Name: %s\n",dtgrps[i].name);
        printf("Time: %s\n", dtgrps[i].timestr);
        printf("Data: %s\n", dtgrps[i].dtstr);
    }

    // TODO load datagroups from a file
    //size_t grpcnt = 3;
    //grp dtgrps[grpcnt];

    /*
    // Create datagroup structures.
    grp vec3dg;
    vec3dg.tmfmt = Time;
    vec3dg.dtfmt = Vec3d;
    vec3dg.name = "Vec3";
    vec3dg.timestr = (char*)malloc(sizeof(char)*15);
    strcpy(vec3dg.timestr, "\\[$H:$M:$S\\]");
    timeformat(&vec3dg);
    vec3dg.idstr = (char*)malloc(sizeof(char)*5);
    strcpy(vec3dg.idstr, "Vec3");
    regexcompile(&vec3dg.idext, vec3dg.idstr);
    vec3dg.dtstr = (char*)malloc(sizeof(char)*11);
    strcpy(vec3dg.dtstr, "($X,$Y,$Z)");
    dataformat(&vec3dg);
    initdparray(&vec3dg.dt, 32);// TODO figure out a good starting size
    dtgrps[0] = vec3dg;
    // Here's an idea. init the arrays at the beginning of the data groups for loop
    // Assume that each data group takes up about the same space, so divide the
    // lncount by the grpcnt and get the size of the init datapoint array
    grp magic;
    magic.tmfmt = Epoch;
    magic.dtfmt = String;
    magic.name = "Magic";
    magic.timestr = (char*)malloc(sizeof(char)*1);
    strcpy(magic.timestr, "");
    timeformat(&magic);
    magic.idstr = (char*)malloc(sizeof(char)*7);
    strcpy(magic.idstr, "^Magic");
    regexcompile(&magic.idext, magic.idstr);
    magic.dtstr = (char*)malloc(sizeof(char)*9);
    strcpy(magic.dtstr, "Magic $S");
    dataformat(&magic);
    initdparray(&magic.dt, 32);
    dtgrps[1] = magic;

    grp power;
    power.tmfmt = Epoch;
    power.dtfmt = Float;
    power.name = "Power";
    power.timestr = (char*)malloc(sizeof(char)*4);
    strcpy(power.timestr, "^$E");
    timeformat(&power);
    power.idstr = (char*)malloc(sizeof(char)*8);
    strcpy(power.idstr, " Power ");
    regexcompile(&power.idext, power.idstr);
    power.dtstr = (char*)malloc(sizeof(char)*9);
    strcpy(power.dtstr, "Power $F");
    dataformat(&power);
    initdparray(&power.dt, 32);
    dtgrps[2] = power;
    */
    

    // End Create datagroup structures.

    if (fnflag == 0) {
        printf("Please select a file using the -f tag\n");
        return 0;
    }

    // create an array of lines
    char** lines = (char**)malloc(MAXLINES*sizeof(char*));
    unsigned long int lncount = ldlog(filename, lines);
    printf("Line count: %u\n", (unsigned int) lncount);
    // loop through each line
    for (int i = 0; i < lncount; i++) {
        //printf("%s",lines[i]);
        //logScrollInsert(css, loglist, lines[i]);
        // for every line, loop through every group
        for (int j = 0; j < grpcnt; j++) {
            int reti = regexexec(&dtgrps[j].idext, lines[i]);
            // check if the line belongs to the group
            if (reti) {
                pt point;
                regmatch_t* matchptr = malloc(sizeof(regmatch_t)*8);
                if (dtgrps[j].timestr[0] != '\0') {
                    reti = regexec(&dtgrps[j].timeext, lines[i], 8, matchptr, 0);
                    // load the time from the line into the data structure
                    if (!reti) {
                        loadtm(lines[i], matchptr, &dtgrps[j], &point);
                    } else {
                        printf("Time formatting error\n");
                    }
                }
                free(matchptr);
                matchptr = malloc(sizeof(regmatch_t)*5);
                reti = regexec(&dtgrps[j].dtext, lines[i], 5, matchptr, 0);
                // load the data from the line into the data structure
                if (!reti) {
                    loaddt(lines[i], matchptr, &dtgrps[j], &point);
                } else {
                    printf("Data formatting error\n");
                }
                // insert the datapoint into the array
                indparray(&dtgrps[j].dt, point);
                
                free(matchptr);
            }
        }
    }


    for (int i = 0; i < grpcnt; i++) {
        //printf("%s %d\n",dtgrps[i]->name, (int)dtgrps[i]->dt.used);
    }

    //GtkApplication* app = UISetup(lines, lncount, grpcnt, dtgrps);
    //int status = UIStart(app);

    for (int i = 0; i < lncount; i++)
        free(lines[i]);
    free(lines);

    for (int i = 0; i < grpcnt; i++) {
        if (dtgrps[i].timestr[0] != '\0')
            regfree(&dtgrps[i].timeext);
        regfree(&dtgrps[i].idext);
        regfree(&dtgrps[i].dtext);
        free(dtgrps[i].name);
        free(dtgrps[i].timestr);
        free(dtgrps[i].dtstr);
        free(dtgrps[i].idstr);
        freedparray(&dtgrps[i].dt);
    }
    free(dtgrps);

    if (filename)
        free(filename);

    //return status;
    return 0;
}
