/*
 * Copyright (C) 2010 Reinhard Prix
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with with program; see the file COPYING. If not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 */

/**
 * \file
 * \ingroup lalapps_pulsar_Tools
 * \author Reinhard Prix
 * \brief
 * Simple standalone code to provide ASCII output for detector response
 * and ephemeris state at given time, for given detector and sky-location
 *
 */

/* ---------- includes ---------- */
#include <math.h>
#include <errno.h>
#include <string.h>

#include <lal/UserInput.h>
#include <lal/LALInitBarycenter.h>
#include <lal/ComputeFstat.h>
#include <lal/LALString.h>

#include <lalapps.h>

/* ----- compile switches ----- */

/*---------- local defines ---------- */

/* ----- Macros ----- */

/* ---------- local types ---------- */

typedef struct
{
  LIGOTimeGPS timeGPS;			/**< GPS time to compute detector state for (LIGOtimeGPS format) */
  SkyPosition skypos;			/**< skyposition Alpha,Delta in radians, equatorial coords. */
  EphemerisData *edat;			/**< ephemeris data (from LALInitBarycenter()) */
  LALDetector *det;			/**< LIGODetector struct holding detector info */
  LIGOTimeGPSVector *timestamps;	/**< timestamps vector holding 1 element: timeGPS */
  REAL8 sinzeta;			/**< detector-arm angle correction (needed for GEO) */
} ConfigVariables;


typedef struct
{
  CHAR *detector;	/**< detector name */

  REAL8 Alpha;		/**< skyposition Alpha: radians, equatorial coords. */
  REAL8 Delta;		/**< skyposition Delta: radians, equatorial coords. */

  CHAR *ephemEarth;	/**< Earth ephemeris file to use */
  CHAR *ephemSun;	/**< Sun ephemeris file to use */

  REAL8 timeGPS;	/**< GPS time to compute detector state for (REAL8 format) */

  BOOLEAN version;	/**< output code versions */

} UserVariables_t;

/* ---------- global variables ----------*/
extern int vrbflg;

/* ---------- local prototypes ---------- */
int XLALInitUserVars ( UserVariables_t *uvar );
int XLALInitCode ( ConfigVariables *cfg, const UserVariables_t *uvar, const char *app_name);

int XLALDestroyConfig ( ConfigVariables *cfg );

/*============================================================
 * FUNCTION definitions
 *============================================================*/

