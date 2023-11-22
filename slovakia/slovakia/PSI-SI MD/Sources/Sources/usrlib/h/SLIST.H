#ifndef __S_LIST_H
#define __S_LIST_H

extern "C"
{
#include "heap.h"
}

struct __sPOSITION { };
typedef __sPOSITION* sPOSITION;

// Template for a list of objects of the type <T>.
//
// use: sListPtr<className> obj ;

template < class T >
class sListPtr
{
	struct sListPtrItem
	{
		sListPtrItem* _next;
		sListPtrItem* _prev;
		T	        * _data;
		sListPtrItem( T* data ) { _data = data; _next = 0; _prev=0 ; }
	};
public:

	sListPtr()	{ _head=0; _tail=0; _cnt =0; }
	sListPtr( sListPtr<T> &list, BOOL (*checkFun)( T *data, void *appData), void *appData ) ;
	sListPtr( sListPtr<T> &list, BOOL (*checkFun)( T *data)=NULL ) ;
   ~sListPtr()  { removeAll( ) ; }

	sPOSITION        rewind() const   { return (sPOSITION)_head ; }
	sPOSITION firstPosition() const   { return rewind() ; }
	sPOSITION  lastPosition() const   { return (sPOSITION)_tail ; }

	T*  next ( sPOSITION& pos ) ;
	T*  prev ( sPOSITION& pos ) ;
	T*  getAt( sPOSITION& pos ) ;
	T*  head ( )                      { return _head ? _head->_data : NULL ; }

	void replace( sPOSITION& pos, T *data ) ;

	// return pointer removed from the list
	T *  removeHead       ( ) ;
	T *  removeTail       ( ) ;
	T *  remove           ( sPOSITION& pos ) ;

	void removeWithData   ( sPOSITION& pos ) ;
	void removeAll        ( ) ;
	void removeAllWithData( ) ;

	// find & remove; -> success
	BOOL remove           ( T* what );
	BOOL removeWithData   ( T* what ); 

	// return position of the new item
	sPOSITION addHead( T *what ) ;
	sPOSITION addTail( T *what ) ;
	sPOSITION add    ( sPOSITION &pos, T *what ) ;	// insert before pos

	// Call f() on all members.
	// f() -> FALSE stops the execution
	// return= TRUE iff f() returned TRUE for all members
	BOOL callForAll( BOOL (*f)( T *data, void *appData), void *appData=NULL ) ;
	BOOL callForAll( BOOL (*f)( T *data) ) ;

	// Return 1st item where check function returns TRUE.
	// To get the value, use
	//   list->getAt( find(fun))
	// (Works also for NULL.)
	sPOSITION find( BOOL (*f)( T *data, void *appData), void *appData=NULL ) ;
	sPOSITION find( BOOL (*f)( T *data) ) ;

	void sort( int (*compare )(const T *elem1, const T *elem2) ) ;

	sPOSITION findPtr( T* what );	// -> item position / NULL

	int  count  () const  { return  _cnt ; }
	BOOL isEmpty() const  { return  _cnt <= 0 ; }

private:
	sListPtrItem* _head;
	sListPtrItem* _tail;
	size_t        _cnt ;
} ;

template< class T>
sPOSITION sListPtr<T>::add( sPOSITION &rPosition, T *what )
{
	sListPtrItem* item = (sListPtrItem*)rPosition ;
	if( !item  ||  !item->_next )
		return  addTail( what ) ;
	if( !item->_prev )
		return  addHead( what ) ;

	sListPtrItem *newItem = new sListPtrItem( what );
	sListPtrItem *prevItem= item->_prev ;
	item->_prev     = newItem ;
	prevItem->_next = newItem ;
	newItem->_prev  = prevItem ;
	newItem->_next  = item ;
	_cnt++ ;
	return  (sPOSITION)newItem ;
}

template< class T>
sPOSITION sListPtr<T>::addHead( T *what )
{
	sListPtrItem* newItem = new sListPtrItem( what ) ;
	if( !_head )
	{
		_head = _tail = newItem ;
	}
	else
	{
		_head->_prev   = newItem ;
		newItem->_next = _head ;
		_head = newItem ;
	}
	_cnt++ ;
	return (sPOSITION)_head ;
}

