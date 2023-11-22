
#include "stdafx.h"
#include "TableHolders.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// uncomment to disable loading PAT and PMT from file
//#define LOAD_DUMMY_TABLES

void BaseHolderDelFn( BaseHolder **holder )
{
	delete *holder ;
	*holder = NULL ;
}

BaseHolderArray::BaseHolderArray( )
{
	setDelFunc(BaseHolderDelFn) ;
}

//--------------------------//
//		PAT Holder
//--------------------------//

PAT_Holder::PAT_Holder( )
 : BaseHolder(PAT_HOLDER) 
{
	_nitPid = 0x10 ;
}

void PAT_Holder::load( PsiData *psiData )
{
	PATable *pat = (PATable*)psiData->_privateSections ;
	int nSections = psiData->_nSections ;

	_transportStreamId = pat->getTransportStreamId() ;

	while (nSections--)
	{
		ASSERT(pat->table_id==TABLE_ID_PAT) ;

		int nPrograms = pat->getNumberOfPrograms() ;
		for ( int i = 0; i < nPrograms; i++ )
		{
			ushort prgNum = pat->getProgramNum(i) ;
			if (prgNum)
			{
				PatProgram program(prgNum, pat->getProgramPID(i) );
				_patPrograms.add(program) ;
			}
			else
				_nitPid = pat->getProgramPID(i) ;
		}

		pat = (PATable*)( ((uchar*)pat) + pat->getTotalLength() ) ;
	}
	setHolderType(PAT_HOLDER) ;
}

#pragma message("PAT_Holder::save ma obmedzenie na 252 programov.")
void PAT_Holder::save( PsiData *psiData )
{
	uchar buffer[MAX_PAT_SIZE] ;
	PATable *pat = (PATable*)buffer ;

	int nPrograms = 1+_patPrograms.count() ;
	if (nPrograms > MAX_PAT_PROGRAMS)
		throw ;

	pat->create( nPrograms, _transportStreamId ) ;
	pat->setProgram( 0, 0, _nitPid ) ; 

	for ( int i = 1; i < nPrograms; i++ )
	{
		PatProgram* program = &_patPrograms.item(i-1) ;
		pat->setProgram( i, program->program_number, program->program_map_PID ) ; 
	}

	int totalSize = pat->getTotalLength() ;
	psiData->_privateSections = (PrivateSection*)malloc(totalSize) ;
	memcpy(psiData->_privateSections, pat, totalSize) ;

	psiData->_nSections = 1 ;
	psiData->_pid = TABLE_PID_PAT ;
	psiData->_freq = 25 ;
}

/*
BOOL PAT_Holder::delChild( BaseHolder *child)
{ 
	if (child->getHolderType()==PMT_HOLDER )
	{
		int res = AfxGetMainWnd()->MessageBox(
			"Do you want to delete selected program ?\n\n"
			"Remember that you should delete\n"
			"also associated service in SDT table",
			"Question",
			MB_ICONQUESTION | MB_YESNO
		) ;

		if (res==IDYES)
		{
			_streams.delObj(streamHolder) ;
			return TRUE ;
		}
		return FALSE ;
	}

	return FALSE ;
}
*/

ushort PAT_Holder::findPID( ushort programNumber )
{
	int i = _patPrograms.count() ;
	while ( i-- )
	{
		PatProgram *prg = &_patPrograms[i] ;
		if (prg->program_number == programNumber )
			return prg->program_map_PID ;
	}

	if (programNumber==0)
		return _nitPid ;

	return 0 ;
}

void PAT_Holder::deleteProgram( ushort programNumber )
{
	int i = _patPrograms.count() ;
	while ( i-- )
	{
		PatProgram *prg = &_patPrograms[i] ;
		if (prg->program_number == programNumber )
			_patPrograms.del(i) ;
	}
}

int	 PAT_Holder::getItemNumber( )
{
	return 1+_patPrograms.count() ;
}

void PAT_Holder::getValueText( int itemIndex, char *buf )
{
	if ( itemIndex<0 || itemIndex>_patPrograms.count() )
		return ;

	if (itemIndex==0)
	{
		sprintf(buf,"0x%x",_nitPid);
		strupr(buf+2);
	}
	else
	{
		PatProgram *prg = &_patPrograms[itemIndex-1] ;
		int pid = prg->program_map_PID ;
		sprintf(buf, "PID 0x%x", pid ) ;
		strupr(buf+6);
	}
}

void PAT_Holder::getItemName( int itemIndex, char *buf )
{
	if ( itemIndex<0 || itemIndex>_patPrograms.count() )
		return ;

	if (itemIndex==0)
		sprintf(buf, "Network Information PID" ) ;
	else
	{
		PatProgram *prg = &_patPrograms[itemIndex-1] ;
		int prgNum = prg->program_number ;
		sprintf(buf, "Program %d", prgNum ) ;
	}
}

BOOL PAT_Holder::editItem( int itemIndex )
{
	if ( itemIndex<0 || itemIndex>_patPrograms.count() )
		return FALSE ;

	BOOL changed ;

	if ( itemIndex==0 )
	{
		int pid = _nitPid ;

		changed = EditIntValue(
			TRUE,
			"Network Information PID",
			"PID value on which NIT (Network Information Table) "
			"is broadcasted. Should be 0x10",
			&pid,
			0x10,
			0x1F
		) ;

		if (changed)
			_nitPid = pid ;
	}
	else
	{
		PatProgram *prg = &_patPrograms[itemIndex-1] ;
		int pid = prg->program_map_PID ;
	
		char desc[255] ;
		sprintf(desc,
			"PID value on which PMT (Program Map Table) "
			"is broadcasted for program %d",
			prg->program_number ) ;

		changed = EditIntValue(
			TRUE,
			"Program map PID",
			desc,
			&pid,
			0x20,
			0x1FFE
		) ;

		if (changed)
			prg->program_map_PID = pid ;
	}

	if (changed)
		_changeFlag = TRUE ;

	return changed ;
}

//--------------------------//
//		CAT Holder
//--------------------------//

CAT_Holder::CAT_Holder( )
 : BaseHolder(CAT_HOLDER) 
{
}

void CAT_Holder::load( PsiData *psiData )
{
	CATable *cat = (CATable*)psiData->_privateSections ;
	int nSections = psiData->_nSections ;

	while (nSections--)
	{
		ASSERT(cat->table_id==TABLE_ID_CAT) ;

		_descriptors.loadFromBin(cat->descriptors, cat->getDescLength() ) ;
		
		cat = (CATable*)( ((uchar*)cat) + cat->getTotalLength() ) ;
	}
	setHolderType(CAT_HOLDER) ;

	int i = _descriptors.count() ;
	while (i--)
	{
		DescriptorHolder *desc = _descriptors[i] ;
		if (desc->getHolderType()==TAG_CA_descriptor)
		{
			CASystemDescHolder *caDesc = (CASystemDescHolder*)desc ;
			caDesc->setCatFlag() ;
		}
	}
}

void CAT_Holder::save( PsiData *psiData )
{
	uchar buffer[2000] ;
	CATable *cat = (CATable*)buffer ;

	cat->create( ) ;
	uchar *end = _descriptors.saveToBin(cat->descriptors) ;

	int totalSize = 4+int(end-buffer) ;
	if (totalSize > 1021)
		throw ;

	cat->setSectionLength(totalSize) ;
	cat->computeCRC() ;

	psiData->_privateSections = (PrivateSection*)malloc(totalSize) ;
	memcpy(psiData->_privateSections, cat, totalSize) ;

	psiData->_nSections = 1 ;
	psiData->_pid = TABLE_PID_CAT ;
	psiData->_freq = 25 ;
}

CASystemDescHolder* CAT_Holder::getItem( int index )
{
	DescriptorHolder *descHolder = _descriptors[index] ;
	ASSERT(descHolder->getHolderType()==TAG_CA_descriptor) ;
	CASystemDescHolder *caHolder = (CASystemDescHolder*) descHolder ;

	return caHolder ;
}

DescriptorHolder* CAT_Holder::addItem( )
{
	int res = AfxGetMainWnd()->MessageBox(
		"Do you want to add CA system ?\n\n"
		"Note that it will created with DEFAULT values.\n"
		"You CAN EDIT them by double click.",
		"Question",
		MB_ICONQUESTION | MB_YESNO
	) ;

	if (res==IDYES)
	{
		CASystemDescHolder *holder = new CASystemDescHolder(TRUE) ;
		_descriptors.add(holder) ;
		return holder ;
	}

	return NULL ;
}

int	 CAT_Holder::getItemNumber( )
{
	return _descriptors.count() ;
}

void CAT_Holder::getValueText( int itemIndex, char *buf )
{
	if ( itemIndex<0 || itemIndex>getItemNumber() )
		return ;

	DescriptorHolder *desc = _descriptors[itemIndex] ;
	desc->getTextDescription(buf) ;
}

void CAT_Holder::getItemName( int itemIndex, char *buf )
{
	if ( itemIndex<0 || itemIndex>getItemNumber() )
		return ;

	DescriptorHolder *descHolder = _descriptors[itemIndex] ;
	strcpy(buf, descriptorName(descHolder->getType()) ); 
}

BOOL CAT_Holder::editItem( int itemIndex )
{
	if ( itemIndex<0 || itemIndex>getItemNumber() )
		return FALSE ;

	DescriptorHolder *descHolder = _descriptors[itemIndex] ;
	return descHolder->editItem(itemIndex);
}

//--------------------------//
//		PMT Holder
//--------------------------//

PmtStreamHolder::PmtStreamHolder( uchar streamType, ushort elemPID )
 : BaseHolder(PMT_STREAM_HOLDER) 
{ 
	_StreamType=streamType; 
	_ElementaryPID=elemPID; 
}