int
main(int argc, char *argv[])
{
  LALStatus status = blank_status;

  ConfigVariables XLAL_INIT_DECL(config);
  UserVariables_t XLAL_INIT_DECL(uvar);

  /* register user-variables */

  XLAL_CHECK ( XLALInitUserVars ( &uvar ) == XLAL_SUCCESS, XLAL_EFUNC );

  /* read cmdline & cfgfile  */
  BOOLEAN should_exit = 0;
  XLAL_CHECK( XLALUserVarReadAllInput( &should_exit, argc, argv ) == XLAL_SUCCESS, XLAL_EFUNC );
  if ( should_exit ) {
    exit(1);
  }

  if ( uvar.version )
    {
      XLALOutputVersionString ( stdout, lalDebugLevel );
      exit(0);
    }

  /* basic setup and initializations */
  XLAL_CHECK ( XLALInitCode( &config, &uvar, argv[0] ) == XLAL_SUCCESS, XLAL_EFUNC );

  /* ----- allocate memory for AM-coeffs ----- */
  AMCoeffs AMold, AMnew1, AMnew2;	/**< containers holding AM-coefs computed by 3 different AM functions */
  AMold.a = XLALCreateREAL4Vector ( 1 );
  AMold.b = XLALCreateREAL4Vector ( 1 );
  AMnew1.a = XLALCreateREAL4Vector ( 1 );
  AMnew1.b = XLALCreateREAL4Vector ( 1 );
  AMnew2.a = XLALCreateREAL4Vector ( 1 );
  AMnew2.b = XLALCreateREAL4Vector ( 1 );

  XLAL_CHECK ( AMold.a && AMold.b && AMnew1.a && AMnew1.b && AMnew2.a && AMnew2.a, XLAL_ENOMEM, "Failed to XLALCreateREAL4Vector ( 1 )\n" );

  /* ----- get detector-state series ----- */
  DetectorStateSeries *detStates = NULL;
  XLAL_CHECK ( (detStates = XLALGetDetectorStates ( config.timestamps, config.det, config.edat, 0 )) != NULL, XLAL_EFUNC );

  /* ----- compute associated SSB timing info ----- */
  SSBtimes *tSSB = XLALGetSSBtimes ( detStates, config.skypos, config.timeGPS, SSBPREC_RELATIVISTIC );
  XLAL_CHECK ( tSSB != NULL, XLAL_EFUNC, "XLALGetSSBtimes() failed with xlalErrno = %d\n", xlalErrno );

  /* ===== 1) compute AM-coeffs the 'old way': [used in CFSv1] ===== */
  BarycenterInput XLAL_INIT_DECL(baryinput);
  AMCoeffsParams XLAL_INIT_DECL(amParams);
  EarthState earth;

  baryinput.site.location[0] = config.det->location[0]/LAL_C_SI;
  baryinput.site.location[1] = config.det->location[1]/LAL_C_SI;
  baryinput.site.location[2] = config.det->location[2]/LAL_C_SI;
  baryinput.alpha = config.skypos.longitude;
  baryinput.delta = config.skypos.latitude;
  baryinput.dInv = 0.e0;

  /* amParams structure to compute a(t) and b(t) */
  amParams.das = XLALMalloc(sizeof(*amParams.das));
  amParams.das->pSource = XLALMalloc(sizeof(*amParams.das->pSource));
  amParams.baryinput = &baryinput;
  amParams.earth = &earth;
  amParams.edat = config.edat;
  amParams.das->pDetector = config.det;
  amParams.das->pSource->equatorialCoords.longitude = config.skypos.longitude;
  amParams.das->pSource->equatorialCoords.latitude = config.skypos.latitude;
  amParams.das->pSource->orientation = 0.0;
  amParams.das->pSource->equatorialCoords.system = COORDINATESYSTEM_EQUATORIAL;
  amParams.polAngle = 0;

  LAL_CALL ( LALComputeAM ( &status, &AMold, config.timestamps->data, &amParams), &status);

  XLALFree ( amParams.das->pSource );
  XLALFree ( amParams.das );


  /* ===== 2) compute AM-coeffs the 'new way' using LALNewGetAMCoeffs() */
  LALGetAMCoeffs ( &status, &AMnew1, detStates, config.skypos );
  if ( status.statusCode ) {
    XLALPrintError ("%s: call to LALGetAMCoeffs() failed, status = %d\n\n", __func__, status.statusCode );
    XLAL_ERROR (  status.statusCode & XLAL_EFUNC );
  }

  /* ===== 3) compute AM-coeffs the 'newer way' using LALNewGetAMCoeffs() [used in CFSv2] */
  LALNewGetAMCoeffs ( &status, &AMnew2, detStates, config.skypos );
  if ( status.statusCode ) {
    XLALPrintError ("%s: call to LALNewGetAMCoeffs() failed, status = %d\n\n", __func__, status.statusCode );
    XLAL_ERROR (  status.statusCode & XLAL_EFUNC );
  }

  /* ===== 4) use standalone version of the above [used in FstatMetric_v2] */
  REAL8 a0,b0;
  if ( XLALComputeAntennaPatternCoeffs ( &a0, &b0, &config.skypos, &config.timeGPS, config.det, config.edat ) != XLAL_SUCCESS ) {
    XLALPrintError ("%s: XLALComputeAntennaPatternCoeffs() failed.\n", __func__ );
    XLAL_ERROR ( XLAL_EFUNC );
  }


  /* ==================== output the results ==================== */
  printf ("\n");
  printf ("----- Input parameters:\n");
  printf ("tGPS = { %d, %d }\n", config.timeGPS.gpsSeconds, config.timeGPS.gpsNanoSeconds );
  printf ("Detector = %s\n", config.det->frDetector.name );
  printf ("Sky position: longitude = %g rad, latitude = %g rad [equatorial coordinates]\n", config.skypos.longitude, config.skypos.latitude );
  printf ("\n");

  printf ("----- Antenna pattern functions (a,b):\n");
  printf ("LALComputeAM:                    ( %-12.8g, %-12.8g)  [REAL4]\n", AMold.a->data[0], AMold.b->data[0] );
  printf ("LALGetAMCoeffs:                  ( %-12.8g, %-12.8g)  [REAL4]\n", AMnew1.a->data[0], AMnew1.b->data[0] );
  printf ("LALNewGetAMCoeffs:               ( %-12.8g, %-12.8g)  [REAL4]\n", AMnew2.a->data[0]/config.sinzeta, AMnew2.b->data[0]/config.sinzeta );
  printf ("XLALComputeAntennaPatternCoeffs: ( %-12.8g, %-12.8g)  [REAL8]\n", a0/config.sinzeta, b0/config.sinzeta );
  printf ("\n");

  printf ("----- Detector & Earth state:\n");
  REAL8 *pos = detStates->data[0].rDetector;
  printf ("Detector position [ICRS J2000. Units=sec]: rDet = {%g, %g, %g}\n", pos[0], pos[1], pos[2] );
  REAL8 *vel = detStates->data[0].vDetector;
  printf ("Detector velocity [ICRS J2000. Units=c]:   vDet = {%g, %g, %g}\n", vel[0], vel[1], vel[2] );
  printf ("Local mean sideral time: LMST = %g rad\n", detStates->data[0].LMST);
  printf ("\n");
  printf ("----- SSB timing data:\n");
  printf ("TOA difference tSSB - tDet = %g s\n", tSSB->DeltaT->data[0] );
  printf ("TOA rate of change dtSSB/dtDet - 1 = %g\n", tSSB->Tdot->data[0] - 1.0 );
  printf ("\n\n");


  /* ----- done: free all memory */
  XLAL_CHECK ( XLALDestroyConfig( &config ) == XLAL_SUCCESS, XLAL_EFUNC );

  XLALDestroyDetectorStateSeries ( detStates );

  XLALDestroyREAL4Vector ( AMold.a );
  XLALDestroyREAL4Vector ( AMold.b );
  XLALDestroyREAL4Vector ( AMnew1.a );
  XLALDestroyREAL4Vector ( AMnew1.b );
  XLALDestroyREAL4Vector ( AMnew2.a );
  XLALDestroyREAL4Vector ( AMnew2.b );

  XLALDestroyREAL8Vector ( tSSB->DeltaT );
  XLALDestroyREAL8Vector ( tSSB->Tdot );
  XLALFree (tSSB);

  LALCheckMemoryLeaks();

  return 0;
} /* main */


