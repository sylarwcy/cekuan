/*****************************************************************************
 * IPType.h
 *****************************************************************************
 *
 * Project:     HALCON/libhalcon
 * Description: Types for image processing and language interfaces
 *
 * (c) 1996-2023 by MVTec Software GmbH
 *                  www.mvtec.com
 *
 *****************************************************************************/

#ifndef IPTYPE_H
#define IPTYPE_H

#ifndef HC_NO_STRUCT_MEMBER_ALIGNMENT
/* sets the struct member alignment to avoid problems if
 * the settings of the user differ to those of the HALCON
 * version. (8 is the default alignment, generally) */
#  pragma pack(push, 8)
#endif

#if !defined(HC_LARGE_IMAGES)
typedef int32_t HIMGDIM;  /* Image dimension, e.g., width and height */
typedef int16_t HIMGCOOR; /* Image coordinates, e.g, row and column */
typedef int32_t HLINCOOR; /* Linearized image coordinates */
typedef int32_t HIMGCNT;  /* Number of pixels, e.g., area or buffer size */
typedef int32_t HITEMCNT; /* Number of elements (contours or runlengths) */
typedef float   HSUBCOOR; /* Sub-pixel precise coordinates */
typedef float   HSUBATTR; /* Sub-pixel precise attributes of contours */
#else
typedef int32_t HIMGDIM;
typedef int32_t HIMGCOOR;
typedef INT4_8  HLINCOOR;
typedef INT4_8  HIMGCNT;
typedef INT4_8  HITEMCNT;
typedef double  HSUBCOOR;
typedef double  HSUBATTR;
#endif


/****************************************************************************/
/*  Types for images                                                        */
/****************************************************************************/

typedef void* (*HImageAllocProc)(size_t size);
typedef void (*HImageFreeProc)(void* mem);

typedef struct
{
  float re; /* real part                                   */
  float im; /* imaginary part                              */
} HComplexPixel;

typedef struct
{
  int16_t* p;        /* pointer to gray vales                       */
  int8_t   num_bits; /* number of used bits                         */
} HInt2Pixel;

typedef struct
{
  uint16_t* p;        /* pointer to gray vales                       */
  int8_t    num_bits; /* number of used bits                         */
} HUInt2Pixel;

typedef struct
{
  float*  row;  /* row direction                               */
  float*  col;  /* column direction                            */
  int32_t kind; /* VF_ABSOLUTE or VF_RELATIVE                  */
} HVFPixel;

typedef union
{
  uint8_t*       b;  /* 0..255 (BYTE_IMAGE)                          */
  uint8_t*       z;  /* 0..255 mod 256 (CYCLIC_IMAGE)                */
  uint8_t*       d;  /* orientation 0..180 (DIR_IMAGE)               */
  int8_t*        i;  /* -127..126 (INT1_IMAGE)                       */
  int32_t*       l;  /* 4 byte integer (LONG_IMAGE)                  */
  int64_t*       i8; /* 8 byte integer (INT8_IMAGE)                  */
  float*         f;  /* 4 byte real (FLOAT_IMAGE)                    */
  HVFPixel       vf; /* vector field (VF_IMAGE)                      */
  HComplexPixel* c;  /* complex image (COMPLEX_IMAGE)                */
  HInt2Pixel     s;  /* 2 bytes with sign (INT2_IMAGE)               */
  HUInt2Pixel    u;  /* 2 bytes without sign (UINT2_IMAGE)           */
} HPixelImage;

typedef struct
{
  int            kind;      /* pixel type                                   */
  HPixelImage    pixel;     /* pixel data                                   */
  HIMGDIM        width;     /* image width                                  */
  HIMGDIM        height;    /* image height                                 */
  HImageFreeProc free_proc; /* function for deallocating image data         */
  bool           free;      /* free image data when deleting image          */
  /* time of creation of image                                              */
  uint16_t msec; /* milliseconds 0..999                          */
  uint8_t  sec;  /* seconds 0..59                                */
  uint8_t  min;  /* minutes 0.59                                 */
  uint8_t  hour; /* 0..23                                        */
  uint8_t  day;  /* 1..31                                        */
  uint16_t yday; /* 1..366                                       */
  uint8_t  mon;  /* 1..12                                        */
  uint16_t year; /* 19xx                                         */
} Himage;