uchar *PmtStreamHolder::loadFromBin( uchar *buffer )
{
	PmtStream *stream = (PmtStream*)buffer ;

	_StreamType = stream->streamType() ;
	_ElementaryPID = stream->elementaryPid() ;

	_descriptors.loadFromBin(stream->descriptors(), stream->descSize());

	setHolderType(PMT_STREAM_HOLDER) ;

	return buffer + stream->getLength() ;
}

uchar *PmtStreamHolder::saveToBin( uchar *buffer )
{
	PmtStream *stream = (PmtStream*)buffer ;

	stream->setStreamType(_StreamType) ;
	stream->setElementaryPid(_ElementaryPID);

	uchar *descEnd = _descriptors.saveToBin(stream->descriptors()) ;
	stream->setDescSize( ushort(descEnd-stream->descriptors()) ) ;

	return buffer + stream->getLength() ;
}

static BOOL delDescrQuestion()
{
	int res = AfxGetMainWnd()->MessageBox(
		"Do you want to delete selected descriptor ?" ,
		"Question",
		MB_ICONQUESTION | MB_YESNO
	) ;

	return (res==IDYES) ;
}

BOOL PmtStreamHolder::delChild( BaseHolder *child)
{ 
	if (child->isDescHolder() && delDescrQuestion())
	{
		DescriptorHolder *descHolder = (DescriptorHolder*)child ;
		_descriptors.delObj(descHolder) ;
		return TRUE ;
	}
	else
		return FALSE ;

}

int	 PmtStreamHolder::getItemNumber( )
{
	return 2+_descriptors.count() ;
}

void PmtStreamHolder::getItemName( int itemIndex, char *buf )
{
	if ( itemIndex<0 || itemIndex>getItemNumber() )
		return ;

	switch (itemIndex)
	{
		case 0:	strcpy(buf, "Stream Type"); break ;
		case 1: strcpy(buf, "Elementary PID"); break ;
		default: 
		{
			DescriptorHolder *descHolder = _descriptors.item(itemIndex-2) ;
			strcpy(buf, descriptorName(descHolder->getType()) ); 
			break ;
		}
	} ;
}

void PmtStreamHolder::getValueText( int itemIndex, char *buf )
{
	if ( itemIndex<0 || itemIndex>getItemNumber() )
		return ;

	switch (itemIndex)
	{
		case 0:	PmtStream::streamTypeName(_StreamType, buf); break ;
		case 1: 
			sprintf(buf,"0x%x",_ElementaryPID);
			strupr(buf+2);
			break ;
		default: 
		{
			DescriptorHolder *descHolder = _descriptors.item(itemIndex-2) ;
			descHolder->getTextDescription(buf) ; 
			break ;
		}
	} ;
}

BOOL PmtStreamHolder::editItem( int itemIndex )
{
	if (itemIndex==0)
	{
		int		vals[14] ;
		LPTSTR	texts[14] ;
		int i=0 ;
		vals[i++] = PmtStream::video_MPEG1;				
		vals[i++] = PmtStream::video_MPEG2;				
		vals[i++] = PmtStream::audio_MPEG1;				
		vals[i++] = PmtStream::audio_MPEG2;				
		vals[i++] = PmtStream::private_section;			
		vals[i++] = PmtStream::PES_with_private_data;	
		vals[i++] = PmtStream::MHEG;					
		vals[i++] = PmtStream::DSMCC;					
		vals[i++] = PmtStream::ITU_T_Rec_H_221_1;		
		vals[i++] = PmtStream::DSMCC_type_A;			
		vals[i++] = PmtStream::DSMCC_type_B;			
		vals[i++] = PmtStream::DSMCC_type_C;			
		vals[i++] = PmtStream::DSMCC_type_D;			
		vals[i++] = PmtStream::auxiliary;				
						
		i = 14 ;
		while (i--)
		{
			char b[128] ;
			PmtStream::streamTypeName(vals[i], b) ;
			texts[i] = strdup(b) ;
		}

		int selected = _StreamType ;
		BOOL res =EditEnumValue(
			FALSE,
			"Stream type",
			"Select type of the stream",
			14,
			vals,
			texts,
			&selected
		) ;

		i = 14 ;
		while (i--)
			free(texts[i]) ;

		if (res)
		{
			_StreamType = selected ;
			_changeFlag = TRUE ;
			return TRUE ;
		}

		return FALSE ;
	}

	if (itemIndex==1)
	{
		int pid = _ElementaryPID ;

		BOOL res =EditIntValue(
			TRUE,
			"Elementary PID",
			"PID value identifies the stream data packets",
			&pid,
			0x20,
			0x1FFE
		) ;

		if (res)
		{
			_ElementaryPID = pid ;
			_changeFlag = TRUE ;
			AfxGetMainWnd()->SendMessage(WM_COMMAND, ID_RefreshTreeItem) ;
			return TRUE ;
		}
	}

	return FALSE ;
}

static void PmtStreamHolderDelFn ( PmtStreamHolder **holder )
{
	delete *holder ;
	*holder = NULL ;
}

PmtStreamHolderArray::PmtStreamHolderArray( )
{
	setDelFunc(PmtStreamHolderDelFn) ;
}

PMT_Holder::PMT_Holder( ushort progNum )
 : BaseHolder(PMT_HOLDER) 
{ 
	_ProgramNumber = progNum ; 
	_PCR_PID = 0x1FFF ; 
}

PmtStreamHolder* PMT_Holder::addStream ( )
{
	int res = AfxGetMainWnd()->MessageBox(
		"Do you want to add a new program stream ?\n\n"
		"NOTE: Program stream will be created using DEFAULT values.\n"
		"You CAN EDIT them by double click",
		"Question",
		MB_ICONQUESTION | MB_YESNO
	) ;

	if (res==IDYES)
	{
		PmtStreamHolder *streamHolder = new PmtStreamHolder ;
		_streams.add(streamHolder) ;
		return streamHolder ;
	}

	return NULL ;
}

void PMT_Holder::load( PsiData *psiData )
{
	PMTable *pmt = (PMTable*)psiData->_privateSections ;

	ASSERT(pmt->table_id==TABLE_ID_PMT) ;
	ASSERT(psiData->_nSections == 1 ) ;	// according to EN 300 468 PMT must be contained in 1 section

	_ProgramNumber = pmt->getProgramNumber() ;
	_PCR_PID = pmt->getPcrPid() ;

	int descSize = pmt->getProgramInfoLength() ;
	_descriptors.loadFromBin(pmt->descriptors, descSize );

	int nStreams = pmt->getNumStreams() ;

	uchar *stream = pmt->pmtStreams() ;
	for ( int i = 0; i < nStreams; i++ )
	{
		PmtStreamHolder *streamHolder = new PmtStreamHolder ;
		stream = streamHolder->loadFromBin(stream) ;
		_streams.add(streamHolder) ;
	}

	setHolderType(PMT_HOLDER) ;
}

#pragma message("PMT_Holder::save ma obmedzenie na 1 PMT sekciu.")
void PMT_Holder::save( PsiData *psiData )
{
	uchar buffer[5000] ;
	PMTable *pmt = (PMTable*)buffer ;
	pmt->create( _ProgramNumber, _PCR_PID ) ;

	uchar *pos = _descriptors.saveToBin( pmt->descriptors );
	pmt->program_info_length = ntols( (int)(pos-pmt->descriptors) ) ;

	int nStreams = _streams.count() ;
	for ( int i = 0; i < nStreams; i++ )
	{
		pos = _streams[i]->saveToBin(pos) ;
		if ( int(pos-buffer) > 1017) 
			throw ;
	}

	pmt->setSectionLength(4+int(pos-buffer)) ;
	pmt->computeCRC() ;

	int totalSize = pmt->getTotalLength() ;
	psiData->_privateSections = (PrivateSection*)malloc(totalSize) ;
	memcpy(psiData->_privateSections, pmt, totalSize) ;

	psiData->_nSections = 1 ;
	psiData->_pid = 0 ;
	psiData->_freq = 25 ;
}

BOOL PMT_Holder::delChild( BaseHolder *child)
{ 
	if (child->isDescHolder() && delDescrQuestion())
	{
		DescriptorHolder *descHolder = (DescriptorHolder*)child ;
		_descriptors.delObj(descHolder) ;
		return TRUE ;
	}
	else if (child->getHolderType()==PMT_STREAM_HOLDER )
	{
		int res = AfxGetMainWnd()->MessageBox(
			"Do you want to delete selected program stream ?" ,
			"Question",
			MB_ICONQUESTION | MB_YESNO
		) ;

		if (res==IDYES)
		{
			PmtStreamHolder *streamHolder = (PmtStreamHolder*)child ;
			_streams.delObj(streamHolder) ;
			return TRUE ;
		}
		return FALSE ;
	}

	return FALSE ;
}

int	 PMT_Holder::getItemNumber( )
{
	return 2+_descriptors.count()+_streams.count() ;
}

void PMT_Holder::getValueText( int itemIndex, char *buf )
{
	if ( itemIndex<0 || itemIndex>getItemNumber() )
		return ;

	switch (itemIndex)
	{
		case 0:	itoa(_ProgramNumber, buf, 10); return ;
		case 1: 
			if (_PCR_PID==0x1FFF)
				strcpy(buf, "Unused") ;
			else
			{
				sprintf(buf,"0x%x",_PCR_PID);
				strupr(buf+2);
			}
			return ;
	} ;

	itemIndex -= 2 ;

	if ( itemIndex<_descriptors.count() )
	{
		DescriptorHolder *descHolder = _descriptors.item(itemIndex) ;
		descHolder->getTextDescription(buf) ; 
		return ;
	}

	itemIndex -= _descriptors.count() ;

	PmtStreamHolder *streamHolder = _streams[itemIndex] ;
	streamHolder->getTextDescription(buf) ;
}

