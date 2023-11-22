/*
 *	Filename:		fileio.cpp
 *
 *	Version:		1.00
 *
 *	Description:	
 *					
 *
 *	History:
*/

#include "tools2.hpp"
#include "mux.hpp"
#include "MuxPacket.hpp"

#include <errno.h>
#include "inbox.hpp"
#include "ClientCfg.hpp"
#include "ServCfg.hpp"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// a = no aliasing
// g = global
// t = fast machine code
// y = no stack frame
#define OPTOPTIONS	"agt"

#pragma warning(disable:4083)	// disable 'expected string' warning

struct FECArray
{
	int size1, size2 ;
	FECDesc desc1 ;
	FECDesc desc2 ;
	FECDesc desc3 ;
} ;


//
// These numbers determine the cycles used for various block size.
// The values were derived from experiments.
//
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Allowed values: 2,4, odd values 3..125
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
static FECArray fecArr[] = 
{
	{ 3,    19, {2,   2,  3,  0, 0}, {1,  3,  0,  0,0}, {1,  2, 0, 0,0} },
	{ 20,   39, {2,   5,  7,  0, 0}, {2,  3,  4,  0,0}, {1,  3, 0, 0,0} },
	{ 40,   59, {3,   3,  7, 11, 0}, {2,  5,  7,  0,0}, {2,  2, 3, 0,0} },
	{ 60,   89, {3,   7, 11, 13, 0}, {3,  3,  5, 11,0}, {2,  3, 5, 0,0} },
	{ 90,  119, {3,  11, 13, 17, 0}, {2, 11, 15,  0,0}, {2,  3, 7, 0,0} },
	{ 120, 159, {3,  15, 17, 23, 0}, {3,  7, 13, 15,0}, {2,  3,11, 0,0} },
	{ 160, 199, {3,  13, 25, 33, 0}, {3,  7, 15, 23,0}, {2, 13,15, 0,0} },
	{ 200, 249, {3,  11, 39, 41, 0}, {3, 13, 21, 23,0}, {3,  3, 7,11,0} },
	{ 250, 299, {3,  17, 41, 49, 0}, {3, 13, 25, 29,0}, {3,  3,11,13,0} },
	{ 300, 359, {3,  11, 57, 63, 0}, {3, 19, 31, 33,0}, {3,  7,11,15,0} },
	{ 360, 429, {3,  17, 67, 73, 0}, {3, 23, 35, 39,0}, {3,  7,15,17,0} },
	{ 430, 511, {3,  19, 83, 85, 0}, {3, 19, 49, 51,0}, {3, 11,17,19,0} },
	{ 512, 512, {4,  17, 87, 89,13}, {3, 31, 47, 49,0}, {3, 11,19,21,0} },
	{ 513, 599, {4,  21, 89, 97,13}, {3, 17, 57, 61,0}, {3, 13,19,21,0} },
	{ 600, 699, {4,  17,111,117,15}, {3, 31, 65, 67,0}, {3, 19,21,25,0} },
	{ 700, 799, {4,  87, 97, 99,17}, {3, 43, 71, 75,0}, {3, 15,29,31,0} },
	{ 800, 899, {4,  87,111,115,25}, {3, 35, 87, 89,0}, {3, 23,29,31,0} },
	{ 900,1023, {4, 101,123,125,31}, {3, 29,103,107,0}, {3, 23,35,37,0} },
	{1024,1024, {4, 119,123,125,33}, {3, 47,101,107,0}, {3, 25,39,41,0} }
} ;

#define n_fecArr	(sizeof(fecArr) / sizeof(FECArray))

BOOL getFecParams(			// FALSE iff blkSize is too small to effectivelly define FEC
	int blkSize,			// IN : num packets per FEC block
	int redundancyClass,	// IN : 1-10%, 2-25%, 3-40%
	FECDesc *fd				// OUT
	)
{
	if( redundancyClass < 1  ||  redundancyClass > 3 )
		return FALSE ;
	if( blkSize < fecArr[0].size1  ||  blkSize > fecArr[n_fecArr-1].size2 )
		return FALSE ;

	FECArray *arr ;
	int j1=0, j2=n_fecArr-1 ;
	while( 1 )
	{
		int j = (j1+j2)/2 ;
		arr = fecArr + j ;

		if( blkSize < arr->size1 )
		{
			if( j2 == j )
			{
				arr-- ;
				break ;
			}
			j2 = j ;
		}
		else
		if( blkSize > arr->size2 )
		{
			if( j1 == j )
			{
				arr++ ;
				break ;
			}
			j1 = j ;
		}
		else
			break ;
	}

	*fd = (&arr->desc1)[ 3-redundancyClass] ;
	return TRUE ;
}

/*
// test
int main( int argc, char **argv )
{
	char *perc[] = { "40%", "25%", "10%" } ;
	srand( time(0) ) ;
	while( 1 )
	{
		int blk = 1 + rand() % 1023 ;
		int cls = rand() % 3 ;
		FECDesc fd ;
		printf( "\nBLD=%4d %s -> ", blk, perc[cls] ) ;
		if( !getFecParams( blk, cls, &fd) )
			printf( "failed" ) ;
		else
		{
			for( int j=0 ; j < fd.numCycles ; ++j )
				printf( "%3d ", fd.cycles[j] ) ;
		}
	}
	return 0 ;
}
*/

/*

// Another test program which tests MuxPacket::set/getFecInfo()

#include "tools2.hpp"
#include "mux.hpp"
#include "fileIo.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CWinApp theApp;

#define MAXBLKSIZE	1000
int  USE_PESHEADER ;

void testFec()
{
	for( int redundancyClass=1 ; redundancyClass < 3 ; ++redundancyClass )
	{
		for( ushort blkSize=0 ; blkSize <= 750 ; blkSize++ )
		{
			FECDesc fd ;
			if( !getFecParams( blkSize, redundancyClass, &fd) )
				continue ;
			printf( "\n%d/%d", redundancyClass, blkSize ) ;

			ushort maxBlk = (blkSize >= 512) ? MAXBLKSIZE : 0 ;
			for( ushort blockInd=100 ; blockInd <= maxBlk ; ++blockInd )
			{
				for( int c=0 ; c < fd.numCycles ; ++c )
				{
					uchar cycleLength = fd.cycles[c] ;
					for( uchar rowInd=0 ; rowInd < cycleLength ; ++rowInd )
					{
						while( 1 )
						{
							MuxPacket p ;
							p.setFecInfo( cycleLength, rowInd, blockInd, blkSize ) ;

							uchar  cycleLength1, rowInd1 ;
							ushort blockInd1, blkSize1 ;
							p.getFecInfo( cycleLength1, rowInd1, blockInd1, blkSize1 ) ;
							if( cycleLength==cycleLength1  &&  rowInd==rowInd1  &&  blockInd==blockInd1  &&  blkSize==blkSize1 )
								break ;
							TRACE( "\nerr: %d/%d/%d/%d/%d",
								redundancyClass, blkSize, blockInd, cycleLength, rowInd ) ;
						}
					}
				}
			}
		}
	}
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	testFec() ;
	return 0 ;
}
*/
