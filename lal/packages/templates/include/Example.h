/**** <lalVerbatim file="ExampleHV">
 * Author: Al A. Lal
 * $Id$
 **** </lalVerbatim> */

/**** <lalLaTeX>
 * 
 * \section{Header \texttt{Example.h}}
 *
 * %% One sentence briefly defining scope of the header
 *
 * \subsection*{Synopsis}
 * \begin{verbatim}
 * #include <lal/Example.h>
 * \end{verbatim}
 * 
 * %% Generic documentation on the header; this is the main place to
 * %% document any stuff not specific to the module
 * 
 * \subsection*{Error conditions}
 * \input{ExampleHE}
 * 
 * \subsection*{Structures}
 * 
 * %% Document here any structures defined in the header.  
 * %% Also include any of them in the index; e.g.:
 * %% \index{\texttt{ExampleOutput}}
 * %% \index{\texttt{ExampleInput}}
 * %% \index{\texttt{ExampleParams}}
 * 
 * \vfill{\footnotesize\input{ExampleHV}}
 * \newpage\input{ExampleC}
 * \newpage\input{ExampleTestC}
 * 
 **** </lalLaTeX> */

#ifndef _EXAMPLE_H
#define _EXAMPLE_H

/** INCLUDE LAL HEADERS NEEDED FOR HEADER (NOT MODULE) **/
#include <lal/LALStdlib.h>

#ifdef  __cplusplus
extern "C" {
#pragma } /** to match the previous brace **/
#endif

/** DEFINE RCS ID STRING **/  
NRCSID( EXAMPLEH, "$Id$" );

/** DEFINE ERROR CODES AND MESSAGES **/
/**** <lalErrTable file="ExampleHE"> */
#define EXAMPLEH_ENULL 1
#define EXAMPLEH_EALOC 2
#define EXAMPLEH_EIEIO 4
#define EXAMPLEH_EMOTE 8
#define EXAMPLEH_MSGENULL "Null pointer"
#define EXAMPLEH_MSGEALOC "Memory allocation error"
#define EXAMPLEH_MSGEIEIO "Have bought the barn"
#define EXAMPLEH_MSGEMOTE "I can't believe this is happening!"
/**** </lalErrTable> */

/** DEFINE NEW STRUCTURES AND TYPES **/
typedef struct tagExampleOutput { /** contents **/ } ExampleOutput; 
typedef struct tagExampleInput  { /** contents **/ } ExampleInput; 
typedef struct tagExampleParams { /** contents **/ } ExampleParams; 

/** FUNCTION PROTOTYPES **/
void
LALExample(
    LALStatus     *status,
    ExampleOutput *output,
    ExampleInput  *input,
    ExampleParams *params
    );

#ifdef  __cplusplus
#pragma { /** to match the next brace **/
}
#endif

#endif /* _EXAMPLE_H */