void PMT_Holder::getItemName( int itemIndex, char *buf )
{
	if ( itemIndex<0 || itemIndex>getItemNumber() )
		return ;

	switch (itemIndex)
	{
		case 0:	strcpy(buf, "Program Number"); return ;
		case 1: strcpy(buf, "Program Clock Reference PID"); return ;
	} ;

	itemIndex -= 2 ;

	if ( itemIndex<_descriptors.count() )
	{
		DescriptorHolder *descHolder = _descriptors.item(itemIndex) ;
		strcpy(buf, descriptorName(descHolder->getType()) );
		return ;
	}

	itemIndex -= _descriptors.count() ;

	PmtStreamHolder *streamHolder = _streams[itemIndex] ;
	sprintf(buf, "Stream on PID %d", streamHolder->_ElementaryPID) ;
}

BOOL PMT_Holder::editItem( int itemIndex )
{
	if (itemIndex==0)
	{
		// _ProgramNumber 

		return FALSE ;
	}

	if (itemIndex==1)
	{
		int pid = _PCR_PID ;

		BOOL res =EditIntValue(
			TRUE,
			"PCR PID",
			"Program Clock Reference PID value identifies the stream "
			"with clock reference packets. "
			"Choose value \"0x1FFF\" if program has no clock reference.",
			&pid,
			0x20,
			0x1FFF
		) ;

		if (res)
		{
			_PCR_PID = pid ;
			_changeFlag = TRUE ;
			return TRUE ;
		}
	}

	return FALSE ;
}

static void PmtHolderDelFn( PMT_Holder **holder )
{
	delete *holder ;
	*holder = NULL ;
}

PMT_HolderArray::PMT_HolderArray( )
{
	setDelFunc( PmtHolderDelFn ) ;
}

void PMT_HolderArray::deleteProgram( ushort programNumber )
{
	int i = count() ;
	while ( i-- )
	{
		PMT_Holder *pmt = item(i) ;
		if (pmt->getProgramNum() == programNumber )
			del(i) ;
	}
}

//--------------------------//
//		SDT Holder
//--------------------------//

SdtServiceHolder::SdtServiceHolder( ushort servId )
 : BaseHolder(SDT_SERVICE_HOLDER) 
{
	_serviceId = servId ;
	_EitPresentFlag = FALSE ;
	_EitScheduleFlag = FALSE ;
	_RunningStatus = SdtService::Running;
	_FreeCAMode = TRUE ;

//	_descriptors.add() ;
}

uchar *SdtServiceHolder::loadFromBin( uchar *buffer )
{
	SdtService *serv = (SdtService*)buffer ;

	_serviceId = serv->getServiceId() ;

	uchar *descStart = serv->descriptors ;
	ushort descSize  = serv->getDescLength() ;
	_descriptors.loadFromBin(descStart, descSize) ;

	_EitPresentFlag	= serv->EitPresent() ;
	_EitScheduleFlag= serv->EitSchedulePresent() ;
	_RunningStatus	= serv->getRunningStatus() ;
	_FreeCAMode		= serv->FreeCAMode() ;

	setHolderType(SDT_SERVICE_HOLDER) ;

	return descStart + descSize ;
}

uchar *SdtServiceHolder::saveToBin( uchar *buffer )
{
	SdtService *serv = (SdtService*)buffer ;

	serv->setServiceId(_serviceId) ;

	uchar *descStart = serv->descriptors ;
	uchar *descEnd   = _descriptors.saveToBin(descStart) ;

	serv->setDescLength( ushort(descEnd-descStart) ) ;

	serv->eit_flags = 0 ;
	if (_EitPresentFlag)
		serv->setEitPresent() ;

	if (_EitScheduleFlag)
		serv->setEitSchedulePresent() ;

	serv->setRunningStatus(_RunningStatus) ;
	if (_FreeCAMode)
		serv->setFreeCAMode() ;
	else
		serv->resetFreeCAMode() ;

	return descEnd ;
}

BOOL SdtServiceHolder::delChild( BaseHolder *child)
{ 
	if (child->isDescHolder() && delDescrQuestion())
	{
		DescriptorHolder *descHolder = (DescriptorHolder*)child ;
		_descriptors.delObj(descHolder) ;
		return TRUE ;
	}
	else
		return FALSE ;

}

int	 SdtServiceHolder::getItemNumber( )
{
	return 5+_descriptors.count() ;
}

void SdtServiceHolder::getItemName( int itemIndex, char *buf )
{
	if ( itemIndex<0 || itemIndex>getItemNumber() )
		return ;

	switch (itemIndex)
	{
		case 0:	strcpy(buf, "Service ID"); return ;
		case 1:	strcpy(buf, "Event Info Table present"); return ;
		case 2: strcpy(buf, "Event Info Table schedule present"); return ;
		case 3: strcpy(buf, "Running status"); return ;
		case 4: strcpy(buf, "CA mode"); return ;
	} ;

	itemIndex -= 5 ;

	DescriptorHolder *descHolder = _descriptors.item(itemIndex) ;
	strcpy(buf, descriptorName(descHolder->getType()) );
}

void SdtServiceHolder::getValueText( int itemIndex, char *buf )
{
	if ( itemIndex<0 || itemIndex>getItemNumber() )
		return ;

	switch (itemIndex)
	{
		case 0:	itoa(_serviceId, buf, 10); return ;
		case 1:	strcpy(buf, _EitPresentFlag?"Present":"Not present"); return ;
		case 2: strcpy(buf, _EitScheduleFlag?"Present":"Not present"); return ;
		case 3: SdtService::runningStatusAsText(_RunningStatus, buf); return ;
		case 4: strcpy(buf, _FreeCAMode?"Free":"Scrambled"); return ;
	} ;

	itemIndex -= 5 ;

	DescriptorHolder *descHolder = _descriptors.item(itemIndex) ;
	descHolder->getTextDescription(buf);
}

BOOL SdtServiceHolder::editItem( int itemIndex )
{
	if (itemIndex==1)
	{
		int		vals[2] ;
		LPTSTR	texts[2], presentSt="Present", notPresentSt="Not present" ;
		int i=0 ;
		vals[0] = 1 ;				
		vals[1] = 0 ;	
		texts[0] = presentSt ;
		texts[1] = notPresentSt ;
						
		int selected = _EitPresentFlag ;
		BOOL res =EditEnumValue(
			FALSE,
			"EIT present",
			"Event Information Table present",
			2,
			vals,
			texts,
			&selected
		) ;

		if (res)
		{
			_EitPresentFlag = selected ;
			_changeFlag = TRUE ;
			return TRUE ;
		}

		return FALSE ;
	}

	if (itemIndex==2)
	{
		int		vals[2] ;
		LPTSTR	texts[2], presentSt="Present", notPresentSt="Not present" ;
		int i=0 ;
		vals[0] = 1 ;				
		vals[1] = 0 ;	
		texts[0] = presentSt ;
		texts[1] = notPresentSt ;
						
		int selected = _EitScheduleFlag ;
		BOOL res =EditEnumValue(
			FALSE,
			"EIT present",
			"Event Information Table schedule present",
			2,
			vals,
			texts,
			&selected
		) ;

		if (res)
		{
			_EitScheduleFlag = selected ;
			_changeFlag = TRUE ;
			return TRUE ;
		}

		return FALSE ;
	}

	if (itemIndex==3)
	{
		int		vals[5] ;
		LPTSTR	texts[5] ;
		int i=0 ;
		vals[i++] = SdtService::Undefined ;
		vals[i++] = SdtService::NotRunning ;
		vals[i++] = SdtService::StartsInFewSeconds ;
		vals[i++] = SdtService::Pausing ;
		vals[i++] = SdtService::Running ;
						
		i = 5 ;
		while (i--)
		{
			char b[128] ;
			SdtService::runningStatusAsText(vals[i], b) ;
			texts[i] = strdup(b) ;
		}

		int selected = _RunningStatus ;
		BOOL res =EditEnumValue(
			FALSE,
			"Running status",
			"Select running status of the service",
			5,
			vals,
			texts,
			&selected
		) ;

		i = 5 ;
		while (i--)
			free(texts[i]) ;

		if (res)
		{
			_RunningStatus = selected ;
			_changeFlag = TRUE ;
			return TRUE ;
		}

		return FALSE ;
	}

	if (itemIndex==4)
	{
		int		vals[2] ;
		LPTSTR	texts[2], freeSt="Free", notFreeSt="Scrambled" ;
		int i=0 ;
		vals[0] = 1 ;				
		vals[1] = 0 ;	
		texts[0] = freeSt ;
		texts[1] = notFreeSt ;
						
		int selected = _FreeCAMode ;
		BOOL res =EditEnumValue(
			FALSE,
			"CA mode",
			"Select Conditional Access mode.",
			2,
			vals,
			texts,
			&selected
		) ;

		if (res)
		{
			_FreeCAMode = selected ;
			_changeFlag = TRUE ;
			return TRUE ;
		}

		return FALSE ;
	}

	return FALSE ;
}

static void SdtServiceHolderDelFn ( SdtServiceHolder **holder )
{
	delete *holder ;
	*holder = NULL ;
}

SdtServiceHolderArray::SdtServiceHolderArray()
{
	setDelFunc(SdtServiceHolderDelFn) ;
}

SDT_Holder::SDT_Holder( ushort tsId, ushort origNtwId )
 : BaseHolder(SDT_HOLDER) 
{ 
	transport_stream_id = tsId ; 
	original_network_id = origNtwId ; 
}