template< class T>
sPOSITION sListPtr<T>::addTail( T *what )
{
	sListPtrItem* newItem = new sListPtrItem( what ) ;
	if( !_tail )
	{
		_head = _tail = newItem ;
	}
	else
	{
		_tail->_next   = newItem ;
		newItem->_prev = _tail ;
		_tail = newItem ;
	}
	_cnt++ ;
	return (sPOSITION)_tail ;
}

template< class T >
inline T* sListPtr<T>::next( sPOSITION& rPosition )
{
	sListPtrItem* item = (sListPtrItem*) rPosition;
	if( !item )
		return  0 ;
	rPosition = (sPOSITION) item->_next;
	return item->_data ;
}

template< class T >
inline T* sListPtr<T>::getAt( sPOSITION& rPosition )
{
	sListPtrItem* item = (sListPtrItem*) rPosition;
	return  item ? item->_data : 0 ;
}

template< class T>
inline T* sListPtr<T>::prev( sPOSITION& rPosition )
{
	sListPtrItem* item = (sListPtrItem*) rPosition;
	if( !item )
		return  0 ;
	rPosition = (sPOSITION) item->_prev ;
	return item->_data ;
}

template< class T>
inline void sListPtr<T>::replace( sPOSITION& rPosition, T *data )
{
	ASSERT( (sListPtrItem*)rPosition != NULL ) ;
	((sListPtrItem*)rPosition)->_data = data ;
}


//---------------------------------------------------------

template< class T>
sPOSITION sListPtr<T>::findPtr( T *what )
{
	for( sListPtrItem *item=_head ; item != NULL ; item=item->_next )
		if( item->_data == what )
			return  (sPOSITION)item ;
	return (sPOSITION)0 ;
}

template< class T>
inline BOOL sListPtr<T>::remove( T* what )
{
	sPOSITION tmpPos = findPtr( what );
	if (!tmpPos) return 0;
	remove( tmpPos );
	return 1;
}

template< class T>
BOOL sListPtr<T>::removeWithData( T* what )
{
	sPOSITION tmpPos = findPtr( what );
	if (!tmpPos) return 0;
	removeWithData( tmpPos );
	return 1;
}

//---------------------------------------------------------

template< class T>
void sListPtr<T>::removeAll()
{
	while( _head )
	{
		sListPtrItem* tmp= _head->_next;
		delete _head;
		_head = tmp;
	}
	_tail= 0 ;
	_cnt = 0 ;
}

template< class T>
void sListPtr<T>::removeAllWithData()
{
	while( _head )
	{
		sListPtrItem* tmp= _head->_next;
		delete _head->_data ;
		delete _head;
		_head = tmp;
	}
	_tail= 0 ;
	_cnt = 0 ;
}

template< class T>
T *sListPtr<T>::removeHead( )
{
	if( !_head )
		return  0 ;
	T *data = _head->_data ;
	sListPtrItem *item = _head ;
	_head = _head->_next ;
    if( _head != NULL )
    	_head->_prev = 0 ;
    else
        _tail = NULL ;
	_cnt-- ;
	delete  item ;
	return  data ;
}

template< class T>
T *sListPtr<T>::removeTail( )
{
	if( !_tail )
		return  0 ;
	T *data = _tail->_data ;
	sListPtrItem *item = _tail ;
	_tail = _tail->_prev ;
    if( _tail != NULL )
	    _tail->_next = 0 ;
    else
        _head = NULL ;
	_cnt-- ;
	delete  item ;
	return  data ;
}

template< class T>
T *sListPtr<T>::remove( sPOSITION& rPosition )
{
	sListPtrItem* item = (sListPtrItem*) rPosition;
	if( item == NULL )
		return  NULL ;
	if( !item->_next )
	{
		ASSERT( item == _tail ) ;
		return  removeTail( ) ;
	}
	if( !item->_prev )
	{
		ASSERT( item == _head ) ;
		return  removeHead( ) ;
	}

	T *data = item->_data ;
	item->_prev->_next = item->_next ;
	item->_next->_prev = item->_prev ;
	_cnt-- ;
	delete  item ;
	return  data ;
}


template< class T>
inline void sListPtr<T>::removeWithData( sPOSITION& rPosition )
{
	T* tmp = remove( rPosition );
	delete tmp;
}

//---------------------------------------------------------

template< class T>
sPOSITION sListPtr<T>::find( BOOL (*f)( T *data) )
{
	ASSERT( f != NULL ) ;
	for( sListPtrItem *item=_head ; item != NULL ; item=item->_next )
	{
		if( (*f)( item->_data) == TRUE )
			return  (sPOSITION)item ;
	}
	return  NULL ;
}