/* Information file for HALCON image file format (.exp)                     */
typedef struct
{
  int32_t width;          /* width of image                               */
  int32_t kind;           /* Pixel type                                   */
  int32_t height;         /* height of image                              */
  char    info[1000 - 4]; /* for further use                              */
} Himage_exp;

/* Information header for HALCON image file format                          */
typedef struct
{
  int32_t version_number; /* version number of the ima file format       */
  int32_t num_images;     /* number of images in file                    */
  bool    is_msb_first;   /* is most significant bit first (byte order)  */
} Himage_hobj_file_header;

/* Information header for HALCON image file format  - for each image        */
typedef struct
{
  int32_t width;        /* width of image                               */
  int32_t height;       /* height of image                              */
  int32_t num_channels; /* channels in image                            */
} Himage_hobj_image_header;


typedef uint8_t* Hbits; /* Binary image                                 */


/****************************************************************************/
/*                       Regionen                                           */
/****************************************************************************/


/* Index range for coordinates of run length regions */
#define MAX_COORD (MAX_FORMAT - 1) /*  2^EXP - 1; EXP=XL? 30: 15 */
/*---------------------------------------------------------------------------*
 * Note: We limit MIN_COORD to -2^EXP + 1, instead of -2^EXP, in order to
 *       ensure that the maximal region dimension
 *       (MAX_COORD - MIN_COORD + 1) stays below/equal to INT32_MAX in XL:
 *
 *       DIM_MAX = [2^30 - 1] - [-2^30 + 1] + 1 = 2^31 - 1 = INT32_MAX
 *---------------------------------------------------------------------------*/
#define MIN_COORD (-MAX_COORD) /* -2^EXP + 1; EXP=XL? 30: 15 */

/* Data type for a single run of a region encoded with runlength encoding   */
typedef struct
{
  HIMGCOOR l;  /* line number (row) of run                     */
  HIMGCOOR cb; /* column index of beginning of run             */
  HIMGCOOR ce; /* column index of ending of run                */
} Hrun;

/* vertical run */
typedef struct
{
  HIMGCOOR c;  /* column index of run                          */
  HIMGCOOR lb; /* row index of beginning of run                */
  HIMGCOOR le; /* row index of ending of run                   */
} Hvrun;

#define SHAPE_RECTANGLE1 (uint8_t)1
#define SHAPE_RECTANGLE2 (uint8_t)2
#define SHAPE_CIRCLE (uint8_t)3
#define SHAPE_ELLIPSE (uint8_t)4

/* Bit-array to check if a feature of a region was calculated               */
typedef struct
{
  unsigned int shape : 1;
  unsigned int is_convex : 1;
  unsigned int is_filled : 1;
  unsigned int is_connected4 : 1;
  unsigned int is_connected8 : 1;
  unsigned int is_thin : 1;
  unsigned int circularity : 1;
  unsigned int compactness : 1;
  unsigned int contlength : 1;
  unsigned int convexity : 1;
  unsigned int phi : 1;
  unsigned int elliptic_axis : 1;       /* ra, rb                         */
  unsigned int elliptic_shape : 1;      /* ra_, rb_                       */
  unsigned int excentricity : 1;        /* anisometry,bulkiness,str_faktor*/
  unsigned int moments : 1;             /* m11, m20, m02, ia, ib          */
  unsigned int center_area : 1;         /* row, col, area                 */
  unsigned int smallest_rectangle1 : 1; /* row1,col1,row2,col2            */
  unsigned int smallest_rectangle2 : 1; /* row_rect, col_rect, phi_rect,  */
                                        /* length1, length2               */
  unsigned int smallest_circle : 1;     /* row_circle, col_circle, radius */
  unsigned int min_max_chord : 1;       /* longest/shortest run           */
  unsigned int min_max_chord_gap : 1;   /* longest/shortest run (holes)   */
  unsigned int rectangularity : 1;
} HFeatureFlags;


