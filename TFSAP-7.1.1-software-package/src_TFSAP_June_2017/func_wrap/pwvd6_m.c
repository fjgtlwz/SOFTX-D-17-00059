/*************************************************
*
* Copyright 2016 Boualem Boashash
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* Author:                 Boualem Boashash         (boualem.boashash@gmail.com)
* Maintainer since 2015:  Samir Ouelha  			(samir_ouelha@hotmail.fr)
*
* The following 2 references should be cited whenever this script is used:
* [1] B. Boashash, Samir Ouelha, Designing time-frequency  and time-scale
* features for efficient classification of non stationary signals: a tutorial
* review with performance comparison, Digital Signal Processing, 2017.
* [2] B. Boashash, Samir Ouelha, Efficient Software Matlab Package for the calculation
* of Time-Frequency Distributions related Time-Scale methods and the extraction of
* signal characteristics, SoftwareX, 2017.
* In addition, the following 3rd reference is relevant for use of the executable
* [3] B. Boashash (ed.), Time-Frequency Signal Analysis and Processing, 2nd Edition
* (London: Elsevier / Academic Press, December 2015); ISBN 978-0-12-398499-9. Book website:
* http://booksite.elsevier.com/9780123984999/
*
* Description:
*	
* gateway routine for pwvd6.c
*************************************************/

#include <math.h>
#include "mex.h"
#include "matrix.h"

#include "arithm.h"		     /* routines for complex data */
#include "tlocal.h"		     /* local function prototypes */

#include "pwvd6.h"
#include "tfsa_c.h"

void mexFunction( int nlhs, MATRIX *plhs[], int nrhs, NEED_CONST MATRIX *prhs[])
{
  double *result; /* stores the resulting PWVD */
  double *signal_r, *signal_i;
  int window_length, time_res;
  int signal_length, fft_length;
  int M,N;
  int window_order, window_r2;
  int nplts, deg, deg_r2;


   /* basic input--output number check */

   if( nrhs < 4 ) {
      tfsaErr( "pwvd61", "Not enough input arguments" );
      winNTcheck( nlhs, plhs );
      return;
   }

   if( nrhs > 5 ) {
      tfsaErr( "pwvd61", "Too many input arguments" );
      winNTcheck( nlhs, plhs );
      return;
   }

   if( nlhs > 1 ) {
      tfsaErr( "pwvd61", "Too many output arguments" );
      winNTcheck( nlhs, plhs );
      return;
   }



   /* Check first input */


   if( !mxIsNumeric( prhs[0] ) || !MXFULL( prhs[0] ) ) {
      tfsaErr( "pwvd61", "Input must be a vector" );
      winNTcheck( nlhs, plhs );
      return;
   }


   /* Get input matrix dimensions */

   M = ( int )mxGetM( prhs[0] );
   N = ( int )mxGetN( prhs[0] );

   /* Check input dimensions: A vector of unity length is considered
    * invalid; matrices are invalid. */

   if( (M==1 && N==1) || (M!=1 && N!=1) ) {
      tfsaErr( "pwvd61", "Input must be a vector" );
      winNTcheck( nlhs, plhs );
      return;
   }

   signal_length = (int)(M>N?M:N);


   /* Check second input */

   if( !GoodScalar( (MATRIX *)prhs[1] ) ) {
      tfsaErr( "pwvd61", "Smoothing window length must be a scalar" );
      winNTcheck( nlhs, plhs );
      return;
   }
   else
     window_length = (int)*(mxGetPr( prhs[1] ) );  /* Get value */


   /* Parameter value check */
   if( window_length < 1 ) {
      tfsaErr( "pwvd61", "Window length must be greater than zero" );
      winNTcheck( nlhs, plhs );
      return;
   }

   /* Check window length against signal length */

   if( window_length > signal_length ) {
      tfsaWarning( "pwvd61", "Window length has been truncated to signal length" );
      window_length = (int)signal_length;
   }


   /* Check third input */

   if( !GoodScalar( (MATRIX *)prhs[2] ) ) {
      tfsaErr( "pwvd61", "Time resolution must be a scalar" );
      winNTcheck( nlhs, plhs );
      return;
   }
   else
     time_res = (int)*(mxGetPr( prhs[2]));

   /* Parameter value check */
   if( time_res < 1 ) {
      tfsaErr( "pwvd61", "Time resolution must be greater than zero" );
      winNTcheck( nlhs, plhs );
      return;
   }

   if (time_res > signal_length) {
      tfsaErr( "pwvd61", "Time resolution must be no greater than signal length" );
      winNTcheck( nlhs, plhs );
      return;
   }


   /* Process fourth input */

   if( !GoodScalar( (MATRIX *)prhs[3] ) ) {
      tfsaErr( "pwvd61", "Interpolation degree must be a scalar" );
      winNTcheck( nlhs, plhs );
      return;
   }
   else
     deg = (int)*(mxGetPr( prhs[3]));	      /* Interpolation degree */

   deg_r2 = 1;
   while (deg_r2 < deg)
     deg_r2 <<= 1;
   deg = deg_r2;


   /* Process fifth input -- if exists */

   if (nrhs == 5) {
      if( !GoodScalar( (MATRIX *)prhs[4] ) ) {
	 tfsaErr( "pwvd61", "FFT length must be a scalar" );
	 winNTcheck( nlhs, plhs );
	 return;
      }
      else
	fft_length = (int)*(mxGetPr( prhs[4]));
   }
   else
     fft_length = (int)window_length;


	if( fft_length < 0 ) {
	   tfsaErr( "pwvd61", "FFT length must be greater than zero" );
	 winNTcheck( nlhs, plhs );
	 return;
      }

   if (fft_length < window_length) fft_length = window_length;



   /* Dereference input	*/

   signal_r = mxGetPr( prhs[0]);
   if (mxIsComplex(prhs[0]))
     signal_i = mxGetPi( prhs[0]);
   else
     signal_i = NULL;


   /* Calculate number of times for computing PWVD */
   nplts = (int) ceil((double)signal_length/time_res);

   /* calculate radix-2 value equal or above window_length */
   window_order = 0;
   window_r2 = 1;
   while( window_r2 < fft_length)
     {
        window_order++;
	window_r2 <<= 1;
     }

  plhs[0] = MXCREATEFULL((int)(window_r2*0.5), nplts, REAL);
   if (plhs[0] == NULL)
     {
	tfsaErr( "pwvd61", "Memory allocation failed");
	winNTcheck( nlhs, plhs );
	return;
     }
   result = mxGetPr( plhs[0] );

   if( pwvd61( signal_r, signal_i, signal_length, result, nplts, time_res,
	     window_length, window_r2, window_order, deg) ) {
		tfsaErr( "pwvd61", "Function failed" );
		winNTcheck( nlhs, plhs );
	     }

}




