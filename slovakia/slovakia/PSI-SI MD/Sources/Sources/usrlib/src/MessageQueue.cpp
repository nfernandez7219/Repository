#include "tools2.hpp"
#include <afxole.h>
#include <mq.h>
#include "MessageQueue.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////////
// MSMQ functions

HRESULT MSMQLocateQueue( char *szQueueLabel, MQSpecificationPtrArray &queues ) ;
HRESULT MSMQCreateQueue( char *szPathNameBuffer, char *szLabelBuffer, char *szFormatNameBuffer ) ;
HRESULT MSMQDeleteQueue( char *szFormatNameBuffer ) ;
HRESULT MSMQOpenQueue( char *szFormatNameBuffer, DWORD dwAccess, HANDLE &hQueue ) ;
HRESULT MSMQCloseQueue( HANDLE hQueue ) ;
HRESULT MSMQSendMessage( HANDLE hQueue, UCHAR *msg, int nMsgBytes, const char *szLabel=NULL, MSMQSendMode *sendMode=NULL ) ;
HRESULT MSMQReceiveMessage( HANDLE hQueue, UCHAR *msg, ULONG &maxMsgBytes, char *szLabel, UINT *classType, DWORD dwTimeout, DWORD action ) ;

static void MQSpecificationDelFn( MQSpecification**p )
{
	delete *p;
}

MQSpecificationPtrArray::MQSpecificationPtrArray( )
	: sTemplateArray<MQSpecification*>( MQSpecificationDelFn ) 
{
}

static MSMQSendMode s_sendMode ;

///////////////////////////////////////////////////////////////////////////////////
// MessageQueue class

MessageQueue::MessageQueue( )
{
	_hQueue = NULL; 
	_formatName[0] = 0; 
	_lastResult = 0;
}

MessageQueue::~MessageQueue	( )
{
	close();
}

BOOL MessageQueue::_create( char *pathName, char *label )
{
	_lastResult = MSMQCreateQueue( pathName, label, _formatName );
	return ! FAILED(_lastResult);
}

static DWORD getAccessCode( DWORD dwAccess )	// access code convertor (you needn't include mq.hpp header => you need to use MessageQueue access codes)
{
	switch(dwAccess)
	{
		case MessageQueue::SendAccess:
			return MQ_SEND_ACCESS;
		case MessageQueue::ReceiveAccess:
			return MQ_RECEIVE_ACCESS;
		case MessageQueue::PeekAccess:
			return MQ_PEEK_ACCESS;
		default:
			return NULL;
	}
}

BOOL MessageQueue::open( char *pathName, DWORD dwAccess )
{
	char formatName[MAX_Q_FORMATNAME_LEN];
	if ( ! findFormatName( formatName, MAX_Q_FORMATNAME_LEN, pathName ) )
	{
		_lastResult = MQ_ERROR_ILLEGAL_QUEUE_PATHNAME;
		return FALSE;
	}
	return _open( formatName, dwAccess );
}

BOOL MessageQueue::_open( char *formatName, DWORD dwAccess )
{
	strcpy(_formatName,formatName);

	_lastResult = MSMQOpenQueue( formatName, getAccessCode(dwAccess), _hQueue ) ;	// fills _hQueue
	return ! FAILED(_lastResult);
}

BOOL MessageQueue::close( )
{
	if ( ! isOpened() )
		return FALSE;
	_lastResult = MSMQCloseQueue( _hQueue ) ;
	BOOL bSucceed = ! FAILED(_lastResult);
	if ( bSucceed )
		_hQueue = NULL;
	return bSucceed ;
}

BOOL MessageQueue::_delete( char *formatName )
{
	if ( formatName == NULL || formatName[0] == 0 )
		return FALSE;

	_lastResult = MSMQDeleteQueue( formatName ) ;
	return ! FAILED(_lastResult);
}

