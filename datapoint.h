#ifndef DATAPOINT_H
#define DATAPOINT_H
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
#endif