template< class T>
sPOSITION sListPtr<T>::find( BOOL (*f)( T *data, void *appData), void *appData )
{
	ASSERT( f != NULL ) ;
	for( sListPtrItem *item=_head ; item != NULL ; item=item->_next )
	{
		if( (*f)( item->_data, appData) == TRUE )
			return  (sPOSITION)item ;
	}
	return  NULL ;
}

template< class T>
BOOL sListPtr<T>::callForAll( BOOL (*f)( T *data, void *appData), void *appData )
{
	ASSERT( f != NULL ) ;
	for( sListPtrItem *item=_head ; item != NULL ; item=item->_next )
	{
		if( (*f)( item->_data, appData) == FALSE )
			return  FALSE ;
	}
	return  TRUE ;
}

template< class T>
BOOL sListPtr<T>::callForAll( BOOL (*f)( T *data) )
{
	ASSERT( f != NULL ) ;
	for( sListPtrItem *item=_head ; item != NULL ; item=item->_next )
	{
		if( (*f)( item->_data) == FALSE )
			return  FALSE ;
	}
	return  TRUE ;
}


template< class T>
void sListPtr<T>::sort( int (*compare )(const T *elem1, const T *elem2) )
{
	for( int cnt=_cnt-1 ; cnt > 0 ; cnt-- )
	{
		int j, ready = 1 ;
		sListPtrItem *item, *nextItem ;
		for( j=cnt, item=_head ; j > 0 ; item=nextItem , j-- )
		{
			nextItem = item->_next ;
			if( (*compare)( item->_data, nextItem->_data) > 0 )
			{
				T *tmp = item->_data ;
				item->_data = nextItem->_data ;
				nextItem->_data = tmp ;
				ready = 0 ;
			}
		}
		if( ready )  break ;
	}
}


template< class T>
sListPtr<T>::sListPtr( sListPtr<T> &list, BOOL (*f)( T *data, void *appData), void *appData )
{
	_head=0 ;
	_tail=0 ;
	_cnt =0 ;
	for( sListPtrItem *item=list._head ; item != NULL ; item=item->_next )
		if( (*f)( item->_data, appData) )
			addTail( item->_data ) ;
}

template< class T>
sListPtr<T>::sListPtr( sListPtr<T> &list, BOOL (*f)( T *data) )
{
	_head=0 ;
	_tail=0 ;
	_cnt =0 ;
	for( sListPtrItem *item=list._head ; item != NULL ; item=item->_next )
		if( f == NULL  ||  (*f)( item->_data) )
			addTail( item->_data ) ;
}


/******************************************************************************/
/*                                  sTemplateArray                             */
/******************************************************************************/


void *___sTemplateArrayAlloc( void *_first, int allocNum, int sizeofT, int &itemsNum, int &bufSiz );