BOOL MessageQueue::send( UCHAR *msg, ULONG nMsgBytes, const char *label, MSMQSendMode *sendMode )
{
	if ( ! isOpened() )
		return FALSE;

	if ( sendMode == NULL )
		sendMode = &s_sendMode;

	_lastResult = MSMQSendMessage( _hQueue, msg, nMsgBytes, label, sendMode);
	return ! FAILED(_lastResult);
}

BOOL MessageQueue::receive( UCHAR *msg, ULONG &maxMsgBytes, char *label, BOOL msgFilter, DWORD dwTimeout, UINT *classType )
{
	if ( ! isOpened() )
		return FALSE;
	
	UINT uiClassType;
	if ( classType == NULL )
		classType = &uiClassType;

	do
	{
		_lastResult = MSMQReceiveMessage(_hQueue, msg, maxMsgBytes, label, classType, dwTimeout, MQ_ACTION_RECEIVE );
		if ( FAILED(_lastResult) )
			return FALSE;

	} while ( msgFilter && (*classType != MQMSG_CLASS_NORMAL ) );

	return TRUE;
}

BOOL MessageQueue::peek( UCHAR *msg, ULONG maxMsgBytes, char *label, DWORD dwTimeout, UINT *classType )
{
	if ( ! isOpened() )
		return FALSE;
	
	UINT uiClassType;
	if ( classType == NULL )
		classType = &uiClassType;

	_lastResult = MSMQReceiveMessage(_hQueue, msg, maxMsgBytes, label, classType, dwTimeout, MQ_ACTION_PEEK_CURRENT );
	if ( FAILED(_lastResult) )
		return FALSE;

	return TRUE;
}

BOOL MessageQueue::isLastErrBufferSmall( )
{
	return _lastResult == MQ_ERROR_BUFFER_OVERFLOW ;
}

BOOL MessageQueue::isLastErrTimeout( )
{
	return _lastResult == MQ_ERROR_IO_TIMEOUT ;
}

////////////////////////////////////////////////////////////////////////////////////////
//	static methods

BOOL MessageQueue::findFormatName( char *buf, int bufSize, char *path )
{
	// convert path to unicode
	int pathLen = strlen(path);
	WCHAR wPath[MAX_Q_PATHNAME_LEN];
	mbstowcs(wPath, path, pathLen);
	wPath[pathLen] = 0;

	// find format name
	DWORD fnSize = MAX_Q_FORMATNAME_LEN;
	WCHAR formatName[MAX_Q_FORMATNAME_LEN];
	HRESULT lastResult;
	try { lastResult = MQPathNameToFormatName( wPath, formatName, &fnSize); }
	catch(Msg &msg)
	{
		throw msg;
	}
	catch(...)
	{
		throw Msg(-1, "MSMQ error.");
	}

	if ( FAILED(lastResult) )
		return FALSE;

	// convert format name from unicode
	wcstombs(buf, formatName, bufSize);
	if ( fnSize >= bufSize )	// small buffer
		fnSize = bufSize - 1;
	buf[fnSize] = 0;

	return TRUE;
}

void MessageQueue::locateQueue( char *label, MQSpecificationPtrArray &locatedQueues )
{
	try
	{
		MSMQLocateQueue(label, locatedQueues); 
	}
	catch(Msg &msg)
	{
		throw msg;
	}
	catch(...)
	{
		throw Msg(-1, "MSMQ error.");
	}
}

// ConnectString syntax: machine\queue_name[format_name]

void MessageQueue::composeConnectString( char *connectString, const char *formatName, const char *path )
{
	sprintf(connectString,"%s[%s]", path, formatName);
}

void MessageQueue::decomposeConnectString( char* formatName, char *path, const char *connectString )
{
	int len = strlen(connectString);
	if (len-- == 0)
		return;
	char buf[MAX_Q_PATHNAME_LEN];
	strcpy(buf, connectString);
	if ( buf[len] == ']' )
		buf[len] = 0;					// delete last ']' parenthese
	char *formatN = strrchr(buf,'[');	// find last '[' parenthese
	if ( formatN != NULL )
	{
		// store format name
		*formatN = 0;	
		formatN++;
		if (formatName)
			strncpy(formatName, formatN, MAX_Q_FORMATNAME_LEN);
	}
	else
		if (formatName)
			formatName[0] = 0;				// empty format name
	// store queue path
	if (path)
		strncpy(path, buf, MAX_Q_PATHNAME_LEN);
}