/* Shape feature caching inside the region structure                        */
typedef struct
{
  union
  {
    HFeatureFlags single;
    long          all; /* if possible use 64 bits!                     */
  } def;
  uint8_t  shape; /* SHAPE_*                                      */
  bool     is_convex;
  bool     is_filled;
  bool     is_connected4;
  bool     is_connected8;
  bool     is_thin; /* one pixel thin                               */
  double   circularity;
  double   compactness;
  double   contlength;
  double   convexity;
  double   phi;
  double   ra, rb;   /* elliptic_axis                                */
  double   ra_, rb_; /* elliptic_shape                               */
  double   anisometry, bulkiness, structure_factor;
  double   m11, m20, m02, ia, ib;
  double   row, col;
  HIMGCNT  area;
  HIMGCOOR row1, col1, row2, col2;
  double   row_rect, col_rect, phi_rect, length1, length2;
  double   row_circle, col_circle, radius;
  HIMGCOOR min_chord, max_chord;
  HIMGCOOR min_chord_gap, max_chord_gap;
  double   rectangularity;
} HRegFeature;


typedef struct
{
  bool        is_compl; /* region is complement                         */
  HITEMCNT    num;      /* number of runs                               */
  HITEMCNT    num_max;  /* maximum number of runs                       */
  HRegFeature feature;  /* already processed features                   */
  Hrun*       rl;       /* pointer on array of run lengths              */
} Hrlregion;


/* vertical runlength encoding */
typedef struct
{
  bool        is_compl; /* region is complement                         */
  HITEMCNT    num;      /* number of runs                               */
  HITEMCNT    num_max;  /* maximum number of runs                       */
  HRegFeature feature;  /* already processed features                   */
  Hvrun*      rc;       /* array of runs                                */
} Hvrlregion;

/* Data structure for fast single pixel access for Hrlregion regions        */
typedef struct
{
  HIMGCOOR  min, max; /* Min/Max row index                            */
  Hrun*     rl;       /* pointer to runs of the region                */
  HITEMCNT* first;    /* array of index values (row begin)            */
  HITEMCNT* last;     /* array of index values (row end)              */
  bool*     no_row;   /* is there this row available?                 */
} Hrlaccess;

/* Data structure for a multichannel image                                  */
typedef struct
{
  Himage*    channels;     /* Channels of the multichannel image           */
  Hrlregion* region;       /* Domain of the multichannel image             */
  int        num_channels; /* Number of channels of the multichannel image */
} HMultiChannelImage;


/****************************************
 *
 ****************************************/
/* simple contour with long coordinates and variable allocation length */
typedef struct
{
  HIMGCOOR* row;     /* row values                           */
  HIMGCOOR* col;     /* columns values                       */
  HITEMCNT  num;     /* length of contour (number of points) */
  HITEMCNT  max_num; /* allocated points of contour          */
} Hcontvar;

/* simple contour with float coordinates and variable allocation length */
typedef struct
{
  HSUBCOOR* row;     /* row values                           */
  HSUBCOOR* col;     /* columns values                       */
  HITEMCNT  num;     /* length of contour (number of points) */
  HITEMCNT  max_num; /* allocated points of contour          */
} Hfcontvar;

/* Contour as chain code                                                    */
typedef struct
{
  HIMGCOOR l, c; /* Start point (row,column)                */
  uint8_t* cc;   /* array of chain code                     */
  HITEMCNT num;  /* length of contour (number of points)    */
} Hchain_code;


/* line (start and end point)                                               */
typedef struct
{
  HSUBCOOR row1, col1; /* Start point                                  */
  HSUBCOOR row2, col2; /* End point                                    */
} Hline;

/* Array of lines                                                           */
typedef struct
{
  HITEMCNT num;
  Hline*   line;
} Hlines;


/****************************************************************************/
/*                       Distribution                                       */
/****************************************************************************/

#define HISTO_LEN 256
typedef HIMGCNT Hhisto_abs[HISTO_LEN];
typedef double  Hhisto_rel[HISTO_LEN];


#define MAX_DISTR (2 * 256 + 1)
#define NULL_DISTR 256


/****************************************************************************/
/*              Misc. image processing                                      */
/****************************************************************************/

/* Convolution (Convol and FFT)                                             */
typedef struct
{
  int32_t  f;
  int32_t* gk;
  int32_t  num_max;
} Hconv_mask_s;

/* 2 dimensional convolution masks                                          */
typedef struct
{
  HIMGCOOR min_row, max_row;
  HIMGCOOR min_col, max_col;
  int32_t  norm;
  int32_t  num;
  HIMGCOOR row[MAX_CONVOL];
  HIMGCOOR col[MAX_CONVOL];
  HLINCOOR koor[MAX_CONVOL];
  int32_t  m[MAX_CONVOL];
} Hconv_mask_2;