template<class T> class sTemplateArray
{
    void    (*_delF)(T*) ;

  protected:
    int       siz, bufSiz;
    T        *_first     ;

  public:
           BOOL copy        ( const sTemplateArray<T> *list         );          
           int  find        ( void *i, BOOL (*_find)( void *, T *), int from = 0  ) const;
           int  find        ( const T&, int from =0) const ;
           void clearList   ( BOOL deleteItems = TRUE ) ;
           BOOL del         ( int i   );
           BOOL delObj      ( const T &val ) ;
           BOOL remove      ( int i ) ;
		   BOOL removeObj	( const T &val ) ;

		   const char *saveToBin ( char *bin, BOOL checked=FALSE ) const;
	inline size_t binarySize( ) const                                           { return  2*sizeof(size_t) + sizeof(ushort) + size() ; }    //Binary size of the list
		   char *loadFromBin( char *bin, BOOL clearOldContents=TRUE ) ;

		   T   *addArray	( const T *arr, int n ) ;		// memcpy array of T elements
		   T   *add         ( const T &i ) ;
	inline T   *insert      ( int atposition, const T &i) ;
		   T   *alloc       ( int n  ) ;					// = pointer to first allocated item

		   BOOL isEqual		( const sTemplateArray<T> &list );	
	inline void setCount	( int n ) ;
    inline void sort        ( int (*compFunc)(const void *e1, const void *e2))  { qsort( (void *)_first, (size_t)siz, sizeof( T ), compFunc );};
    inline void exchange    ( int i, int j)                                     { ASSERT(i>=0 && j>=0 && i<siz && j<siz); T a = _first[i]; _first[i]=_first[j]; _first[j]=a;} ;
    inline int  count       ( void   ) const                                    { return siz;            } ;
    inline int  size        (        ) const                                    { return siz*sizeof( T); };
    inline T   &item        ( int i  ) const                                    { ASSERT(i>=0 && i<siz );   return _first[i];     } ;
    inline T   &first       (        ) const                                    { ASSERT(siz>0);            return _first[0];     } ;
    inline T   &last        (        ) const                                    { ASSERT(siz>0);            return _first[siz-1]; } ;

    inline void  setDelFunc ( void (*_del)(T*))                                 { _delF=_del;           } ;     
    inline void *getDelFunc ( )                                                 { return _delF ;        } ;     
    
    inline                          operator T*(      ) const                   { return _first;        } ;
    inline T                       &operator [](int i )                         { ASSERT( i>=0 && i<siz );  return _first[i];     } ;
    inline const sTemplateArray<T> &operator = ( const sTemplateArray<T> &list) { copy( &list);				return *this;} ;
    inline const sTemplateArray<T> &operator <<( const T &t )                   { add( t );                 return *this;} ;
           const sTemplateArray<T> &operator <<( const sTemplateArray<T> &list) ;
	inline const BOOL				operator ==( const sTemplateArray<T> &list)	{ return  isEqual(list); }
	inline const BOOL				operator !=( const sTemplateArray<T> &list)	{ return !isEqual(list); }
    
    inline sTemplateArray( void (*_del)(T*) = NULL )                            { bufSiz=0;siz=0;_first=NULL;_delF=_del; };
    inline sTemplateArray( const sTemplateArray<T> &list, void (*_del)(T*)=NULL){ bufSiz=0;siz=0;_first=NULL;_delF=_del; copy( &list );};
    inline ~sTemplateArray()                                                    { clearList(); };
} ;

/*
#LSave list to binary file
#BReturn:
    #Nreturns bin + binarySize()

#BRemark:
    #Nbinary format:
    (size_t)  nItems
    (size_t)  nItems+checked; checked=0/1 
    (ushort)  crc (0 if not checked)
    ... items ...
*/
template<class T> const char * sTemplateArray<T> :: saveToBin( char *bin, BOOL checked ) const
{
	if( checked )
		checked = 1 ;
	size_t cnt      = count() ;
	size_t cntBytes = size( ) ;

	*(size_t*)bin = cnt ;
	bin += sizeof( size_t ) ;

	cnt += checked ;
	*(size_t*)bin = cnt ;
	bin += sizeof( size_t ) ;

	ushort crc = (ushort)((checked  &&  cntBytes > 0) ? crc16( (char*)_first, cntBytes) : 0) ;

	*(ushort*)bin = crc ;
	bin += sizeof(ushort) ;

	if( cntBytes > 0 )
	{
		memcpy( bin, _first, cntBytes ) ;
		bin += cntBytes ;
	}
	return  bin ;
}

/*
#LLoad list from binary file
#BReturn:
    #N TRUE on successs

#BRemark:
  #NFormat see in #AsaveToBin
*/
template<class T> char *sTemplateArray<T> :: loadFromBin( char *bin, BOOL clearOldContents ) 
{
	size_t cnt     = *(size_t*)bin ;
	size_t cntBytes= cnt * sizeof(T) ;
	bin += sizeof( size_t ) ;

	int    checked = *(size_t*)bin - cnt ;
	bin += sizeof( size_t ) ;

	ushort crc     = *(ushort*)bin ;
	bin += sizeof( ushort ) ;

	if( checked == 1 )
	{
		if( crc != crc16( bin, cntBytes) )
			return  NULL ;
	}
	else
	if( checked != 0 )
		return  NULL ;

	if( clearOldContents )
		clearList( ) ;
	if( cnt > 0 )
	{
		size_t oldSize = size() ;
		alloc( cnt ) ;
		memcpy( _first+oldSize, bin, cntBytes ) ;
	}
	return  bin+cntBytes ;
}

//Clear all items in this list
template<class T> void sTemplateArray<T> :: clearList( BOOL deleteItems )
{
    if( _first != NULL )
    {
        if( deleteItems && _delF  != NULL ) 
            for( int i=0; i<siz; i++ )
                _delF( _first+i ) ;
        FREE( (void*)_first );
    }
    siz     = 0;
    bufSiz  = 0;
   _first   = NULL;
} ; 