void SDT_Holder::load( PsiData *psiData )
{
	uchar *sdtStart = (uchar*)psiData->_privateSections ;
	int nSections = psiData->_nSections ;

	while (nSections--)
	{
		SDTable *sdt = (SDTable*)sdtStart ;

		if (sdt->table_id!=TABLE_ID_SDT) 
			throw ;
		
		transport_stream_id = sdt->getTransportStreamId() ;
		original_network_id = sdt->getOriginalNetworkId() ;

		int nServices = sdt->getNumServices() ;

		uchar *service = sdt->services ;
		for ( int i = 0; i < nServices; i++ )
		{
			SdtServiceHolder *servHolder = new SdtServiceHolder ;
			service = servHolder->loadFromBin(service) ;
			_services.add(servHolder) ;
		}

		sdtStart += sdt->getTotalLength() ;
	}

	setHolderType(SDT_HOLDER) ;
}

void SDT_Holder::save( PsiData *psiData ) 
{
	uchar buffer[3000] ;
	SDTable *sdt = (SDTable*)buffer ;
	int nSections = 1 ;
	int totalSize = 0 ;

	// find out number of sections and total size of SDT sections
	uchar *pos = sdt->services ;

	int nServices = _services.count() ;
	for ( int i = 0; i < nServices; i++ )
	{
		uchar *endPos = _services[i]->saveToBin(pos) ;
		
		int sectionSize = 4+int(endPos-buffer) ;
		if ( sectionSize > 1021 )
		{
			sectionSize = 4+int(pos-buffer) ;
			nSections++ ;
			totalSize += sectionSize ;

			pos = _services[i]->saveToBin(sdt->services) ;
		}
		else
			pos = endPos ;
	}
	totalSize += 4+int(pos-buffer) ;

	psiData->_privateSections = (PrivateSection*)malloc(totalSize) ;
	psiData->_nSections = nSections ;
	psiData->_pid = TABLE_PID_SDT ;
	psiData->_freq = 2000 ;


	uchar iSection = 0 ;

	sdt = (SDTable*)psiData->_privateSections ;
	sdt->create( transport_stream_id, original_network_id ) ;
	sdt->section_number = iSection++ ;
	sdt->last_section_number = (uchar)(nSections-1) ;
	pos = sdt->services ;

	for ( i = 0; i < nServices; i++ )
	{
		uchar *endPos = _services[i]->saveToBin(pos) ;
		
		int sectionSize = 4+int(endPos-((uchar*)sdt)) ;
		if ( sectionSize > 1021 )
		{
			sectionSize = 4+int(pos-((uchar*)sdt)) ;
			sdt->setSectionLength(sectionSize) ;
			sdt->computeCRC() ;

			sdt = (SDTable*)(pos+4) ;
			sdt->create( transport_stream_id, original_network_id ) ;
			sdt->section_number = iSection++ ;
			sdt->last_section_number = (uchar)nSections ;
			pos = _services[i]->saveToBin(sdt->services) ;
			continue ;
		}

		pos = endPos ;
	}

	int sectionSize = 4+int(pos-((uchar*)sdt)) ;
	sdt->setSectionLength(sectionSize) ;
	sdt->computeCRC() ;
}
/*
void SDT_Holder::load( PsiData *psiData )
{
	SDTable *sdt = (SDTable*)psiData->_privateSections ;

	ASSERT(sdt->table_id==TABLE_ID_SDT) ;
	ASSERT(psiData->_nSections == 1 ) ;

	transport_stream_id = sdt->getTransportStreamId() ;
	original_network_id = sdt->getOriginalNetworkId() ;

	int nServices = sdt->getNumServices() ;

	uchar *service = sdt->services ;
	for ( int i = 0; i < nServices; i++ )
	{
		SdtServiceHolder *servHolder = new SdtServiceHolder ;
		service = servHolder->loadFromBin(service) ;
		_services.add(servHolder) ;
	}

	setHolderType(SDT_HOLDER) ;
}

void SDT_Holder::save( PsiData *psiData ) 
{
	uchar buffer[2000] ;
	SDTable *sdt = (SDTable*)buffer ;

	sdt->create( transport_stream_id, original_network_id ) ;

	uchar *pos = sdt->services ;

	int nServices = _services.count() ;
	for ( int i = 0; i < nServices; i++ )
		pos = _services[i]->saveToBin(pos) ;

	int totalSize = 4+int(pos-buffer) ;
	sdt->setSectionLength(totalSize) ;
	sdt->computeCRC() ;

	psiData->_privateSections = (PrivateSection*)malloc(totalSize) ;
	memcpy(psiData->_privateSections, sdt, totalSize) ;

	psiData->_nSections = 1 ;
	psiData->_pid = TABLE_PID_SDT ;
	psiData->_freq = 25 ;
}
*/

BOOL SDT_Holder::delChild( BaseHolder *child)
{ 
	if (child->getHolderType()==SDT_SERVICE_HOLDER )
	{
		int res = AfxGetMainWnd()->MessageBox(
			"Do you want to delete selected service description ?\n\n"
			"Remember that it is associated\n" 
			"with the program in the PMT table",
			"Question",
			MB_ICONQUESTION | MB_YESNO
		) ;

		if (res==IDYES)
		{
			SdtServiceHolder *servHolder = (SdtServiceHolder*)child ;
			_services.delObj(servHolder) ;
			return TRUE ;
		}
		return FALSE ;
	}

	return FALSE ;
}

void SDT_Holder::deleteService( ushort id )
{
	int i = _services.count() ;
	while ( i-- )
	{
		SdtServiceHolder *serv = _services.item(i) ;
		if (serv->getServiceId()==id)
			_services.del(i) ;
	}
}

int	 SDT_Holder::getItemNumber( )
{
	return 2+_services.count() ;
}

void SDT_Holder::getValueText( int itemIndex, char *buf )
{
	if ( itemIndex<0 || itemIndex>getItemNumber() )
		return ;

	switch (itemIndex)
	{
		case 0:	itoa(transport_stream_id, buf, 10); return ;
		case 1: networkIdAsText(original_network_id,buf); return ;
	} ;

	itemIndex -= 2 ;

	SdtServiceHolder *servHolder = _services.item(itemIndex) ;
	servHolder->getTextDescription(buf);
}

void SDT_Holder::getItemName( int itemIndex, char *buf )
{
	if ( itemIndex<0 || itemIndex>getItemNumber() )
		return ;

	switch (itemIndex)
	{
		case 0:	strcpy(buf, "Transport Stream ID"); return ;
		case 1: strcpy(buf, "Original Network ID"); return ;
	} ;

	itemIndex -= 2 ;

	SdtServiceHolder *servHolder = _services.item(itemIndex) ;
	sprintf(buf, "Service %d", servHolder->_serviceId );
}

BOOL editNetworkId ( int *id, BOOL bOrigId )
{
		DECLARE_EDIT_ENUM(200) ;
		#define EDIT_ENUM_TEXT_ENUMERATOR networkIdAsText

		int num = 0 ;
		for ( int i = 1; i <= 0xFF00; i++ )
		{
			char buf[256] ;
			networkIdAsText(i, buf) ;
			if (buf[0]!='\x0')
			{
				num++ ;
				ADD_ENUM_VALUE(i) ;
			}
		}

		char description[1024] ;

		sprintf(description,
			"Specify the id number of the %snetwork. "
			"Id's are assigned in the DVB standard (ETSI ETR 162). "
			"If your company does not have assigned number "
			"use some from the interval 0xFF00-0xFFFF",
			bOrigId?"original ":""
		) ;

		BOOL res = EditEnumValue(
			TRUE,
			bOrigId?"Original network id":"Network id",
			description,
			num,
			ENUM_VALS(),
			ENUM_TEXTS(),
			id
		) ;

		FREE_ENUM_VALUES() ;
		#undef EDIT_ENUM_TEXT_ENUMERATOR

		return res ;
}

BOOL SDT_Holder::editItem( int itemIndex )
{
	if (itemIndex==0)
	{
		//transport_stream_id
		return FALSE ;
	}

	if (itemIndex==1)
	{
		int id = original_network_id ;

		if (editNetworkId(&id, TRUE))
		{
			original_network_id = id ;
			_changeFlag = TRUE ;
			return TRUE ;
		}
	}
	return FALSE ;
}

//---------------------------//
//		NIT Holder
//---------------------------//

NitTransportStreamHolder::NitTransportStreamHolder(ushort tsId, ushort origNtwId )
 : BaseHolder(NIT_TS_HOLDER) 
{ 
	_transportStreamId=tsId; 
	_originalNetworkId=origNtwId; 
}

uchar *NitTransportStreamHolder::loadFromBin( uchar *buffer )
{
	NitTransportStream *stream = (NitTransportStream*)buffer ;

	_transportStreamId = stream->getTransportStreamId();
	_originalNetworkId = stream->getOriginalNetworkId();

	int descLen = stream->getDescLength() ;
	_descriptors.loadFromBin(stream->descriptors, descLen ) ;

	setHolderType(NIT_TS_HOLDER) ;

	return stream->descriptors + descLen ;
}

uchar *NitTransportStreamHolder::saveToBin( uchar *buffer )
{
	NitTransportStream *stream = (NitTransportStream*)buffer ;

	stream->setTransportStreamId(_transportStreamId);
	stream->setOriginalNetworkId(_originalNetworkId);

	uchar *descEnd = _descriptors.saveToBin(stream->descriptors) ;
	int len = int(descEnd-stream->descriptors) ;
	if (len > 251)
		throw ;

	stream->setDescLength(len) ;

	return descEnd ;
}

BOOL NitTransportStreamHolder::delChild( BaseHolder *child)
{ 
	if (child->isDescHolder() && delDescrQuestion())
	{
		DescriptorHolder *descHolder = (DescriptorHolder*)child ;
		_descriptors.delObj(descHolder) ;
		return TRUE ;
	}
	else
		return FALSE ;

}

int	 NitTransportStreamHolder::getItemNumber( )
{
	return 2+_descriptors.count() ;
}