BOOL MessageQueue::validateConnectString ( char *connString )
{
	char path[MAX_Q_PATHNAME_LEN];
	char format[MAX_Q_FORMATNAME_LEN];
	decomposeConnectString(format, path, connString);
	BOOL bSucceed;
	try
	{
		bSucceed = findFormatName(format, MAX_Q_FORMATNAME_LEN, path);
		if (bSucceed)
			composeConnectString(connString, format, path);
	}
	catch(...)
	{
		bSucceed = FALSE;
	}
	return bSucceed ;
}

///////////////////////////////////////////////////////////////////////////////////
// MSMQ functions - implementation

#define BUFFERSIZE			256
#define MAX_VAR				20

static void AnsiStringToUnicode( WCHAR *lpsUnicode, const char *lpsAnsi, DWORD  nSize )
{
    if (lpsUnicode == NULL)
    {
        return;
    }

    ASSERT(lpsAnsi != NULL);

    size_t rc = mbstowcs(lpsUnicode, lpsAnsi, nSize);
    ASSERT(rc != (size_t)(-1));
    if (lpsUnicode[nSize-1] != L'\0')
        lpsUnicode[nSize] = L'\0';
}

#define MAXINDEX 31

//----------------------------------------------------------------------
//	MSMQCreateQueue()
//----------------------------------------------------------------------


static HRESULT MSMQCreateQueue( char *szPathNameBuffer, char *szLabelBuffer, char *szFormatNameBuffer )
{
    MQPROPVARIANT aVariant[MAXINDEX];
    QUEUEPROPID aPropId[MAXINDEX];
    DWORD PropIdCount = 0;

	WCHAR wPath[MAX_Q_PATHNAME_LEN];
	WCHAR wLabel[MAX_Q_PATHNAME_LEN];
	
	AnsiStringToUnicode( wPath, szPathNameBuffer, strlen(szLabelBuffer) );
	AnsiStringToUnicode( wLabel, szLabelBuffer, strlen(szLabelBuffer) );
    //
    // Set the PROPID_Q_PATHNAME property
    aPropId[PropIdCount] = PROPID_Q_PATHNAME;    //PropId
    aVariant[PropIdCount].vt = VT_LPWSTR;        //Type
    aVariant[PropIdCount].pwszVal = wPath;

    PropIdCount++;

    //
    // Set the PROPID_Q_LABEL property
    aPropId[PropIdCount] = PROPID_Q_LABEL;    //PropId
    aVariant[PropIdCount].vt = VT_LPWSTR;     //Type
    aVariant[PropIdCount].pwszVal = wLabel;

    PropIdCount++;

    //
    // Set the MQEUEUPROPS structure
    MQQUEUEPROPS QueueProps;
    QueueProps.cProp = PropIdCount;           //No of properties
    QueueProps.aPropID = aPropId;             //Id of properties
    QueueProps.aPropVar = aVariant;           //Value of properties
    QueueProps.aStatus = NULL;                //No error reports

    //
    // No security (default)
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;

    //
    // Create the queue
    HRESULT hr;
    DWORD dwFormatNameBufferLength = MAX_Q_FORMATNAME_LEN;
	try
	{
		WCHAR szwFormatNameBuffer[MAX_Q_FORMATNAME_LEN];
		hr = MQCreateQueue(
				pSecurityDescriptor,            //Security
				&QueueProps,                    //Queue properties
				szwFormatNameBuffer,            //Output: Format Name
				&dwFormatNameBufferLength       //Output: Format Name len
				);

		if (SUCCEEDED(hr))
		{
			size_t rc = wcstombs(szFormatNameBuffer, szwFormatNameBuffer, dwFormatNameBufferLength);
			ASSERT(rc != (size_t)(-1));
		}
	}
	catch( Msg &msg )
	{
		throw msg;
	}
	catch( ... )
	{
		throw Msg(-1,"MSMQ error");
	}

	return hr ;
}