/** register all "user-variables" */
int
XLALInitUserVars ( UserVariables_t *uvar )
{
  XLAL_CHECK ( uvar != NULL, XLAL_EINVAL );

  /* set a few defaults */
  uvar->ephemEarth = XLALStringDuplicate("earth00-19-DE405.dat.gz");
  uvar->ephemSun = XLALStringDuplicate("sun00-19-DE405.dat.gz");

  uvar->timeGPS = 714180733;


  /* register all user-variables */
  XLALRegisterUvarMember( 	detector,	STRING, 'I', REQUIRED, 	"Detector name (eg. H1,H2,L1,G1,etc).");

  XLALRegisterUvarMember(	Alpha,		REAL8, 'a', OPTIONAL,	"skyposition Alpha in radians, equatorial coords.");
  XLALRegisterUvarMember(	Delta, 		REAL8, 'd', OPTIONAL,	"skyposition Delta in radians, equatorial coords.");

  XLALRegisterUvarMember( 	timeGPS,        REAL8, 't', OPTIONAL, 	"GPS time at which to compute detector state");

  XLALRegisterUvarMember(	ephemEarth,   	 STRING, 0,  OPTIONAL,     "Earth ephemeris file to use");
  XLALRegisterUvarMember(	ephemSun,     	 STRING, 0,  OPTIONAL,     "Sun ephemeris file to use");

  XLALRegisterUvarMember(	version,        BOOLEAN, 'V', SPECIAL,      "Output code version");

  return XLAL_SUCCESS;

} /* XLALInitUserVars() */


