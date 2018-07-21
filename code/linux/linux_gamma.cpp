/*
** LINUX_GAMMA.C
*/

#include "../server/exe_headers.h"



#include <assert.h>
#include "../renderer/tr_local.h"
#include "../qcommon/qcommon.h"
#include "linux_local.h"

#define HIBYTE(W)       (unsigned char)(((W) >> 8) & 0xFF)

static x11drv_gamma_ramp s_oldHardwareGamma;

/*
** WG_CheckHardwareGamma
**
** Determines if the underlying hardware supports gamma correction API.
*/

void WG_CheckHardwareGamma( void )
{
#ifdef PANDORA
	glConfig.deviceSupportsGamma = qtrue;
#else
	glConfig.deviceSupportsGamma = qfalse;

	if ( !r_ignorehwgamma->integer )
	{
		glConfig.deviceSupportsGamma = X11DRV_XF86VM_GetGammaRamp(&s_oldHardwareGamma );

		if ( glConfig.deviceSupportsGamma )
		{
			//
			// do a sanity check on the gamma values
			//
			if ( ( HIBYTE( s_oldHardwareGamma.red[255] ) <= HIBYTE( s_oldHardwareGamma.red[0] ) ) ||
				 ( HIBYTE( s_oldHardwareGamma.green[255] ) <= HIBYTE( s_oldHardwareGamma.green[0] ) ) ||
				 ( HIBYTE( s_oldHardwareGamma.blue[255] ) <= HIBYTE( s_oldHardwareGamma.blue[0] ) ) )
			{
				glConfig.deviceSupportsGamma = qfalse;
				ri.Printf( PRINT_WARNING, "WARNING: device has broken gamma support, generated gamma.dat\n" );
			}

			//
			// make sure that we didn't have a prior crash in the game, and if so we need to
			// restore the gamma values to at least a linear value
			//
			if ( ( HIBYTE( s_oldHardwareGamma.red[181] ) == 255 ) )
			{
				int g;

				ri.Printf( PRINT_WARNING, "WARNING: suspicious gamma tables, using linear ramp for restoration\n" );

				for ( g = 0; g < 255; g++ )
				{
					s_oldHardwareGamma.red[g] = g << 8;
					s_oldHardwareGamma.green[g] = g << 8;
					s_oldHardwareGamma.blue[g] = g << 8;
				}
			}
		}
	}
#endif
}

/*
** GLimp_SetGamma
**
** This routine should only be called if glConfig.deviceSupportsGamma is TRUE
*/
void GLimp_SetGamma( unsigned char red[256], unsigned char green[256], unsigned char blue[256] ) {
	x11drv_gamma_ramp table;
	int		i, j;
	int		ret;


	if ( !glConfig.deviceSupportsGamma || r_ignorehwgamma->integer) {
		return;
	}

//mapGammaMax();

#ifdef PANDORA
    double new_gamma;
    char tmp_gamma[51];
    if(red[128])
        new_gamma = log(0.5)/log((( ( ( unsigned short ) red[128] ) << 8 ) | red[128])/65535.0);
    else
        new_gamma = (double)0.0;
    if(new_gamma!=(double)0.0)
        snprintf(tmp_gamma, 50,"sudo /usr/pandora/scripts/op_gamma.sh %.2f", new_gamma);
    else
        snprintf(tmp_gamma, 50,"sudo /usr/pandora/scripts/op_gamma.sh 0");
    system(tmp_gamma);
#else

	for ( i = 0; i < 256; i++ ) {
		table.red[i] = ( ( ( unsigned short ) red[i] ) << 8 ) | red[i];
		table.green[i] = ( ( ( unsigned short ) green[i] ) << 8 ) | green[i];
		table.blue[i] = ( ( ( unsigned short ) blue[i] ) << 8 ) | blue[i];
	}



	// enforce constantly increasing

	for ( i = 1 ; i < 256 ; i++ ) {
		if ( table.red[i] < table.red[i-1] ) {
			table.red[i] = table.red[i-1];
		}
		if ( table.green[i] < table.green[i-1] ) {
			table.green[i] = table.green[i-1];
		}
		if ( table.blue[i] < table.blue[i-1] ) {
			table.blue[i] = table.blue[i-1];
		}
	}



	ret = X11DRV_XF86VM_SetGammaRamp(&table);
	if ( !ret ) {
		Com_Printf( "SetDeviceGammaRamp failed.\n" );
	}
#endif
}

/*
** WG_RestoreGamma
*/
void WG_RestoreGamma( void )
{
#ifdef PANDORA
	system("sudo /usr/pandora/scripts/op_gamma.sh 0");
#else
	if ( glConfig.deviceSupportsGamma )
	{
		X11DRV_XF86VM_SetGammaRamp(&s_oldHardwareGamma );
	}
#endif
}