void NitTransportStreamHolder::getValueText( int itemIndex, char *buf )
{
	if ( itemIndex<0 || itemIndex>getItemNumber() )
		return ;

	switch (itemIndex)
	{
		case 0:	itoa(_transportStreamId, buf, 10); return ;
		case 1: networkIdAsText(_originalNetworkId, buf); return ;
	} ;

	itemIndex -= 2 ;

	DescriptorHolder *descHolder = _descriptors.item(itemIndex) ;
	descHolder->getTextDescription(buf);
}

void NitTransportStreamHolder::getItemName( int itemIndex, char *buf )
{
	if ( itemIndex<0 || itemIndex>getItemNumber() )
		return ;

	switch (itemIndex)
	{
		case 0:	strcpy(buf, "Transport Stream ID"); return ;
		case 1: strcpy(buf, "Original Network ID"); return ;
	} ;

	itemIndex -= 2 ;

	DescriptorHolder *descHolder = _descriptors.item(itemIndex) ;
	strcpy(buf, descriptorName(descHolder->getType()) );
}

BOOL NitTransportStreamHolder::editItem( int itemIndex )
{
	if (itemIndex==0)
	{
		//transport_stream_id
		return FALSE ;
	}

	if (itemIndex==1)
	{
		int id = _originalNetworkId;

		if (editNetworkId(&id, TRUE))
		{
			_originalNetworkId = id ;
			_changeFlag = TRUE ;
			return TRUE ;
		}
	}
	return FALSE ;
}

static void NitTSHolderDelFn( NitTransportStreamHolder **holder )
{
	delete *holder ;
	*holder = NULL ;
}

NitTSHolderArray::NitTSHolderArray( )
{
	setDelFunc(NitTSHolderDelFn) ;
}

NIT_Holder::NIT_Holder( )
 : BaseHolder(NIT_HOLDER) 
{
}

void NIT_Holder::load( PsiData *psiData )
{
	NITable *nit = (NITable*)psiData->_privateSections ;

	ASSERT(nit->table_id==TABLE_ID_NIT) ;
//	ASSERT(psiData->_nSections == 1 ) ;

	_networkId = nit->getNetworkId() ;
	
	_descriptors.loadFromBin( nit->descriptors, nit->getDescLength() ) ;

	int nStreams = nit->getNumStreams() ;

	uchar *stream = nit->transpStreams() ;
	for ( int i = 0; i < nStreams; i++ )
	{
		NitTransportStreamHolder *tsHolder = new NitTransportStreamHolder;
		stream = tsHolder->loadFromBin(stream) ;
		_transpStreams.add(tsHolder) ;
	}

	setHolderType(NIT_HOLDER) ;
}

void NIT_Holder::save( PsiData *psiData )
{
	uchar buffer[3000] ;
	NITable *nit = (NITable*)buffer ;
	nit->create( _networkId ) ;

	uchar *pos = _descriptors.saveToBin( nit->descriptors );
	nit->network_descriptors_length = ntols( (int)(pos-nit->descriptors) ) ;

	uchar *streamStart = (pos +=2) ;

	int nStreams = _transpStreams.count() ;
	for ( int i = 0; i < nStreams; i++ )
		pos = _transpStreams[i]->saveToBin(pos) ;

	nit->setStreamsSize(int(pos-streamStart)) ;
	int nitLen = 4+int(pos-buffer) ;
	if ( nitLen > 1017 )
		throw ;

	nit->setSectionLength(nitLen) ;
	nit->computeCRC() ;

	int totalSize = nit->getTotalLength() ;
	psiData->_privateSections = (PrivateSection*)malloc(totalSize) ;
	memcpy(psiData->_privateSections, nit, totalSize) ;

	psiData->_nSections = 1 ;
	psiData->_pid = TABLE_PID_NIT ;
	psiData->_freq = 2000 ;
}

BOOL NIT_Holder::delChild( BaseHolder *child)
{ 
	if (child->isDescHolder() && delDescrQuestion())
	{
		DescriptorHolder *descHolder = (DescriptorHolder*)child ;
		_descriptors.delObj(descHolder) ;
		return TRUE ;
	}
	else
		return FALSE ;

}

int	 NIT_Holder::getItemNumber( )
{
	return 1+_descriptors.count()+_transpStreams.count() ;
}

void NIT_Holder::getItemName( int itemIndex, char *buf )
{
	if ( itemIndex<0 || itemIndex>getItemNumber() )
		return ;

	if (itemIndex==0)
	{
		strcpy(buf, "Network ID"); return ;
	} 

	itemIndex-- ;

	if (itemIndex < _descriptors.count())
	{
		DescriptorHolder *descHolder = _descriptors.item(itemIndex) ;
		strcpy(buf, descriptorName(descHolder->getType()) );
		return ;
	}

	itemIndex -= _descriptors.count() ;

	NitTransportStreamHolder *tsHolder = _transpStreams.item(itemIndex) ;
	sprintf(buf, "Transport Stream %d", tsHolder->_transportStreamId);
}

void NIT_Holder::getValueText( int itemIndex, char *buf )
{
	if ( itemIndex<0 || itemIndex>getItemNumber() )
		return ;

	if (itemIndex==0)
	{
		networkIdAsText(_networkId,buf); return ;
	} 

	itemIndex-- ;

	if (itemIndex < _descriptors.count())
	{
		DescriptorHolder *descHolder = _descriptors.item(itemIndex) ;
		descHolder->getTextDescription(buf);
		return ;
	}

	itemIndex -= _descriptors.count() ;

	NitTransportStreamHolder *tsHolder = _transpStreams.item(itemIndex) ;
	tsHolder->getTextDescription(buf);
}

BOOL NIT_Holder::editItem( int itemIndex )
{
	if (itemIndex==0)
	{
		int id = _networkId;

		BOOL res = editNetworkId(&id, TRUE) ;

		if (res)
		{
			_networkId = id ;
			_changeFlag = TRUE ;
			return TRUE ;
		}
	}

	return FALSE ;
}

//---------------------------//
//		TableHolder
//---------------------------//

void PsiDataArray::getTableSizes( UINT *patSize, UINT *pmtSize, UINT *catSize, UINT *sdtSize, UINT *nitSize )
{
	*patSize=  *pmtSize=  *catSize=  *sdtSize=  *nitSize = 0 ;
	
	int i = count() ;
	while (i--)
	{
		PsiData *data = &item(i) ;
		int size ;
		data->verifySections( &size) ;

		switch ( data->getDataType() )
		{
		case TABLE_ID_PAT			:
			*patSize += size ;
			break ;
		case TABLE_ID_CAT			:
			*catSize += size ;
			break ;
		case TABLE_ID_PMT			:	
		{
			*pmtSize += size ;
			break ;
		}
		case TABLE_ID_NIT			:		
			*nitSize += size ;
			break ;
		case TABLE_ID_SDT			:
			*sdtSize += size ;
			break ;
		}
	}
}

UINT PsiDataArray::getTotalSpeed( )
{
	UINT patSize=0  ,pmtSize=0  ,catSize=0  ,sdtSize=0  ,nitSize =0 ;
	UINT patFreq=0  ,pmtFreq=0  ,catFreq=0  ,sdtFreq=0  ,nitFreq =0 ;
	
	int i = count() ;
	while (i--)
	{
		PsiData *data = &item(i) ;
		int size ;
		data->verifySections( &size) ;

		switch ( data->getDataType() )
		{
		case TABLE_ID_PAT			:
			patSize += size ;
			patFreq = data->_freq ;
			break ;
		case TABLE_ID_CAT			:
			catSize += size ;
			catFreq = data->_freq ;
			break ;
		case TABLE_ID_PMT			:	
		{
			pmtSize += size ;
			pmtFreq = data->_freq ;
			break ;
		}
		case TABLE_ID_NIT			:		
			nitSize += size ;
			nitFreq = data->_freq ;
			break ;
		case TABLE_ID_SDT			:
			sdtSize += size ;
			sdtFreq = data->_freq ;
			break ;
		}
	}

	UINT patSpeed =	patFreq?(patSize*8/patFreq):0 ; 
	UINT pmtSpeed = pmtFreq?(pmtSize*8/pmtFreq):0 ; 
	UINT catSpeed = catFreq?(catSize*8/catFreq):0 ; 
	UINT sdtSpeed = sdtFreq?(sdtSize*8/sdtFreq):0 ;
	UINT nitSpeed = nitFreq?(nitSize*8/nitFreq):0 ;

	return patSpeed+pmtSpeed+catSpeed+sdtSpeed+nitSpeed ;
}