//----------------------------------------------------------------------
//	MSMQDeleteQueue()
//----------------------------------------------------------------------


static HRESULT MSMQDeleteQueue( char *szFormatNameBuffer )
{
    HRESULT hr;
	WCHAR szwFormatNameBuffer[MAX_Q_FORMATNAME_LEN];
	AnsiStringToUnicode( szwFormatNameBuffer, szFormatNameBuffer, strlen(szFormatNameBuffer) );

	hr = MQDeleteQueue(szwFormatNameBuffer);  // FormatName of the Queue to be deleted.
	
	return hr ;
}


//----------------------------------------------------------------------
//	MSMQDeleteQueue()
//----------------------------------------------------------------------


static HRESULT MSMQOpenQueue( char *szFormatNameBuffer, DWORD dwAccess, HANDLE &hQueue )
{
    HRESULT hr;

	WCHAR szwFormatNameBuffer[MAX_Q_FORMATNAME_LEN];
	AnsiStringToUnicode( szwFormatNameBuffer, szFormatNameBuffer, strlen(szFormatNameBuffer) );

	hr = MQOpenQueue(
			szwFormatNameBuffer,    // Format Name of the queue to be opened.
			dwAccess,               // Access rights to the Queue.
			MQ_DENY_NONE ,
			&hQueue                 // OUT: handle to the opened Queue.
			);

	return hr ;
}

static HRESULT MSMQCloseQueue( HANDLE hQueue )
{
    return MQCloseQueue( hQueue );
}


//----------------------------------------------------------------------
//	MSMQSendQueue()
//----------------------------------------------------------------------


MSMQSendMode::MSMQSendMode( )
{
	memset( this, 0, sizeof(MSMQSendMode) ) ;
	nAcknowledge = MQMSG_ACKNOWLEDGMENT_NONE ;
	nPriority=3 ;
	dwTimeToReachQueue=LONG_LIVED ;
	dwTimeToBeReceived=LONG_LIVED ;
	nDelivery = MQMSG_DELIVERY_EXPRESS ;

	dwTimeToReachQueue = -1;
	dwTimeToBeReceived = -1;
	bJournal = FALSE;
	bDeadLetter = FALSE;
	bAuthenticated = FALSE;
	bEncrypted = FALSE ;

	szAdminFormatName[0] = 0;	//	fill this before sending message
}

#define SND_DEFAULT_FLAGS			ExpressDelivery
#define SND_ACKNOWLEDGMENT_FLAGS	(FullReachAcknowledge | NegReachAcknowledge | FullRcvAcknowledge | NegRcvAcknowledge)

void MSMQSendMode::setFlags( unsigned int flags )
{
	if( flags == Defaults )
		flags = SND_DEFAULT_FLAGS ;

	bAuthenticated	= (flags & Authenticate) ;
	bEncrypted		= (flags & Encrypt) ;
	bJournal		= (flags & CopyToJournalBeforeSend) ;
	bDeadLetter		= (flags & UseDeadLetterQueue) ;
	nDelivery		= (flags & ExpressDelivery) ? MQMSG_DELIVERY_EXPRESS : MQMSG_DELIVERY_RECOVERABLE ;

	switch( flags & SND_ACKNOWLEDGMENT_FLAGS )
	{
		case FullReachAcknowledge: nAcknowledge = MQMSG_ACKNOWLEDGMENT_FULL_REACH_QUEUE; break ;
		case NegReachAcknowledge : nAcknowledge = MQMSG_ACKNOWLEDGMENT_NACK_REACH_QUEUE; break ;
		case FullRcvAcknowledge  : nAcknowledge = MQMSG_ACKNOWLEDGMENT_FULL_RECEIVE; break ;
		case NegRcvAcknowledge   : nAcknowledge = MQMSG_ACKNOWLEDGMENT_NACK_RECEIVE; break ;
		default : nAcknowledge = MQMSG_ACKNOWLEDGMENT_NONE ;
	}
}