/* 1 dimensional convolution masks                                          */
typedef struct
{
  HIMGCOOR min_col, max_col;
  int32_t  norm;
  int32_t  num;
  HIMGCOOR col[MAX_CONVOL];
  int32_t  m[MAX_CONVOL];
} Hconv_mask_1;


/* Color (name plus RGB values)                                             */
typedef struct
{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  char    name[MAX_COLOR_NAME_LENGTH];
} Hcolor;


/****************************************************************************/
/*                  Extended Line Description (XLD)                         */
/****************************************************************************/

/****************************************************************************/
/* XLD-Object Contour                                                       */
/****************************************************************************/

typedef enum cont_class
{
  cont_unknown,    /* unknown                                              */
  cont_no_junc,    /* neither start nor end point points are junctions     */
  cont_start_junc, /* start point is a junction                            */
  cont_end_junc,   /* end point is a junction                              */
  cont_both_junc,  /* both start and end point are junctions               */
  cont_closed      /* closed contour                                       */
} Hcont_class;

typedef struct cont_attrib
{
  char*     name; /* name of the attribute                                */
  HSUBATTR* val;  /* value of the attribute (per point)                   */
} Hcont_attrib;

typedef struct cont_global_attrib
{
  char*    name; /* name of the global attribute                         */
  HSUBATTR val;  /* value of the attribute (per contour)                 */
} Hcont_global_attrib;

typedef struct cont_type
{
  HITEMCNT             num;        /* number of points along the contour     */
  HSUBCOOR*            row;        /* points / row indices (y coordinates)   */
  HSUBCOOR*            col;        /* points / column indices (x coord.)     */
  Hcont_class          cont_class; /* contour class                          */
  int32_t              num_attrib; /* number of additional attributes        */
  Hcont_attrib*        attribs;    /* additional attributes (for each point) */
  int32_t              num_global; /* number of additional global attributes */
  Hcont_global_attrib* global;     /* additional attributes (per contour)    */
  int32_t              h;          /* auxiliary (temporary)                  */
} Hcont;


/****************************************************************************/
/* XLD-Objects: polygons                                                    */
/****************************************************************************/

typedef struct lin_seg_type
{
  HSUBCOOR row, col; /* control points of the polygon                       */
  HSUBATTR length;   /* length of the line from the cur. to the next point  */
  HSUBATTR phi;      /* orientation (rad) of this line                      */
  Hkey     ref;      /* data base key of the underlying contour             */
  HITEMCNT first;    /* index of the first point of the underlying contour  */
  HITEMCNT last;     /* index of the last contour point                     */
} Hline_seg;

typedef struct poly_type
{
  HITEMCNT   num_line; /* number of lines                                   */
  HITEMCNT   len_line; /* maximum number of lines (size of the array lines) */
  Hline_seg* lines;    /* control points of the polygon                     */
} Hpoly;


/****************************************************************************/
/* XLD-Objects: parallels                                                   */
/****************************************************************************/

typedef struct para_poly_type
{
  Hkey     poly1;   /* Pointer to 1st polygon of the relation (P1)         */
  Hkey     poly2;   /* Pointer to 2nd polygon of the relation (P2)         */
  HITEMCNT first1;  /* Index of the start line segm. of the parallel in P1 */
  HITEMCNT last1;   /* Index of the end line segment of the parallel in P1 */
  HITEMCNT first2;  /* Index of the start line segm. of the parallel in P2 */
  HITEMCNT last2;   /* Index of the end line segment of the parallel in P2 */
  double   quality; /* Measure of the degree of parallelism of the polygons*/
} Hpara_poly;


/****************************************************************************/
/* XLD-Objects parallel object borders                                      */
/****************************************************************************/

typedef struct mod_para_type
{
  Hkey     poly1;  /* Pointer to 1st polygon of the relation (P1)         */
  Hkey     poly2;  /* Pointer to 2nd polygon of the relation (P2)         */
  HITEMCNT first1; /* Index of the start line segm. of the parallel in P1 */
  HITEMCNT last1;  /* Index of the end line segment of the parallel in P1 */
  HITEMCNT first2; /* Index of the start line segm. of the parallel in P2 */
  HITEMCNT last2;  /* Index of the end line segment of the parallel in P2 */
  HSUBATTR first1_dist; /* Half distance of the first line of P1 to P2      */
  HSUBATTR last1_dist;  /* Half distance of the last line of P1 to P2       */
  HSUBATTR first2_dist; /* Half distance of the first line of P2 to P1      */
  HSUBATTR last2_dist;  /* Half distance of the last line of P2 to P1       */
  int16_t  dir1; /* Position of P1 relative to P2:                      */
                 /*   dir1 = 1: P2 is to the right of P1                */
                 /*   dir1 = -1: P2 is to the left of P1                */
  int16_t dir2;  /* Position of P2 relative to P1:                      */
                 /*   dir2 = 1: P1 is to the right of P2                */
                 /*   dir2 = -1: P1 is to the left of P2                */
} Hmod_para;