void TableHolder::LoadFromPsiDataArray( PsiDataArray *arr )
{
	_patFreq = 100;
	_pmtFreq = 100;
	_catFreq = 100;
	_sdtFreq = 1000;
	_nitFreq = 10000 ;

#ifndef LOAD_DUMMY_TABLES
	try 
	{
		int count = arr->count() ;

		for ( int i = 0; i < count; i++ )
		{
			PsiData *data = &arr->item(i) ;
			switch ( data->getDataType() )
			{
			case TABLE_ID_PAT			:
				_patHolder.load(data) ;
				_patFreq = data->_freq ;
				break ;
			case TABLE_ID_CAT			:
				_catHolder.load(data) ;
				_catFreq = data->_freq ;
				break ;
			case TABLE_ID_PMT			:	
			{
				PMT_Holder *pmtHolder = new PMT_Holder ;
				pmtHolder->load(data) ;
				_pmtHolders.add(pmtHolder) ;
				_pmtFreq = data->_freq ;
				break ;
			}
			case TABLE_ID_NIT			:		
				_nitHolder.load(data) ;
				_nitFreq = data->_freq ;
				break ;
			case TABLE_ID_NIT_OTHER_NET	:
				break ;
			case TABLE_ID_SDT			:
				_sdtHolder.load(data) ;
				_sdtFreq = data->_freq ;
				break ;
			case TABLE_ID_SDT_OTHER_TS	:
				break ;
			}
		}
	} catch(...)
	{
		AfxGetMainWnd()->MessageBox("Profile corrupted !","Error", MB_OK|MB_ICONERROR);
	}
#else
	_nitHolder._networkId = 0xFF00 ;
	_nitHolder._transpStreams.add(new NitTransportStreamHolder(1,0xFF00)) ;

	_patHolder._transportStreamId = 1 ;
	_patHolder._nitPid = TABLE_PID_NIT ;
	_pmtHolders.clearList() ;
	_sdtHolder.transport_stream_id = 1 ;
	_sdtHolder.original_network_id = 0xFF00 ;
	_sdtHolder._services.clearList() ;
	{
		PatProgram program(100, 200 );
		_patHolder._patPrograms.add(program) ;
		_pmtHolders.add(new PMT_Holder(100)) ;
		_pmtHolders[0]->_PCR_PID = 210 ;
		_pmtHolders[0]->_streams.add(new PmtStreamHolder(PmtStream::video_MPEG2,210)) ;
		_pmtHolders[0]->_streams.add(new PmtStreamHolder(PmtStream::audio_MPEG2,220)) ;
		
		_sdtHolder._services.add(new SdtServiceHolder(100)) ;

		DescriptorHolder *dh = new DescriptorHolder ;
		ServiceDescriptor *servDesc = (ServiceDescriptor*)dh->getBaseDesc() ;
		servDesc->create(ServiceDescriptor::PAL_coded_signal, "Prov0", "Service100") ;
		_sdtHolder._services[0]->_descriptors.add(dh) ;
	}
	{
		PatProgram program(101, 300 );
		_patHolder._patPrograms.add(program) ;
		_pmtHolders.add(new PMT_Holder(101)) ;
		_pmtHolders[1]->_PCR_PID = 310 ;
		_pmtHolders[1]->_streams.add(new PmtStreamHolder(PmtStream::video_MPEG2,310)) ;
		_pmtHolders[1]->_streams.add(new PmtStreamHolder(PmtStream::audio_MPEG2,320)) ;

		_sdtHolder._services.add(new SdtServiceHolder(101)) ;

		DescriptorHolder *dh = new DescriptorHolder ;
		ServiceDescriptor *servDesc = (ServiceDescriptor*)dh->getBaseDesc() ;
		servDesc->create(ServiceDescriptor::PAL_coded_signal, "Prov1", "Service101") ;
		_sdtHolder._services[1]->_descriptors.add(dh) ;
	}
	{
		PatProgram program(102, 400 );
		_patHolder._patPrograms.add(program) ;
		_pmtHolders.add(new PMT_Holder(102)) ;
		_pmtHolders[2]->_streams.add(new PmtStreamHolder(PmtStream::video_MPEG2,400)) ;

		_sdtHolder._services.add(new SdtServiceHolder(102)) ;

		DescriptorHolder *dh = new DescriptorHolder ;
		ServiceDescriptor *servDesc = (ServiceDescriptor*)dh->getBaseDesc() ;
		servDesc->create(ServiceDescriptor::PAL_coded_signal, "Prov2", "Service102") ;
		_sdtHolder._services[2]->_descriptors.add(dh) ;
	}
#endif
}

#define DISP_SAVE_ERR(table)	sprintf(buf,"Can't save %s section.\nIt is probably too big.", table) ; AfxGetMainWnd()->MessageBox(buf,"Error", MB_OK|MB_ICONERROR);

PsiDataArray *TableHolder::GeneratePsiDataArray( )
{
	PsiDataArray *arr = new PsiDataArray ;
	int nPMT = _pmtHolders.count() ;
	arr->setCount( 4 + nPMT ) ;

	int i = 0 ;
	char buf[512] ;

	try	{ _patHolder.save(&arr->item(i)) ; (*arr)[i++]._freq = _patFreq ; } catch(...) { DISP_SAVE_ERR("PAT") ; }
	try	{ _sdtHolder.save(&arr->item(i)) ; (*arr)[i++]._freq = _sdtFreq ; } catch(...) { DISP_SAVE_ERR("SDT") ; }
	try	{ _nitHolder.save(&arr->item(i)) ; (*arr)[i++]._freq = _nitFreq ; } catch(...) { DISP_SAVE_ERR("NIT") ; }
	try	{ _catHolder.save(&arr->item(i)) ; (*arr)[i++]._freq = _catFreq ; } catch(...) { DISP_SAVE_ERR("CAT") ; }
	
	for ( int j = 0; j < nPMT; j++ )
	{
		PMT_Holder  *pmtHolder  = _pmtHolders[j] ;
		PsiData		*data		= &arr->item(i) ;

		try 
		{ 
			pmtHolder->save( data ) ;
			ushort pid = _patHolder.findPID(pmtHolder->_ProgramNumber) ;
			data->_pid = pid ;
			data->_freq= _pmtFreq ;
			i++;
		} catch(...) { DISP_SAVE_ERR("some PMT") ; }
	}

	arr->setCount( i ) ;

	return arr ;
}

void TableHolder::UpdateSpeed( TableHolder  *holder )
{
	_patFreq = holder->_patFreq ;
	_pmtFreq = holder->_pmtFreq ;
	_sdtFreq = holder->_sdtFreq ;
	_nitFreq = holder->_nitFreq ;
	_catFreq = holder->_catFreq ;
}

void TableHolder::UpdateSpeed( PsiDataArray *arr )
{
	int count = arr->count() ;

	for ( int i = 0; i < count; i++ )
	{
		PsiData *data = &arr->item(i) ;
		switch ( data->getDataType() )
		{
		case TABLE_ID_PAT			:
			data->_freq = _patFreq ;
			break ;
		case TABLE_ID_CAT			:
			data->_freq = _catFreq;
			break ;
		case TABLE_ID_PMT			:	
		{
			data->_freq = _pmtFreq;
			break ;
		}
		case TABLE_ID_NIT			:		
			data->_freq = _nitFreq ;
			break ;
		case TABLE_ID_SDT			:
			data->_freq = _sdtFreq ;
			break ;
		}
	}
}

void TableHolder::GetRunningTablesNames	( char *buf )
{
	strcpy(buf, "[ ") ;
	if (_patFreq>0)	strcat(buf, "PAT ") ;
	if (_pmtFreq>0)	strcat(buf, "PMT ") ;
	if (_sdtFreq>0)	strcat(buf, "SDT ") ;
	if (_nitFreq>0)	strcat(buf, "NIT ") ;
	if (_catFreq>0)	strcat(buf, "CAT ") ;
	strcat(buf, "]") ;
}

BaseHolder* TableHolder::ReplaceSubstHolder( BaseHolder *substHolder, BaseHolder *parent )
{
	if (!substHolder->isSubstHolder())
		return substHolder ;

	int res = AfxGetMainWnd()->MessageBox(
		"Do you want to add a new descriptor ?\n\n"
		"NOTE: Descriptor will be created using DEFAULT values.\n"
		"To change descriptor value you CAN EDIT it by double click",
		"Question",
		MB_ICONQUESTION | MB_YESNO
	) ;

	if (res!=IDYES)
		return substHolder ;

	int type = substHolder->getHolderType() & 0x0FFF ;
	BaseHolder* newHolder = parent->createChild(type) ;

	if (newHolder==NULL)
	{
		AfxGetMainWnd()->MessageBox(
			"Unsuppoted descriptor type",
			"PSI/SI Generator",
			MB_ICONWARNING | MB_OK
		) ;
		return substHolder ;
	}

	_substHolders.delObj(substHolder) ;
	return newHolder ;
}

BaseHolder* TableHolder::CreateSubstHolder( int type )
{
	BaseHolder *holder = new BaseHolder(type, TRUE) ;
	_substHolders.add(holder) ;
	return holder ;
}

void TableHolder::AddProgram( ushort prgNum, ushort pmtPid, PMT_Holder *pmtHolder, SdtServiceHolder *service )
{
	_patHolder.addProgram(prgNum,pmtPid) ;
	_pmtHolders.add(pmtHolder) ;
	_sdtHolder.addService(service) ;
}

BOOL TableHolder::DeleteProgram( int prgNum )
{
	char msg[512] ;
	sprintf( msg,
		"Do you really want to delete program n. %d ?\n\n"
		"This action will ALSO DELETE :\n\n"
		"\t1. program assignment in PAT\n" 
		"\t2. program map table for program %d\n" 
		"\t3. service description for service %d (in SDT)\n\n"
		"Are you sure you want to delete the program ?",
		prgNum, prgNum, prgNum
	) ;

	int res = AfxGetMainWnd()->MessageBox( msg, "Question",	MB_ICONQUESTION | MB_YESNO ) ;
	if (res!=IDYES)
		return FALSE ;

	_sdtHolder.deleteService(prgNum) ;
	_pmtHolders.deleteProgram(prgNum) ;
	_patHolder.deleteProgram(prgNum) ;

	return TRUE ;
}

BOOL PAT_Holder::findPidUse( ushort pid, char *usedBuf )
{
	if (_nitPid==pid)
	{
		strcpy(usedBuf,"PAT - Network Information PID") ;
		return TRUE ;
	}

	int i = _patPrograms.count() ;
	while (i--)
		if (_patPrograms[i].program_map_PID==pid)
		{
			sprintf(usedBuf,"Program %d PMT PID", _patPrograms[i].program_number) ;
			return TRUE ;
		}

	return FALSE ;
}

BOOL PMT_Holder::findPidUse( ushort pid, char *usedBuf )
{
	if (_PCR_PID==pid)
	{
		sprintf(usedBuf,"Program %d PCR PID", _ProgramNumber) ;
		return TRUE ;
	}

	int i = _streams.count() ;
	while (i--)
		if (_streams[i]->_ElementaryPID==pid)
		{
			sprintf(usedBuf,"Program %d stream PID", _ProgramNumber) ;
			return TRUE ;
		}

	return FALSE ;
}

