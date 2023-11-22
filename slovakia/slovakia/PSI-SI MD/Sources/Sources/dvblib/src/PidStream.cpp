
class PidStreamAttrib
{
	ushort	_pid ;
	ushort	_convertedPid ;
	uchar	_continuityCounter ;
	int		_nReferences ;

  public:
	PidStreamAttrib( ushort pid ) ;

	inline uchar	getContinuityCounter ()	{ return (_continuityCounter++)&0x0F; }
	inline ushort	getConvertedPid		 () { return _convertedPid ; }
} ;

class PidStreamAttribManager
{
	sTemplateArray<PidStreamAttrib*>	_streamAttribs ;

  public:
	PidStreamAttrib  *getPidStreamAttrib		( ushort pid ) ;
	void			  releasePidStreamAttrib	( PidStreamAttrib *streamAttrib ) ;

	~PidStreamAttribManager() ;
} ;

PidStreamAttrib::PidStreamAttrib( ushort pid )
{
	_pid=pid&0x1FFF;
	_convertedPid = _pid>>8 | _pid<<8 ;
	_continuityCounter=0; 
	_nReferences=1; 
}

PidStreamAttribManager::~PidStreamAttribManager()
{
	int nStreamAttribs = _streamAttribs.count() ;
	if (nStreamAttribs)
		TRACE("PidStreamAttribManager error: %d unreleased PidStreamAttribs.", nStreamAttribs);
}

PidStreamAttrib *PidStreamAttribManager::getPidStreamAttrib( ushort pid )
{
	PidStreamAttrib *streamAttrib ;
	int i = _streamAttrib.count() ;
	while(i--)
	{
		streamAttrib = _streamAttrib[i];
		if (streamAttrib->_pid==pid)
		{
			InterlockedIncrement(&streamAttrib->_nReferences);
			return streamAttrib ;
		}
	}

	streamAttrib = new PidStreamAttrib(pid) ;
	_streamAttribs.add(streamAttrib) ;
	return streamAttrib ;
}

void PidStreamAttribManager::releasePidStreamAttrib ( PidStreamAttrib *streamAttrib )
{
	long nRef = InterlockedDecrement(&streamAttrib->_nReferences) ;
	if (nRef==0)
	{
		_streamAttribs.delObj(streamAttrib) ;
		delete streamAttrib ;
	}
}
