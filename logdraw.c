#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>// command line options
#include <string.h>
#include <time.h>// datapoint time value
#include <stdint.h>// uint32_t for defining color
#include <regex.h>// keep in mind this is POSIX, and linux only

// TODO
// this should be settable by the user, but for now it's just a variable
#define MAXLINES 1024
#define MAX_ERROR_MSG 0x1000

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
    char str[32]; // string
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
    pt* point; // all of the data points
} grp;
// dtordr and tmordr are in a class of varible called datapoint order
// this is used to map the order in the regex to the expected order.
// For example the data in the regex may be ordered Z,X,Y, but the expected
// order for the data is X,Y,Z. So 0 (X) would be 1, 1 (Y) would be 2, and 2 (Z)
// would be 0. There are enough here to hold DateTime, the element with the
// greatest number of elements.

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

// ****************************************************************************
// Merge Sort the format function arrays
// This sorts one array using another array
// ****************************************************************************
// sorts the working array into the output array
void mrg(char** work, unsigned short* work_,
         size_t start, size_t mid, size_t end,
         char** out, unsigned short* out_) {
    size_t i = start;
    size_t j = mid;

    for (size_t k = start; k < end; k++) {
        // if the left run head exists and is <= the existing right run head
        // NULL is weighted to be at the end of the list
        // If the right run is null or the left run is not null then the left
        // run must be greater than the right run or the right run is null. If
        // that is true then use the left run, otherwise use the right run.
        if (i < mid && (j >= end ||
                    ((work[j] == NULL || work[i] != NULL) &&
                     (work[i] <= work[j] || work[j] == NULL)))) {
            out_[k] = work_[i];
            out[k] = work[i++];
        } else {
            out_[k] = work_[j];
            out[k] = work[j++];
        }
    }
}

// sort out array using the working array
// start in inclusive, end is exclusive
void spltmrg(char** work, unsigned short* work_,
             size_t start, size_t end,
             char** out, unsigned short* out_) {
    if (end - start < 2) // if the size is 1 it's sorted
        return;
    // split it in half
    size_t mid = (end + start) / 2;
    // recursively sort the halves
    spltmrg(out, out_, start, mid, work, work_);
    spltmrg(out, out_, mid, end, work, work_);

    // merge the two runs
    mrg(work, work_, start, mid, end, out, out_);
}

// this takes in the two arrays, the array to sort by (in) and the array that
// is being sorted by, (in_). It then creates working arrays and copies over
// the input arrays into those. It then calls a function to sort the working
// arrays into the input arrays
void mrgsrt(char** in, unsigned short* in_, size_t n) {
    char* work[n];
    unsigned short work_[n];
    memcpy(work, in, sizeof(char*)*n);
    memcpy(work_, in_, sizeof(unsigned short)*n);
    spltmrg(work, work_, 0, n, in, in_);
}
// ****************************************************************************
// End Merge Sort
// ****************************************************************************

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
            // add 1 so that if the dot is at point 0 "if (dot)" is still true
            // this has the added benefit that when the decimal for loop starts
            // it starts after the dot.
            break;
        }
        n *= 10;
        n += str[i] - '0';
    }
    // decimals
    if (dot) {
        int mult = 0.1;
        for (int i = dot; i < matchptr.rm_eo; i++) {
            n += (str[i] - '0') * mult;
            mult *= 0.1;
        }
    }
    if (neg)
        n *= -1;
    return n;
}

// create the regex for time
void timeformat(grp* group, char* text) {
    char* regtxt;
    // the behavior depends on the format of time
    switch(group->tmfmt){
      case Epoch:
        break;
      case Date:
        break;
      case Time:;//empty statement to avoid compile errors
        // extract the order of elements from the text
        char* regorder[4] = {
            strstr(text, "$H"),
            strstr(text, "$M"),  // minutes
            strstr(text, "$S"),
            strstr(text, "$m")}; // milliseconds
        // fill the tmordr array
        for (int i = 0; i < 4; i++)
            group->tmordr[i] = i;
        // sort the tmordr array using the regorder array
        mrgsrt(regorder, group->tmordr, 4);
        // create the regex from the user input
        regtxt = replace(text, "$H", "\\([0-9]\\{1,2\\}\\)", 1);
        regtxt = replace(regtxt, "$M", "\\([0-9]\\{1,2\\}\\)", 0);
        regtxt = replace(regtxt, "$S", "\\([0-9]\\{1,2\\}\\)", 0);
        regtxt = replace(regtxt, "$m", "\\([0-9]\\{1,3\\}\\)", 0);
        break;
      case DateTime:
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
        regtxt = replace(text, "$X","\\([0-9]\\{1,\\}\\)", 1);
        regtxt = replace(regtxt, "$Y","\\([0-9]\\{1,\\}\\)", 0);
        regtxt = replace(regtxt, "$Z","\\([0-9]\\{1,\\}\\)", 0);
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
                        switch (dtgrps[j]->tmfmt) {
                          case Epoch:
                            break;
                          case Date:
                            break;
                          case Time:;
                            int hour, min, sec, mil = -1;
                            for (int k = 0; k < 4; k++) {
                                regmatch_t match = matchptr[dtgrps[j]->tmordr[k]+1];
                                if (match.rm_eo < 0)
                                    continue;
                                switch (k) {
                                  case 0:
                                    hour = mtchstoi(lines[i], match);
                                    break;
                                  case 1:
                                    min = mtchstoi(lines[i], match);
                                    break;
                                  case 2:
                                    sec = mtchstoi(lines[i], match);
                                    break;
                                  case 3:
                                    mil = mtchstoi(lines[i], match);
                                    break;
                                }
                            }
                            point.time.ml = (mil == -1);
                            point.time.date = (1==0);
                            point.time.time = sec + (min * 60) + (hour * 360);
                            break;
                          case DateTime:
                            break;
                          default:
                            break;
                        }
                    }
                    free(matchptr);
                    matchptr = malloc(sizeof(regmatch_t)*5);
                    reti = regexec(&dtgrps[j]->dtext, lines[i], 5, matchptr, 0);
                    // load the data from the line into the data structure
                    if (!reti) {
                        switch (dtgrps[j]->dtfmt) {
                            case Int:
                              break;
                            case Float:
                              break;
                            case String:
                              break;
                            case Vec2d:
                              break;
                            case Vec3d:
                              for (int k = 0; k < 3; k++) {
                                  regmatch_t match = matchptr[dtgrps[j]->dtordr[k]+1];
                                  if (match.rm_eo < 0)
                                      continue;
                                  switch (k) {
                                    case 0:
                                      point.data.vec3d.x = mtchstof(lines[i], match);
                                      break;
                                    case 1:
                                      point.data.vec3d.y = mtchstof(lines[i], match);
                                      break;
                                    case 2:
                                      point.data.vec3d.z = mtchstof(lines[i], match);
                                      break;
                                  }
                              }
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
                    }
                    dtgrps[j]->point = &point;
                    free(matchptr);
                }
            }
            free(lines[i]);
        }
        free(lines);
    }

    regfree(&vec3dg.timeext);
    regfree(&vec3dg.idext);
    regfree(&vec3dg.dtext);
    // regex free

    if (filename)
        free(filename);

    return 0;
}
