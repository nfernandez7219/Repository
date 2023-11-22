#ifndef __BASECFG_HPP__
#define __BASECFG_HPP__


//  (Exc) indicates Msg exception thrown on error.


//-----------------------------------------------------------------------------------
//	CfgBaseSetup
//  Base class for access to the user config files.
//-----------------------------------------------------------------------------------


class CfgBaseSetup
{
  protected:
	ConfigClass	   *config;
	char		   *fileName;
	
	void			openConfig();

  public:
	void	 set	( const char *sect, const char *var, const char *value )	{ config->set( sect, var, value  ) ; }
	char	*get	( const char *sect, const char *var )						{ return config->get( sect, var ) ; }
	void	 get	( const char *sect, const char *var, char *buf, int bufSize); 

	void	 setInt	( const char *sect, const char *var, int  val )				{ config->setInt( sect, var, val ) ; }
	void	 getInt	( const char *sect, const char *var, int *val )				{ config->getInt( sect, var, val ) ; }

	inline ConfigClass	*cfg()		{ 	if( config == NULL ) config = new ConfigClass( fileName ); return config; }
	
	virtual void storage( CfgBaseSetup *cfg, BOOL saveFlag, LPTSTR lpErrBuffer=NULL )=0;							//Exc
	void	save   ( LPTSTR lpErrBuffer=NULL );		//Exc
	void	load   ( LPTSTR lpErrBuffer=NULL );		//Exc
	void	load   ( BaseConfigClass *pCfg, LPTSTR lpErrBuff=NULL );		//Exc

	CfgBaseSetup( const char *file );
    virtual ~CfgBaseSetup();
};

void copyWithoutSpace ( char *dest, const char *src, int size ) ;

//-----------------------------------------------------------------------------------
//  CfgContext
//  Stand alone config file storing program specific information between consecutive runs.
//-----------------------------------------------------------------------------------


class CfgContext : public CfgBaseSetup
{
	void storage( CfgBaseSetup *cfg, BOOL saveFlag, LPTSTR lpErrBuffer=NULL );	//Exc

  public:
	CfgContext( const char *fileName = "Config\\Context.cfg" ) : CfgBaseSetup( fileName ) {};
};

CfgContext	  *MfxContext();


#endif