// prepare the property array PROPVARIANT.
#define SET_PROPERTY( propId, variantType)	{	\
	aPropId[PropIdCount] = propId;				\
	aVariant[PropIdCount].vt = variantType;		\
	PropIdCount++;								\
}

static HRESULT MSMQSendMessage( HANDLE hQueue, UCHAR *msg, int nMsgBytes, const char *szLabel, MSMQSendMode *sendMode )
{
	MSMQSendMode *sm = sendMode ? sendMode : &s_sendMode ;

    MQMSGPROPS MsgProps;
    MQPROPVARIANT aVariant[MAXINDEX];
    MSGPROPID aPropId[MAXINDEX];
    DWORD PropIdCount = 0;

    aVariant[PropIdCount].bVal = sm->nPriority;
	SET_PROPERTY( PROPID_M_PRIORITY, VT_UI1) ;

    aVariant[PropIdCount].bVal = sm->nDelivery;
	SET_PROPERTY( PROPID_M_DELIVERY, VT_UI1) ;

    aVariant[PropIdCount].caub.cElems = nMsgBytes ;
    aVariant[PropIdCount].caub.pElems = (unsigned char *)msg ;
	SET_PROPERTY( PROPID_M_BODY, VT_VECTOR|VT_UI1) ;

	WCHAR wcLabel[MAX_Q_PATHNAME_LEN];
	if( szLabel != NULL )
	{
		AnsiStringToUnicode(wcLabel, szLabel, strlen(szLabel) );
		aVariant[PropIdCount].pwszVal = wcLabel ;
		SET_PROPERTY( PROPID_M_LABEL, VT_LPWSTR) ;
	}

    aVariant[PropIdCount].ulVal = sm->dwTimeToReachQueue;
	SET_PROPERTY( PROPID_M_TIME_TO_REACH_QUEUE, VT_UI4) ;

    aVariant[PropIdCount].ulVal = sm->dwTimeToBeReceived;
	SET_PROPERTY( PROPID_M_TIME_TO_BE_RECEIVED, VT_UI4) ;

    if( sm->bJournal || sm->bDeadLetter )
    {
        aVariant[PropIdCount].bVal = sm->bJournal ? MQMSG_JOURNAL : 0 ;
        if( sm->bDeadLetter )
            aVariant[PropIdCount].bVal |= MQMSG_DEADLETTER;

		SET_PROPERTY( PROPID_M_JOURNAL, VT_UI1) ;
    }

	if( sm->bAuthenticated )
	{
		aVariant[PropIdCount].ulVal = MQMSG_AUTH_LEVEL_ALWAYS;
		SET_PROPERTY( PROPID_M_AUTH_LEVEL, VT_UI4) ;
	}

	if( sm->bEncrypted )
	{
		aVariant[PropIdCount].ulVal = MQMSG_PRIV_LEVEL_BODY;
		SET_PROPERTY( PROPID_M_PRIV_LEVEL, VT_UI4) ;
	}

	if( sm->nAcknowledge != MQMSG_ACKNOWLEDGMENT_NONE )
	{
		aVariant[PropIdCount].bVal = sm->nAcknowledge;
		SET_PROPERTY( PROPID_M_ACKNOWLEDGE, VT_UI1) ;

		WCHAR szwAdminFormatNameBuffer[MAX_Q_FORMATNAME_LEN];
		AnsiStringToUnicode(szwAdminFormatNameBuffer, sm->szAdminFormatName, strlen(sm->szAdminFormatName) );
		aVariant[PropIdCount].pwszVal = szwAdminFormatNameBuffer;

		SET_PROPERTY( PROPID_M_ADMIN_QUEUE, VT_LPWSTR) ;
	}

    //
    // Set the MQMSGPROPS structure
    MsgProps.cProp = PropIdCount;       //Number of properties.
    MsgProps.aPropID = aPropId;         //Id of properties.
    MsgProps.aPropVar = aVariant;       //Value of properties.
    MsgProps.aStatus  = NULL;           //No Error report.

    //
    // Send the message.
    HRESULT hr = MQSendMessage(
            hQueue,                     // handle to the Queue.
            &MsgProps,                  // Message properties to be sent.
            NULL                        // No transaction
            );
	return hr ;
}