/****************************************************************************/
/* XLD-Objects Extended object borders                                      */
/****************************************************************************/

typedef struct ext_para_type
{
  Hkey     poly1;  /* Pointer to 1st polygon of the relation (P1)         */
  Hkey     poly2;  /* Pointer to 2nd polygon of the relation (P2)         */
  HITEMCNT first1; /* Index of the start line segm. of the parallel in P1 */
  HITEMCNT last1;  /* Index of the end line segment of the parallel in P1 */
  HITEMCNT first2; /* Index of the start line segm. of the parallel in P2 */
  HITEMCNT last2;  /* Index of the end line segment of the parallel in P2 */
} Hext_para;


/****************************************************************************/
/* Homogeneous coordinates: Hhom_mat_[23]d / Hproj_mat_[23]d                */
/****************************************************************************/
typedef double Hhom_mat_2d[2][3];
typedef double (
    *H_RESTRICT pHhom_mat_2d)[3]; /* pointer type variant of Hhom_mat_2d   */
typedef double const (*H_RESTRICT pHhom_mat_2d_const)[3];
typedef double (
    *puHhom_mat_2d)[3]; /* unrestricted pointer type variant of Hhom_mat_2d */
typedef double Hhom_mat_3d[3][4];
typedef double (
    *H_RESTRICT pHhom_mat_3d)[4]; /* pointer type variant of Hhom_mat_3d   */
typedef double (
    *puHhom_mat_3d)[4]; /* unrestricted pointer type variant of Hhom_mat_3d */
typedef double Hproj_mat_2d[3][3];
typedef double (
    *H_RESTRICT pHproj_mat_2d)[3]; /* pointer type variant of Hproj_mat_2d */
typedef double const (*H_RESTRICT pHproj_mat_2d_const)[3];
typedef double (*puHproj_mat_2d)
    [3]; /* unrestricted pointer type variant of Hproj_mat_2d */
typedef double Hproj_mat_3d[4][4];
typedef double (
    *H_RESTRICT pHproj_mat_3d)[4]; /* pointer type variant of Hproj_mat_3d */
typedef double (*puHproj_mat_3d)
    [4]; /* unrestricted pointer type variant of Hproj_mat_3d */


/****************************************************************************/
/* 1D-Functions                                                             */
/****************************************************************************/

typedef struct
{
  int32_t num;         /* Number of elements in x and y                      */
  float*  x;           /* Array of the x-values of the function (always set) */
  float*  y;           /* Array of the y-values of the function              */
  float   xmin;        /* Minimum x value (always set)                       */
  float   xmax;        /* Maximum x value (always set)                       */
  float   xdist;       /* Distance of x-values if they are equidistant       */
  bool    is_equidist; /* Are the x-values equidistant?                      */
} Hfunction_1d;


/****************************************************************************/
/*  Parameter passing (inside library and for language interfaces)          */
/****************************************************************************/

typedef union
{
  INT4_8* l; /* 4-8 Byte integer                             */
  double* f; /* 8 Byte real                                  */
  char*   s; /* Pointer to string                            */
} DPHpar;    /* For HALCON/C                                 */


typedef union
{
  INT4_8 l; /* 4-8 Byte integer                             */
  double f; /* 8 Byte real                                  */
  char*  s; /* Pointer to string                            */
} DVHpar;   /* For HALCON/C                                 */


/****************************************************************************/
/*  Type for callback function for HALCON low level error messages          */
/****************************************************************************/

#ifndef _WIN32
#  define __stdcall
#endif
typedef void(__stdcall* HLowLevelErrorCallbackProc)(const char* err);

#ifndef HC_NO_STRUCT_MEMBER_ALIGNMENT
#  pragma pack(pop) /* unset struct member alignment */
#endif
#endif
