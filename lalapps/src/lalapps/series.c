#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <FrameL.h>
#include "series.h"

double epoch_diff( const LIGOTimeGPS *t2, const LIGOTimeGPS *t1 )
{
  long long dt;
  dt  = 1000000000LL * (long long) t2->gpsSeconds;
  dt += (long long) t2->gpsNanoSeconds;
  dt -= 1000000000LL * (long long) t1->gpsSeconds;
  dt -= (long long) t1->gpsNanoSeconds;
  return 1e-9 * (double)dt;
}

void epoch_add( LIGOTimeGPS *t1, LIGOTimeGPS *t0, double dt )
{
  long long t;
  t  = 1000000000LL * (long long) t0->gpsSeconds;
  t += (long long) t0->gpsNanoSeconds;
  t += (long long)( 1e9 * dt );
  t1->gpsSeconds = t / 1000000000LL;
  t1->gpsNanoSeconds = t % 1000000000LL;
  return;
}

int write_ilwd( const char *fname, const struct series *ser )
{
  size_t i;
  FILE *fp;
  if ( ser->type != FR_VECT_4R && ser->type != FR_VECT_8C )
  {
    fprintf( stderr, "unknown data type in series: %d\nmust be 4R or 8C\n",
        ser->type );
    return 1;
  }
  fp = fopen( fname, "w" );
  fprintf( fp, "<?ilwd?>\n" );
  if ( IS_TIME( ser->dom) )
  {
    fprintf( fp, "<ilwd name='%s::sequence' size='7'>\n", ser->name );
    if ( ser->type == FR_VECT_8C )
    {
      fprintf( fp, "<lstring name='complex:domain' size='4'>TIME</lstring>\n" );
    }
    else
    {
      fprintf( fp, "<lstring name='real:domain' size='4'>TIME</lstring>\n" );
    }
  }
  else
  {
    fprintf( fp, "<ilwd name='%s::sequence' size='9'>\n", ser->name );
    if ( ser->type == FR_VECT_8C )
    {
      fprintf( fp, "<lstring name='complex:domain' size='4'>FREQ</lstring>\n" );
    }
    else
    {
      fprintf( fp, "<lstring name='real:domain' size='4'>FREQ</lstring>\n" );
    }
  }
  fprintf( fp, "<int_4u name='gps_sec:start_time' units='sec'>%d</int_4u>\n",
      ser->tbeg.gpsSeconds );
  fprintf( fp, "<int_4u name='gps_nan:start_time' units='nanosec'>%d</int_4u>\n",
      ser->tbeg.gpsNanoSeconds );
  fprintf( fp, "<int_4u name='gps_sec:stop_time' units='sec'>%d</int_4u>\n",
      ser->tend.gpsSeconds );
  fprintf( fp, "<int_4u name='gps_nan:stop_time' units='nanosec'>%d</int_4u>\n",
      ser->tend.gpsNanoSeconds );
  if ( IS_TIME( ser->dom) )
  {
    fprintf( fp, "<real_8 name='time:step_size' units='sec'>%e</real_8>\n",
        ser->step );
  }
  else
  {
    fprintf( fp, "<real_8 name='start_freq' units='hz'>0</real_8>\n" );
    fprintf( fp, "<real_8 name='stop_freq' units='hz'>%e</real_8>\n",
        ( ser->size - 1 ) * ser->step );
    fprintf( fp, "<real_8 name='freq:step_size' units='hz'>%e</real_8>\n", 
        ser->step );
  }
  if ( ser->type == FR_VECT_8C )
  {
    fprintf( fp, "<complex_8 dims='%d' name='data' units='%s'>",
        (int)ser->size, ser->unit );
    fprintf( fp, "%e %e", ser->data[0], ser->data[1] );
    for ( i = 1; i < ser->size; ++i )
      fprintf( fp, " %e %e", ser->data[2*i], ser->data[2*i+1] );
    fprintf( fp, "</complex_8>\n" );
  }
  else
  {
    fprintf( fp, "<real_4 dims='%d' name='data' units='%s'>",
        (int)ser->size, ser->unit );
    fprintf( fp, "%e", ser->data[0] );
    for ( i = 1; i < ser->size; ++i )
      fprintf( fp, " %e", ser->data[i] );
    fprintf( fp, "</real_4>\n" );
  }
  fprintf( fp, "</ilwd>\n" );
  fclose( fp );
  return 0;
}

FrameH *fr_add_proc_data( FrameH *frame, const struct series *ser )
{
  char *channel = ser->name;
  char comment[] = "Generated by $Id$";
  char seconds[] = "s";
  char hertz[]   = "Hz";
  struct FrVect     *vect;
  struct FrProcData *proc;
  size_t i;

  if ( ser->type != FR_VECT_4R && ser->type != FR_VECT_8C )
  {
    fprintf( stderr, "unknown data type in series: %d\nmust be 4R or 8C\n",
        ser->type );
    return NULL;
  }

  if ( ! frame )
  {
    char src[2];
    src[0] = ser->name[0];
    src[1] = 0;
    frame = FrameHNew( src );
    frame->run    = 1;
    frame->frame  = 1;
    frame->GTimeS = (int) ser->tbeg.gpsSeconds;
    frame->GTimeN = (int) ser->tbeg.gpsNanoSeconds;
    frame->dt     = epoch_diff( &ser->tend, &ser->tbeg );
  }

  vect = FrVectNew1D( channel, ser->type, ser->size, ser->step,
      IS_TIME( ser->dom) ? seconds : hertz, ser->unit );
  proc = calloc( 1, sizeof( *proc ) );
  proc->classe     = FrProcDataDef();
#if defined FR_VERS && FR_VERS < 5000
  proc->sampleRate = IS_TIME( ser->dom ) ? 1.0 / ser->step : -1;
#endif
#if defined FR_VERS && FR_VERS >= 6000
  proc->type = IS_TIME( ser->dom ) ? 1 : 2;
  proc->subType = IS_TRANS( ser->dom ) ? 6 : 3;
#endif
  proc->fShift     = 0;
  proc->data       = vect;
  proc->next       = frame->procData;
  frame->procData  = proc;
  FrStrCpy( &proc->name, channel );
  FrStrCpy( &proc->comment, comment );
  if ( ser->type == FR_VECT_8C )
  {
    for ( i = 0; i < ser->size; ++i )
    {
      vect->dataF[2*i]   = ser->data[2*i];
      vect->dataF[2*i+1] = ser->data[2*i+1];
    }
  }
  else
  {
    for ( i = 0; i < ser->size; ++i )
    {
      vect->dataF[i] = ser->data[i];
    }
  }

  return frame;
}