//----------------------------------------------------------------------
//	MSMQReceiveQueue()
//----------------------------------------------------------------------


// MQ_OK, MQ_ERROR_IO_TIMEOUT, other codes
static HRESULT MSMQReceiveMessage( HANDLE hQueue,
						   UCHAR *msg, ULONG &maxMsgBytes,
						   char *szLabel,				// <= 250 Unicode chars
						   UINT  *classType,			// see help for PROPID_M_CLASS
						   DWORD dwTimeout,				// [msec]; can be INFINITE
						   DWORD action
						  )
{
    MQMSGPROPS	  MsgProps;
    MQPROPVARIANT aVariant[MAXINDEX];
    MSGPROPID	  aPropId[MAXINDEX];
    DWORD		  PropIdCount = 0;

    //
    // prepare the property array PROPVARIANT of message properties that we want to receive

	// 0 = message data
    aVariant[PropIdCount].caub.cElems = maxMsgBytes ;
    aVariant[PropIdCount].caub.pElems = msg ;
	SET_PROPERTY( PROPID_M_BODY, VT_VECTOR|VT_UI1) ;

	// 1 = message label
    WCHAR szMessageLabelBuffer[BUFFERSIZE];
    aVariant[PropIdCount].pwszVal = szMessageLabelBuffer;
	SET_PROPERTY( PROPID_M_LABEL, VT_LPWSTR) ;

	// 2 = class type
	SET_PROPERTY( PROPID_M_CLASS, VT_UI2) ;

	// 3 = max. allowed label length
    aVariant[PropIdCount].ulVal = 250 ;				// max. allowed label length [Unicode]
	SET_PROPERTY( PROPID_M_LABEL_LEN, VT_UI4) ;

	SET_PROPERTY( PROPID_M_BODY_SIZE, VT_NULL);

	//SET_PROPERTY( PROPID_M_PRIORITY, VT_UI1) ;

	//SET_PROPERTY( PROPID_M_AUTHENTICATED, VT_UI1) ;

	// User id; use LookupAccountSid() to convert to user name
    //BYTE  blobBuffer[BUFFERSIZE];
	//aVariant[PropIdCount].blob.pBlobData = blobBuffer;
	//aVariant[PropIdCount].blob.cbSize = sizeof(blobBuffer);
	//SET_PROPERTY( PROPID_M_SENDERID, VT_UI1|VT_VECTOR) ;

	//SET_PROPERTY( PROPID_M_PRIV_LEVEL, VT_UI4) ;

    //
    // Set the MQMSGPROPS structure
    MsgProps.cProp    = PropIdCount;    //Number of properties.
    MsgProps.aPropID  = aPropId;        //Id of properties.
    MsgProps.aPropVar = aVariant;       //Value of properties.
    MsgProps.aStatus  = NULL;           //No Error report.

    //
    // Receive the message.
    HRESULT hr = MQReceiveMessage(
               hQueue,               // handle to the Queue.
               dwTimeout,            // Max time (msec) to wait for the message.
               action,    // Action.
               &MsgProps,            // properties to retrieve.
               NULL,                 // No overlaped structure.
               NULL,                 // No callback function.
               NULL,                 // No Cursor.
               NULL                  // No transaction
               );

	maxMsgBytes = aVariant[4].ulVal; // get data size (even when error occured)
	*classType = aVariant[2].uiVal ;
	if(hr == MQ_OK)
	{
		ASSERT( wcstombs( szLabel, szMessageLabelBuffer, 512) != (size_t)(-1) ) ;
	}
	return hr ;
}