BOOL CAT_Holder::findPidUse( ushort pid, char *usedBuf )
{
	int i = _descriptors.count() ;
	while (i--)
		if (_descriptors[i]->getHolderType()==TAG_CA_descriptor)
		{
			CASystemDescHolder *desc = (CASystemDescHolder*)_descriptors[i] ;
			if (desc->getCaSystemPid()==pid)
			{
				strcpy(usedBuf, "CAT - EMM PID") ;
				return TRUE ;
			}
		}

	return FALSE ;
}

void TableHolder::FindPidUse( ushort pid, char *usedBuf )
{
	usedBuf[0] = '\x0' ;
	if ( _patHolder.findPidUse( pid, usedBuf ) )
		return ;

	int i = _pmtHolders.count() ;
	while (i--)
		if ( _pmtHolders[i]->findPidUse( pid, usedBuf ) )
			return ;

	if ( _catHolder.findPidUse( pid, usedBuf ) )
		return ;
}

BOOL TableHolder::CheckPidUse( UINT pid, char *usedBuf )
{
	char buf[256] ;
	AfxGetMainWnd()->SendMessage(WM_COMMAND, ID_CheckPidUnique|(pid<<16), (long)buf) ;
	if (usedBuf)
		strcpy(usedBuf,buf) ;
	return buf[0] != 0 ;
}

//--------------------------//
//		Holder items
//--------------------------//

struct EnumeratorItem
{
	char*	_presentedText ;
	union Value
	{
		int		_intValue ;
		float	_floatValue ;
	} ;
} ;

struct ItemRec
{
	enum ValueType
	{
		VA_String = 0,
		VA_Integer,
		VA_MinMaxInteger,
		VA_Float,
		VA_MinMaxFloat,
		VA_Enumerate,
	} ;

	char*	_item ;
	char*	_value ;
	int		_valueType ;

	union
	{
		struct
		{
			int min ;
			int max ;
		} _intInterval ;

		struct
		{
			float min ;
			float max ;
		} _floatInterval;

		struct
		{
			int				_nEnumItems ;
			EnumeratorItem*	_enum ;
		} _enumerator ;
	} ;
} ;

//--------------------------//
//		Edit dialogs
//--------------------------//

class EditValueDlg : public CDialog
{
	const char *_item ;
	const char *_description ;
	char _value[512];

protected:
	virtual BOOL OnInitDialog( ) ;
	virtual void OnOK( );

public:
	EditValueDlg( CWnd* parent, const char *item, const char *value, const char *description="" ) ;

	void GetText ( char *buf ) ;
} ;

EditValueDlg::EditValueDlg( CWnd* parent, const char *item, const char *value, const char *description )
 : CDialog(IDD_EditValue, parent)	
{ 
	_item=item ;
	strcpy(_value, value); 
	_description=description ;
}

BOOL EditValueDlg::OnInitDialog( )
{
	GetDlgItem(IDC_ItemName)->SetWindowText(_item) ;
	GetDlgItem(IDC_edtValue)->SetWindowText(_value) ;
	GetDlgItem(IDC_Description)->SetWindowText(_description) ;

	GetDlgItem(IDC_cbValue)->ShowWindow(SW_HIDE) ;
	GetDlgItem(IDC_cbEditValue)->ShowWindow(SW_HIDE) ;

	return CDialog::OnInitDialog( ) ;
}

void EditValueDlg::OnOK( )
{
	GetDlgItem(IDC_edtValue)->GetWindowText(_value, 511) ;
	CDialog::OnOK( ) ;
}

void EditValueDlg::GetText( char *buf )
{
	strcpy(buf, _value) ;
}


class SelectValueDlg : public CDialog
{
	const char *_item ;
	const char *_description ;

	BOOL	_bEditValue ;
	int		_selected ;
	
	int		_nValues ;
	int*	_values ;
	char**	_meanings ;

protected:
	virtual BOOL OnInitDialog( ) ;
	virtual void OnOK( );
	void	SelectItem( int index ) ;

	inline CComboBox*	GetCombo( )				{ return _bEditValue?&m_EditCombo:&m_Combo ; }

	//{{AFX_DATA(PatPmtDlg)
	enum { IDD = IDD_EditValue };
	CComboBox	m_Combo ;
	CComboBox	m_EditCombo ;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PatPmtDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) ;
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(SelectValueDlg)
	afx_msg void OnCBSelchange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	SelectValueDlg( CWnd* parent, BOOL bEditValue, int nValues, int *values, char **meanings, int selected, const char *item, const char *description="" ) ;

	int GetSelected ( )			{ return _selected ; }
} ;

BEGIN_MESSAGE_MAP(SelectValueDlg, CDialog)
	//{{AFX_MSG_MAP(SelectValueDlg)
	ON_CBN_SELCHANGE(IDC_cbEditValue, OnCBSelchange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

SelectValueDlg::SelectValueDlg( CWnd* parent, BOOL bEditValue, int nValues, int *values, char **meanings, int selected, const char *item, const char *description )
 : CDialog(IDD_EditValue, parent)	
{ 
	_bEditValue	= bEditValue ;
	_item		= item ;
	_description= description ;
	_selected	= selected;
	_nValues	= nValues ;
	_values		= values;
	_meanings	= meanings;
}

void SelectValueDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PatPmtDlg)
	DDX_Control(pDX, IDC_cbValue, m_Combo);
	DDX_Control(pDX, IDC_cbEditValue, m_EditCombo);
	//}}AFX_DATA_MAP
}

BOOL SelectValueDlg::OnInitDialog( )
{
	GetDlgItem(IDC_ItemName)->SetWindowText(_item) ;
	GetDlgItem(IDC_Description)->SetWindowText(_description) ;

	GetDlgItem(IDC_edtValue)->ShowWindow(SW_HIDE) ;
	GetDlgItem(_bEditValue?IDC_cbValue:IDC_cbEditValue)->ShowWindow(SW_HIDE) ;

	if (!CDialog::OnInitDialog( ))
		return FALSE ;

	CComboBox *combo = GetCombo() ;

	for ( int i = 0; i < _nValues; i++ )
	{
		int index = combo->AddString(_meanings[i]) ;
		combo->SetItemData(index, _values[i]) ;

		if (_values[i]==_selected)
			SelectItem(index) ;
	}

	return TRUE ;
}

BOOL SelectValueDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam==ID_RefreshValueList)
	{
		CComboBox *combo = GetCombo() ;
		char *txt = (char*)lParam ;
		combo->SetWindowText(txt) ;
		free(txt) ;
		return TRUE ;
	}

	return CDialog::OnCommand(wParam, lParam) ;
}

void SelectValueDlg::OnOK( )
{
	CComboBox *combo = GetCombo() ;
	if (!_bEditValue)
	{
		int index = combo->GetCurSel() ;
		_selected = (int)combo->GetItemData(index) ;
	}
	else
	{
		char buf[256], *end ;
		combo->GetWindowText(buf,255) ;
		_selected = strtol(buf, &end, 16) ;

		if (end!=buf+strlen(buf))
		{
			MessageBox("Please enter hexadecimal integer value (e.g. 0x5FE1)", "Error", MB_OK|MB_ICONERROR) ;
			combo->SetFocus() ;
			return;
		}
	}

	CDialog::OnOK( ) ;
}

void SelectValueDlg::SelectItem( int index )
{
	CComboBox *combo = GetCombo() ;
	if (!_bEditValue)
		combo->SetCurSel(index) ;
	else
	{
		int value = combo->GetItemData(index) ;
		char c[30] ;
		sprintf(c, "0x%x", value);
		PostMessage(WM_COMMAND, ID_RefreshValueList, (LPARAM)strdup(c)) ;
	}

}

void SelectValueDlg::OnCBSelchange()
{
	if (_bEditValue)
	{
		CComboBox *combo = GetCombo() ;
		int sel = combo->GetCurSel() ;
		if (sel==CB_ERR)
			return ;

		SelectItem(sel) ;
	}
}

//--------------------------//
//		Edit Functions
//--------------------------//

BOOL EditIntValue( BOOL bHexVal, const char *item, const char *description, int *value, int min, int max, CWnd *parent )
{
	char buf[50] ;

	if (bHexVal)
	{
		sprintf(buf, "0x%x", *value) ;
		strupr(buf+2);
	}
	else
		itoa(*value, buf, 10) ;

	if (parent==NULL)
		parent = AfxGetMainWnd() ;

	EditValueDlg dlg(parent, item, buf, description) ;

	while (1)
	{
		int res = dlg.DoModal() ;
		
		if (res != IDOK)
			return FALSE ;

		dlg.GetText(buf) ;

		char *endSt ;
		int newVal = strtol(buf, &endSt, bHexVal?16:10) ;

		if (*endSt != '\x0')
		{
			parent->MessageBox(
				"Illegal value.\n"
				"Please enter integer value",
				"Error",
				MB_OK | MB_ICONERROR ) ;
			continue ;
		}

		if (newVal ==  *value)
			return FALSE ;

		if (newVal>=min && newVal<=max)
		{
			*value = newVal ;
			return TRUE ;
		}

		char msg[128] ;
		sprintf(msg, 
			"Illegal value.\n"
			"Must be within %d and %d.",
			min, max
		) ;

		parent->MessageBox(
			msg,
			"Error",
			MB_OK | MB_ICONERROR ) ;
	}

	return TRUE ;
}

