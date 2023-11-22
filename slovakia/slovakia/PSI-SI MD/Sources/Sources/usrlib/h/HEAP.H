#ifndef __INC_HEAP_H__

#define __INC_HEAP_H__

typedef unsigned char	uchar  ;

//#define _USER_DEBUG 

void *scr_realloc( char *filename, int lineno, void *src, size_t size ) ;
void *scr_calloc ( char *filename, int lineno, size_t n_items, size_t size ) ;
void *scr_malloc ( char *filename, int lineno, size_t size ) ;
char *scr_strdup ( const char *filename, int lineno, const char *src ) ;
char *scr_tempnam( char *filename, int lineno, char *dir, char *prefix ) ;
void  scr_free   ( const char *filename, int lineno, void *adr ) ;

void      heap_dump  ( char *title, int show_report ) ;
void  scr_heap_report( char *title, int show_report ) ;
int   scr_heap_check ( int showReport ) ;

#ifdef _USER_DEBUG
    #define REALLOC( ptr, size)         scr_realloc( __FILE__, __LINE__, ptr, size)
    #define  CALLOC( n_items, size)     scr_calloc ( __FILE__, __LINE__, n_items, size)
    #define  MALLOC( size)              scr_malloc ( __FILE__, __LINE__, size)
    #define  STRDUP( str)               scr_strdup ( __FILE__, __LINE__, str)
    #define TEMPNAM( dir, prefix)       scr_tempnam( __FILE__, __LINE__, dir, prefix)
    #define    FREE( adr)               scr_free   ( __FILE__, __LINE__, adr)
    #define  MALLOCINDIRECT( size , file , line ) scr_malloc ( file , line , size )
#else
    #define SCR_SMALLBLOCK        1024
    #define REALLOC( ptr, size)         realloc( ptr, size)
    #define  CALLOC( n_items, size)     calloc( n_items, size)
    #define  MALLOC( size)              malloc( size)
    #define  STRDUP( str)               strdup( str)
    #define TEMPNAM( dir, prefix)       tempnam( dir, prefix)
    #define    FREE( adr)               free( adr)
    #define  MALLOCINDIRECT( size , file , line ) malloc( size)
#endif

#endif