static HRESULT MSMQLocateQueue( char *szQueueLabel, MQSpecificationPtrArray &queues )
{
	queues.clearList();

    MQPROPERTYRESTRICTION PropertyRestriction;
    MQRESTRICTION  Restriction;
    MQCOLUMNSET    Column;
    QUEUEPROPID    aPropId[2]; // only two properties to retrieve.
    MQPROPVARIANT   aPropVar[MAX_VAR] = {0};
    DWORD           dwColumnCount = 0;
    DWORD dwFormatNameLength = MAX_Q_FORMATNAME_LEN;

    //
    // Prepare property restriction.
    // Restriction = All queue with PROPID_Q_LABEL equal to "MQ API test".
    //
    PropertyRestriction.rel = PREQ;
    PropertyRestriction.prop = PROPID_Q_LABEL;
    PropertyRestriction.prval.vt = VT_LPWSTR;

	// convert label to unicode
	DWORD size = _tcslen(szQueueLabel) +1;
	WCHAR wcLabel[MAX_PATH];
	AnsiStringToUnicode(wcLabel, szQueueLabel, size);
	PropertyRestriction.prval.pwszVal = wcLabel;

    //
    // prepare a restriction with one property restriction.
    Restriction.cRes = 1;
    Restriction.paPropRes = &PropertyRestriction;

    //
    // Columnset (In other words what property I want to retrieve).
    // Only the PathName is important.
    aPropId[dwColumnCount] = PROPID_Q_PATHNAME;
    dwColumnCount++;

    aPropId[dwColumnCount] = PROPID_Q_INSTANCE;
    dwColumnCount++;

    Column.cCol = dwColumnCount;
    Column.aCol = aPropId;

    //
    // Locate the queues. Issue the query
    HANDLE  hEnum;
    HRESULT hr = MQLocateBegin(
            NULL,           //start search at the top.
            &Restriction,   //Restriction
            &Column,        //ColumnSet
            NULL,           //No sort order
            &hEnum          //Enumeration Handle
            );

    if(FAILED(hr))
		return hr ;

    //
    // If cQueue == 0 it means that no Variants were retrieved in the last MQLocateNext.
    while( 1 )
    {
	    DWORD  cQueue=MAX_VAR;		// # of queues retrieved
        hr = MQLocateNext(
                hEnum,      // handle returned by MQLocateBegin.
                &cQueue,    // size of aPropVar array.
                aPropVar    // OUT: an array of MQPROPVARIANT to get the results in.
                );

        if(FAILED(hr))
        {
			MQLocateEnd(hEnum) ;
			return hr ;
        }

		if( cQueue == 0 )
			break ;

        for( DWORD i=0; i<cQueue; i++)
        {
            //
            // add the new path names to the path name array.
            MQSpecification *pArrayQ = new MQSpecification;
			size_t rc = wcstombs(pArrayQ->szPathName, aPropVar[i].pwszVal, MAX_Q_PATHNAME_LEN);
			ASSERT(rc != (size_t)(-1));
            //
            // move to the next property.
            i = i + 1;
			try
            {
				// Get the FormatName of the queue and set it in the PathName array.
				WCHAR szwFormatNameBuffer[MAX_Q_FORMATNAME_LEN];
				hr = MQInstanceToFormatName(aPropVar[i].puuid, szwFormatNameBuffer, &dwFormatNameLength);
				if (SUCCEEDED(hr))
				{
					size_t rwc =wcstombs(pArrayQ->szFormatName, szwFormatNameBuffer, dwFormatNameLength);
					ASSERT(rwc != (size_t)(-1));
				}

				//
				// Free the memory allocated by MSMQ
				MQFreeMemory(aPropVar[i].pwszVal);
			}
			catch( Msg &msg )
			{
				delete pArrayQ ;
				MQLocateEnd(hEnum) ;
				throw msg;
			}
			catch(...)
			{
				delete pArrayQ ;
				MQLocateEnd(hEnum) ;
				throw Msg(-1,"MSMQ error.");
			}

            if(FAILED(hr))
            {
				delete pArrayQ ;
				MQLocateEnd(hEnum) ;
				return hr ;
            }

			queues.add( pArrayQ ) ;
        }
    }

    //
    // End the locate operation.
    hr = MQLocateEnd(hEnum);   // handle returned by MQLocateBegin.
	return hr ;
}