BOOL EditFloatValue( const char *item, const char *description, double *value, double min , double max, CWnd *parent )
{
	char buf[50] ;
	sprintf(buf,"%f",*value) ;

	if (parent==NULL)
		parent = AfxGetMainWnd() ;

	EditValueDlg dlg(parent, item, buf, description) ;

	while (1)
	{
		int res = dlg.DoModal() ;
		
		if (res != IDOK)
			return FALSE ;

		dlg.GetText(buf) ;

		char *endSt ;
		double newVal = strtod(buf, &endSt) ;

		if (*endSt != '\x0')
		{
			parent->MessageBox(
				"Illegal value.\n"
				"Please enter numeric value",
				"Error",
				MB_OK | MB_ICONERROR ) ;
			continue ;
		}

		if (newVal == *value)
			return FALSE ;

		if (newVal>=min && newVal<=max)
		{
			*value = newVal ;
			return TRUE ;
		}

		char msg[128] ;
		sprintf(msg, 
			"Illegal value.\n"
			"Must be within %f and %f.",
			min, max
		) ;

		parent->MessageBox(
			msg,
			"Error",
			MB_OK | MB_ICONERROR ) ;
	}

	return TRUE ;
}

BOOL EditStringValue( const char *item, const char *description, char *value, CWnd *parent )
{
	char buf[256] ;
	strcpy(buf, value) ;

	if (parent==NULL)
		parent = AfxGetMainWnd() ;

	EditValueDlg dlg(parent, item, buf, description) ;

	int res = dlg.DoModal() ;
	
	if (res != IDOK)
		return FALSE ;

	dlg.GetText(buf) ;

	if (strcmp(buf,value)==0)
		return FALSE ;

	strcpy(value,buf) ;
	return TRUE ;
}

static char HexDigitToChar ( uchar digit )
{
	digit &= 0x0F ;
	return (digit<=9)? (digit+'0') : (digit+'A'-10) ;
}

static uchar CharToHexDigit ( char c )
{
	if ( c <= '9' && c >='0' )
		return c-'0' ;
	if ( c <= 'F' && c >='A' )
		return c-'A'+10 ;
	if ( c <= 'f' && c >='a' )
		return c-'a'+10 ;
	return 0xFF ;
}

void HexStringToString( const uchar *data, int nBytes, char *buf )
{
	for ( int i = 0; i < nBytes; i++ )
	{
		buf[2*i]   = HexDigitToChar(data[i]>>4) ;
		buf[2*i+1] = HexDigitToChar(data[i]&0x0F) ;
	}
	buf[2*nBytes] = '\x0' ;
}

BOOL StringToHexString( const char *buf, uchar *data, int &nBytes )
{
	int i = 0 ;
	nBytes = 0 ;
	while ( buf[i] )
	{
		uchar c1 = CharToHexDigit(buf[i++]);
		if ( buf[i]=='\x0' || c1>0x0F)
			return FALSE ;
		uchar c2 = CharToHexDigit(buf[i++]);
		if ( c2>0x0F)
			return FALSE ;

		data[nBytes++] = (c1<<4) | c2 ;
	}

	return TRUE ;
}

BOOL EditHexStringValue( const char *item, const char *description, int &nBytes, uchar *data, CWnd *parent )
{
	char buf[256] ;

	if (parent==NULL)
		parent = AfxGetMainWnd() ;

	HexStringToString(data, nBytes, buf) ;

	EditValueDlg dlg(parent, item, buf, description) ;

	while (1)
	{
		int res = dlg.DoModal() ;
		
		if (res != IDOK)
			return FALSE ;

		dlg.GetText(buf) ;

		if ( StringToHexString(buf, data, nBytes) )
			return TRUE ;

		AfxGetMainWnd()->MessageBox(
			"Please enter valid hex-string.\n"
			"Non-hex characters and odd length is prohibited",
			"Error", 
			MB_OK | MB_ICONERROR
		) ;
	}

	return FALSE ;
}

BOOL EditEnumValue( BOOL bEditValue, const char *item, const char *description, int nValues, int *values, char **meanings, int *selected, CWnd *parent )
{
	if (parent==NULL)
		parent = AfxGetMainWnd() ;

	SelectValueDlg dlg( parent, bEditValue, nValues, values, meanings, *selected, item, description ) ;

	int res = dlg.DoModal() ;
	
	if (res != IDOK)
		return FALSE ;

	int  sel = dlg.GetSelected() ;
	if (*selected != sel)
	{
		*selected = sel ;
		return TRUE ;
	}

	return FALSE ;
}

//--------------------------//
//		PSI Data
//--------------------------//

static void PsiDataDelFn( PsiData *p )
{
	p->clear() ;
}

BOOL PsiData::verifySections( int *sectionSize )
{
	PrivateSection *sect = _privateSections ;
	int p = _nSections ;
	int size = 0 ;
	while (p--)
	{
		if (
			!( sect->table_id==TABLE_ID_PAT
			|| sect->table_id==TABLE_ID_CAT			
			|| sect->table_id==TABLE_ID_PMT			
			|| sect->table_id==TABLE_ID_NIT			
			|| sect->table_id==TABLE_ID_NIT_OTHER_NET	
			|| sect->table_id==TABLE_ID_SDT			
			|| sect->table_id==TABLE_ID_SDT_OTHER_TS	
			|| sect->table_id==TABLE_ID_BAT			
			|| sect->table_id==TABLE_ID_EIT			
			|| sect->table_id==TABLE_ID_EIT_OTHER_TS	
			|| sect->table_id==TABLE_ID_TDT			
			|| sect->table_id==TABLE_ID_RST			
			|| sect->table_id==TABLE_ID_ST				
			|| sect->table_id==TABLE_ID_TOT			
			|| sect->table_id==TABLE_ID_DIT			
			|| sect->table_id==TABLE_ID_SIT	)
		)
			return FALSE ;

		int sectSize = sect->getTotalLength() ;
		size += sectSize ;
		sect = (PrivateSection*)( ((uchar*)sect) + sectSize ) ;
	}

	if (sectionSize)
		*sectionSize = size ;

	return TRUE ;
}

PsiDataArray::PsiDataArray()
{
	setDelFunc(PsiDataDelFn) ;
}

BOOL PsiDataArray::loadFromFile( const char *fileName )
{
	uchar *buffer = NULL ;
	DWORD fileSize ;

	try
	{
		HANDLE handle = CreateFile( fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL ) ;
		if (handle==INVALID_HANDLE_VALUE)
			return FALSE ;

		fileSize = GetFileSize(handle, NULL) ;

		buffer = (uchar*)malloc(fileSize) ;
		if (buffer==NULL)
			return FALSE ;

		DWORD nBytes = 0 ;
		ReadFile(handle, buffer, fileSize, &nBytes, NULL) ;
		if (nBytes != fileSize)
		{
			free(buffer) ;
			return FALSE ;
		}

		CloseHandle(handle) ;

		if (strncmp((const char*)buffer,"PSI/SI", 6 )!=0)
		{
			free(buffer) ;
			return FALSE ;
		}

		uchar *p = buffer+6 ;
		int nRecords = *(int*)p ;
		p += 4 ;

		setCount(nRecords) ;

		for ( int i = 0; i < nRecords; i++ )
		{
			PsiData *psiData = &item(i) ;
			memcpy(psiData, p, 10) ;
			p += 10 ;

			int sectionSize = 0 ;
			BOOL bSectOK = FALSE ;
			
			psiData->_privateSections = (PrivateSection*)p ;
			bSectOK = psiData->verifySections(&sectionSize) ;
			psiData->_privateSections = NULL ;

			if ( !bSectOK  || (p+sectionSize > buffer+fileSize) )
			{
				if (i > 0)
					setCount(i-1) ;

				clearList() ;
				free(buffer) ;
				return FALSE ;
			}

			psiData->_privateSections = (PrivateSection*)malloc(sectionSize) ;
			memcpy(psiData->_privateSections, p, sectionSize) ;
			p += sectionSize ;
		}

	} catch(...)
	{
		if (buffer)
			free(buffer) ;
		return FALSE ;
	}

	free(buffer) ;

	return TRUE ;
}

BOOL PsiDataArray::saveToFile( const char *fileName )
{
	HANDLE handle = CreateFile( 
		fileName, 
		GENERIC_WRITE, 
		0, NULL, 
		CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL 
	) ;

	if ( handle==INVALID_HANDLE_VALUE )
		return FALSE ;

	DWORD written ;
	char signature[10] ;
	strcpy(signature, "PSI/SI") ;
	int nRecords = count() ;
	*(int*)(signature+6) = nRecords ;
	
	WriteFile( handle, signature, 10, &written, NULL );
	ASSERT(written==10) ;

	for ( int i = 0; i < nRecords; i++ )
	{
		PsiData *psiData = &item(i) ;

		WriteFile( handle, psiData, 10, &written, NULL );
		ASSERT(written==10) ;
		
		int sectSize = 0 ;
		psiData->verifySections(&sectSize) ;

		WriteFile( handle, psiData->_privateSections, sectSize, &written, NULL );
		ASSERT((int)written==sectSize) ;
	}

	CloseHandle(handle) ;

	return TRUE ;
}


/*
// Utils

void *___sTemplateArrayAlloc( void *_first, int n, int sizeofT, int &siz, int &bufSiz )
{
    //#define     utilMAXREALLOC 20
    int  utilMAXREALLOC ;
	if( bufSiz < 100 )
		utilMAXREALLOC = 20 ;
	else
	if( bufSiz < 500 )
		utilMAXREALLOC = 100 ;
	else
	if( bufSiz < 1000 )
		utilMAXREALLOC = 300 ;
	else
		utilMAXREALLOC = 500 ;

    void  *t=_first; 
    if( bufSiz < siz+n )
    {
        bufSiz += (n/utilMAXREALLOC)*utilMAXREALLOC + utilMAXREALLOC;
       _first   = realloc( _first, bufSiz*sizeofT ) ;
    }
    if( _first != NULL )
    {
        memset( ((char*)_first)+(siz*sizeofT), 0, n*sizeofT );
        siz += n;
        return _first;
    }
    _first = t;
    bufSiz = 0;
    return NULL;
}
*/