template<class T> T* sTemplateArray<T> :: addArray( const T *arr, int n )
{
	T *t ;
	if( siz+n > bufSiz )
		t = alloc( n ) ;
	else
	{
		t = _first+siz ;
		siz += n ;
	}
	memcpy( t, arr, n*sizeof(T) ) ;
	return t ;
}

template<class T> T* sTemplateArray<T> :: add( const T &i )
{
	T *t ;
	if( siz >= bufSiz )
		t = alloc(1) ;
	else
	{
		t = _first + siz++ ;
		memset( t, 0, sizeof(T) ) ;
	}

	*t = i ;
	return t ;
}

template<class T> T* sTemplateArray<T> :: alloc( int n )
{
	if( n <= 0 )
		return NULL;
	int s = siz;
	_first = (T*)___sTemplateArrayAlloc( (void*)_first, n, sizeof(T), siz, bufSiz );
	return _first ? &_first[s] : NULL;
}

template<class T> void sTemplateArray<T>::setCount( int n )
{
	if( n > siz )
	{
		if( n <= bufSiz )
			memset( _first+siz, 0, (n-siz) * sizeof(T) ) ;
		else
			alloc( n - siz ) ;	// alloc clears added items
	}
	siz = n ;
}


//Copy of the list 'list' to this one
template<class T> BOOL sTemplateArray<T> :: copy( const sTemplateArray<T> *list )
{
	int listCnt = list->count() ;
	if( listCnt <= 0 )
	{
		clearList() ;
		return 1;
	}
	if( _delF != NULL ) 
		clearList() ;

	if( listCnt > bufSiz )
		alloc( listCnt-siz ) ;
	else
		siz = listCnt ;

	for( int i=0 ; i < listCnt ; i++ )
		_first[i] = list->_first[i];
	return 1;
}

template<class T> BOOL sTemplateArray<T> :: isEqual( const sTemplateArray<T> &list )
{
	if( count() != list.count() )
		return FALSE;
    for( int i=0; i < count(); i++ )
        if( _first[i] != list[i] )
            return FALSE;
	return TRUE;
};

template<class T> const sTemplateArray<T> &sTemplateArray<T> :: operator<<( const sTemplateArray<T> &list )
{
	int listCnt = list.count() ;
	if( listCnt > 0 )
	{
		int ind0 = siz ;
		alloc( listCnt ) ;
		for( int i=0 ; i < listCnt ; i++ )
			_first[ind0+i] = list._first[i];
	}
	return *this;
}

template<class T> int sTemplateArray<T> :: find( void *val, BOOL (*_find)( void *, T *), int from ) const
{
    if( from < 0 || from >= siz )	return -1;
    for( int i=from; i < siz; i++ )
        if( _find( val, &_first[i] ) )
            return i;
    return -1;
};

template<class T> int sTemplateArray<T> :: find( const T &val, int from ) const
{
    if( from < 0 || from >= siz )	return -1;
    for( int i=from; i < siz; i++ )
        if( val == _first[i] )
            return i;
    return -1;
};


// delFun() is automatic called (if defined)
template<class T> BOOL sTemplateArray<T> :: del( int i )
{
    if( i < 0 || i >= siz )
        return FALSE;
    siz--;
    if( _delF != NULL )
		_delF( _first+i ) ;
    for( ; i<siz; i++)
		_first[i] = _first[i+1]; 
    return TRUE;
} ;

template<class T> BOOL sTemplateArray<T> :: delObj( const T &val )
{
    for( int i=0; i < siz; i++ )
        if( val == _first[i] )
            return del( i );
    return  FALSE;
}

template<class T> BOOL sTemplateArray<T>::remove( int i )
{
	if( i < 0 || i >= siz )
		return FALSE ;
	siz-- ;
	for( ; i < siz ; i++ )
		_first[i] = _first[i + 1] ;
	return TRUE ;
}
		
template<class T> BOOL sTemplateArray<T> :: removeObj( const T &val )
{
    for( int i=0; i < siz; i++ )
        if( val == _first[i] )
            return remove( i );
    return  FALSE;
}

template<class T> T *sTemplateArray<T> :: insert( int atposition, const T &i)
{
	if( siz >= bufSiz )
		alloc(1) ;
	else
		siz++ ;
	for( int j = siz - 1 ; j > atposition ; j--)
		_first[j] = _first[j - 1]; 
	return &( _first[atposition] = i ) ;
}

#endif