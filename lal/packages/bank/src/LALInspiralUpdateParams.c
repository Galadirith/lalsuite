/*  <lalVerbatim file="LALInspiralUpdateParamsCV">
Author: Sathyaprakash, B. S.
$Id$
</lalVerbatim>  */

/*  <lalLaTeX>

\subsection{Module \texttt{LALInspiralUpdateParams.c}}

Module to update the parameters used in creating a coarse
bank. While scanning the $x0$-direction after reaching the 
boundary of the parameter space, we have to return to the 
starting point of the same line and use the metric there
to scan one step upwards in the direction of $x1.$ 
to a {\it template list}.

\subsubsection*{Prototypes}
\vspace{0.1in}
\input{LALInspiralUpdateParamsCP}
\index{\texttt{LALInspiralUpdateParams()}}

\subsubsection*{Description}

Description info etc.

\subsubsection*{Algorithm}

Copy the parameters in the temporary parameter structure
to the current parameter structure.

\subsubsection*{Uses}
None.

\subsubsection*{Notes}

\vfill{\footnotesize\input{LALInspiralUpdateParamsCV}}

</lalLaTeX>  */



#include <lal/LALInspiralBank.h>

NRCSID (LALINSPIRALUPDATEPARAMSC, "Id: $");

/*  <lalVerbatim file="LALInspiralUpdateParamsCP"> */
void 
LALInspiralUpdateParams(
   LALStatus               *status,
   InspiralBankParams   *bankParams,
   InspiralMetric       metric,
   REAL8                minimalmatch
)
{  /*  </lalVerbatim>  */
   REAL8 dx0, dx1, myphi, theta, fac;

   INITSTATUS (status, "LALInspiralUpdateParams", LALINSPIRALUPDATEPARAMSC);

   dx0 = sqrt(2. * (1. - minimalmatch)/metric.g00 );
   dx1 = sqrt(2. * (1. - minimalmatch)/metric.g11 );
   myphi = atan2(dx1, dx0);
   theta = fabs(metric.theta);
   if (theta <= myphi) {
      fac = cos(theta);
      bankParams->dx0 = dx0 / fac;
      bankParams->dx1 = dx1 * fac;
   } 
   else {
      fac = sin(theta);
      bankParams->dx0 = dx1 / fac;
      bankParams->dx1 = dx0 * fac;
   }

   RETURN (status);

}