/**
 * basic initializations: deal with user input and return standardized 'ConfigVariables'
 */
int
XLALInitCode ( ConfigVariables *cfg, const UserVariables_t *uvar, const char *app_name)
{
  if ( !cfg || !uvar || !app_name ) {
    XLALPrintError ("%s: illegal NULL pointer input.\n\n", __func__ );
    XLAL_ERROR ( XLAL_EINVAL );
  }

  /* init ephemeris data */
  XLAL_CHECK ( (cfg->edat = XLALInitBarycenter ( uvar->ephemEarth, uvar->ephemSun )) != NULL, XLAL_EFUNC );

  /* convert input REAL8 time into LIGOTimeGPS */
  if ( XLALGPSSetREAL8( &cfg->timeGPS, uvar->timeGPS ) == NULL ) {
    XLALPrintError ("%s: failed to convert input GPS %g into LIGOTimeGPS\n", __func__, uvar->timeGPS );
    XLAL_ERROR ( XLAL_EFUNC );
  }

  /* set up dummy timestamps vector containing just this one timestamps
   * (used to interface with LALComputeAM(), LALGetAMCoeffs() and LALNewGetAMCoeffs())
   */
  if ( (cfg->timestamps = XLALCreateTimestampVector( 1 )) == NULL ) {
    XLALPrintError ("%s: XLALCreateTimestampVector( 1 ) failed.", __func__ );
    XLAL_ERROR ( XLAL_EFUNC );
  }
  cfg->timestamps->data[0] = cfg->timeGPS;

  /* convert detector name into site-info */
  if ( ( cfg->det = XLALGetSiteInfo ( uvar->detector )) == NULL )
    {
      XLALPrintError ("%s: XLALGetSiteInfo('%s') failed.\n", __func__, uvar->detector );
      XLAL_ERROR ( XLAL_EFUNC );
    }

  /* NOTE: contrary to ComputeAM() and LALGetAMCoffs(), the new function LALNewGetAMCoeffs()
   * computes 'a * sinzeta' and 'b * sinzeta': for the comparison we therefore need to correct
   * for GEO's opening-angle of 94.33degrees [JKS98]: */
  if ( ! strcmp ( cfg->det->frDetector.name, "GEO_600" ) )
    cfg->sinzeta = 0.997146;
  else
    cfg->sinzeta = 1;


  /* convert input skyposition to radians Alpha/Delta [trivial here] */
  cfg->skypos.system = COORDINATESYSTEM_EQUATORIAL;
  cfg->skypos.longitude = uvar->Alpha;
  cfg->skypos.latitude  = uvar->Delta;


  return XLAL_SUCCESS;

} /* XLALInitCode() */


/** Destructor for internal configuration struct */
int
XLALDestroyConfig ( ConfigVariables *cfg )
{
  XLAL_CHECK ( cfg != NULL, XLAL_EINVAL );

  XLALDestroyUserVars ();

  XLALDestroyTimestampVector ( cfg->timestamps );

  XLALDestroyEphemerisData ( cfg->edat );

  XLALFree ( cfg->det );

  return XLAL_SUCCESS;

} /* XLALDestroyConfig() */
