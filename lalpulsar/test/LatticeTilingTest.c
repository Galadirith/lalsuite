//
// Copyright (C) 2014 Karl Wette
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with with program; see the file COPYING. If not, write to the
// Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
// MA 02111-1307 USA
//

// Tests of the lattice-based template generation code in LatticeTiling.[ch].

#include <stdio.h>
#include <inttypes.h>
#include <float.h>
#include <math.h>
#include <errno.h>
#include <gsl/gsl_blas.h>

#include <lal/LatticeTiling.h>
#include <lal/LALStdlib.h>
#include <lal/LALStdio.h>
#include <lal/XLALError.h>
#include <lal/LALConstants.h>
#include <lal/Factorial.h>
#include <lal/DopplerFullScan.h>
#include <lal/SuperSkyMetrics.h>
#include <lal/LALInitBarycenter.h>

#include "../src/GSLHelpers.h"

#define MISM_HIST_BINS 20

// The reference mismatch histograms were generated in Octave
// using the LatticeMismatchHist() function available in OctApps.

const double Z1_A1s_mism_hist[MISM_HIST_BINS+1] = {
  4.531107, 1.870257, 1.430467, 1.202537, 1.057047, 0.953084, 0.875050, 0.813050, 0.762368, 0.719968,
  0.683877, 0.652659, 0.625394, 0.601300, 0.579724, 0.560515, 0.542944, 0.527142, 0.512487, 0.499022
};

const double Z2_mism_hist[MISM_HIST_BINS+1] = {
  1.570963, 1.571131, 1.571074, 1.571102, 1.570808, 1.570789, 1.570617, 1.570716, 1.570671, 1.570867,
  1.157132, 0.835785, 0.645424, 0.503305, 0.389690, 0.295014, 0.214022, 0.143584, 0.081427, 0.025878
};

const double Z3_mism_hist[MISM_HIST_BINS+1] = {
  0.608404, 1.112392, 1.440652, 1.705502, 1.934785, 2.139464, 2.296868, 2.071379, 1.748278, 1.443955,
  1.155064, 0.879719, 0.616210, 0.375368, 0.223752, 0.131196, 0.071216, 0.033130, 0.011178, 0.001489
};

const double A2s_mism_hist[MISM_HIST_BINS+1] = {
  1.210152, 1.210142, 1.209837, 1.209697, 1.209368, 1.209214, 1.209399, 1.209170, 1.208805, 1.208681,
  1.208631, 1.208914, 1.208775, 1.209021, 1.208797, 0.816672, 0.505394, 0.315665, 0.170942, 0.052727
};

const double A3s_mism_hist[MISM_HIST_BINS+1] = {
  0.327328, 0.598545, 0.774909, 0.917710, 1.040699, 1.150991, 1.250963, 1.344026, 1.431020, 1.512883,
  1.590473, 1.664510, 1.595423, 1.391209, 1.194340, 1.004085, 0.729054, 0.371869, 0.098727, 0.011236
};

static int BasicTest(
  const size_t n,
  const TilingLattice lattice,
  const UINT8 total_ref
  )
{

  // Create lattice tiling parameter space
  printf("Number of dimensions: %zu\n", n);
  LatticeTilingSpace* space = XLALCreateLatticeTilingSpace(n);
  XLAL_CHECK(space != NULL, XLAL_EFUNC);

  // Add bounds
  for (size_t i = 0; i < n; ++i) {
    XLAL_CHECK(XLALSetLatticeTilingConstantBound(space, i, 0.0, pow(100.0, 1.0/n)) == XLAL_SUCCESS, XLAL_EFUNC);
  }

  // Set metric to the Lehmer matrix
  gsl_matrix* GAMAT(metric, n, n);
  for (size_t i = 0; i < n; ++i) {
    for (size_t j = 0; j < n; ++j) {
      const double ii = i+1, jj = j+1;
      gsl_matrix_set(metric, i, j, jj >= ii ? ii/jj : jj/ii);
    }
  }

  // Create lattice tiling
  printf("Lattice type: %u\n", lattice);
  LatticeTiling* tiling = XLALCreateLatticeTiling(space, lattice, metric, 0.3);
  XLAL_CHECK(tiling != NULL, XLAL_EFUNC);

  // Count number of templates
  UINT8 total = XLALLatticeTilingTotalPointCount(tiling);
  printf("Number of lattice points: %" LAL_UINT8_FORMAT "\n", total);
  XLAL_CHECK(total == total_ref, XLAL_EFUNC, "ERROR: total = %" LAL_UINT8_FORMAT " != %" LAL_UINT8_FORMAT " = total_ref", total, total_ref);

  // Get all templates
  gsl_matrix* templates = XLALLatticeTilingUniquePoints(tiling, n - 1);
  XLAL_CHECK(templates != NULL, XLAL_EFAILED);

  // Get nearest point to each template; should be template itself
  gsl_matrix* nearest = NULL;
  UINT8Vector* indices = NULL;
  XLAL_CHECK(XLALNearestLatticeTilingPoints(tiling, templates, &nearest, &indices) == XLAL_SUCCESS, XLAL_EFUNC);
  UINT8 failed = 0;
  for (UINT8 i = 0; i < total; ++i) {
    if (indices->data[i] != i) {
      ++failed;
      XLALPrintError("ERROR: indices->data[i] = %" LAL_UINT8_FORMAT " != %" LAL_UINT8_FORMAT "\n", indices->data[i], i);
    }
  }
  if (failed > 0) {
    XLAL_ERROR(XLAL_EFAILED, "ERROR: number of failed index lookups = %" LAL_UINT8_FORMAT " > 0", failed);
  }

  // Cleanup
  XLALDestroyLatticeTilingSpace(space);
  XLALDestroyLatticeTiling(tiling);
  XLALDestroyUINT8Vector(indices);
  GFMAT(metric, templates, nearest);
  LALCheckMemoryLeaks();
  printf("\n");

  return XLAL_SUCCESS;

}

static int MismatchTest(
  LatticeTilingSpace* space,
  gsl_matrix* metric,
  const double max_mismatch,
  const TilingLattice lattice,
  const UINT8 total_ref,
  const double mism_hist_ref[MISM_HIST_BINS+1]
  )
{

  // Create lattice tiling
  printf("Lattice type: %u\n", lattice);
  LatticeTiling* tiling = XLALCreateLatticeTiling(space, lattice, metric, max_mismatch);
  XLAL_CHECK(tiling != NULL, XLAL_EFUNC);
  const size_t n = XLALTotalLatticeTilingDimensions(tiling);

  // Count number of templates
  UINT8 total = XLALLatticeTilingTotalPointCount(tiling);
  printf("Number of lattice points: %" LAL_UINT8_FORMAT "\n", total);
  XLAL_CHECK(total == total_ref, XLAL_EFUNC, "ERROR: total = %" LAL_UINT8_FORMAT " != %" LAL_UINT8_FORMAT " = total_ref", total, total_ref);

  // Get all templates
  gsl_matrix* templates = XLALLatticeTilingUniquePoints(tiling, n - 1);
  XLAL_CHECK(templates != NULL, XLAL_EFAILED);

  // Initialise mismatch histogram counts
  double mism_hist[MISM_HIST_BINS+1] = {0};

  // Perform 10 injections for every template
  gsl_matrix* GAMAT(injections, 3, total);
  gsl_matrix* GAMAT(nearest, 3, total);
  gsl_matrix* GAMAT(temp, 3, total);
  RandomParams* rng = XLALCreateRandomParams(total);
  XLAL_CHECK(rng != NULL, XLAL_EFUNC);
  for (size_t i = 0; i < 10; ++i) {

    // Generate random injection points
    XLAL_CHECK(XLALRandomLatticeTilingPoints(space, rng, injections) == XLAL_SUCCESS, XLAL_EFUNC);

    // Find nearest lattice template points
    XLAL_CHECK(XLALNearestLatticeTilingPoints(tiling, injections, &nearest, NULL) == XLAL_SUCCESS, XLAL_EFUNC);

    // Compute mismatch between injections
    gsl_matrix_sub(nearest, injections);
    gsl_blas_dsymm(CblasLeft, CblasUpper, 1.0, metric, nearest, 0.0, temp);
    for (size_t j = 0; j < temp->size2; ++j) {
      gsl_vector_view temp_j = gsl_matrix_column(temp, j);
      gsl_vector_view nearest_j = gsl_matrix_column(nearest, j);
      double mismatch = 0.0;
      gsl_blas_ddot(&nearest_j.vector, &temp_j.vector, &mismatch);
      mismatch /= max_mismatch;

      // Add mismatch to histogram; out-of-range values go in last bin
      if (mismatch < 0.0 || mismatch > 1.0) {
        ++mism_hist[MISM_HIST_BINS];
      } else {
        ++mism_hist[lround(floor(mismatch * MISM_HIST_BINS))];
      }

    }

  }

  // Normalise histogram
  double mism_hist_total = 0.0;
  for (size_t i = 0; i < MISM_HIST_BINS + 1; ++i) {
    mism_hist_total += mism_hist[i];
  }
  for (size_t i = 0; i < MISM_HIST_BINS + 1; ++i) {
    mism_hist[i] *= MISM_HIST_BINS / mism_hist_total;
  }

  // Print mismatch histogram and its reference
  printf("Mismatch histogram: ");
  for (size_t i = 0; i < MISM_HIST_BINS + 1; ++i) {
    printf(" %0.3f", mism_hist[i]);
  }
  printf("\n");
  printf("Reference histogram:");
  for (size_t i = 0; i < MISM_HIST_BINS + 1; ++i) {
    printf(" %0.3f", mism_hist_ref[i]);
  }
  printf("\n");

  // Determine error between mismatch histogram and its reference
  double mism_hist_error = 0.0;
  for (size_t i = 0; i < MISM_HIST_BINS + 1; ++i) {
    mism_hist_error += fabs(mism_hist[i] - mism_hist_ref[i]);
  }
  mism_hist_error /= MISM_HIST_BINS + 1;
  printf("Mismatch histogram error: %0.3e\n", mism_hist_error);
  const double mism_hist_error_tol = 5e-2;
  if (mism_hist_error >= mism_hist_error_tol) {
    XLAL_ERROR(XLAL_EFAILED, "ERROR: mismatch histogram error exceeds %0.3e\n", mism_hist_error_tol);
  }

  // Cleanup
  XLALDestroyLatticeTilingSpace(space);
  XLALDestroyLatticeTiling(tiling);
  XLALDestroyRandomParams(rng);
  GFMAT(metric, templates, injections, nearest, temp);
  LALCheckMemoryLeaks();
  printf("\n");

  return XLAL_SUCCESS;

}

static int MismatchSquareTest(
  const TilingLattice lattice,
  const double freqband,
  const double f1dotband,
  const double f2dotband,
  const UINT8 total_ref,
  const double mism_hist_ref[MISM_HIST_BINS+1]
  )
{

  // Create lattice tiling parameter space
  LatticeTilingSpace* space = XLALCreateLatticeTilingSpace(3);
  XLAL_CHECK(space != NULL, XLAL_EFUNC);

  // Add bounds
  const double fndot[3] = {100, 0, 0};
  const double fndotband[3] = {freqband, f1dotband, f2dotband};
  for (size_t i = 0; i < 3; ++i) {
    printf("Bounds: f%zudot=%0.3g, f%zudotband=%0.3g\n", i, fndot[i], i, fndotband[i]);
    XLAL_CHECK(XLALSetLatticeTilingConstantBound(space, i, fndot[i], fndot[i] + fndotband[i])
               == XLAL_SUCCESS, XLAL_EFUNC);
  }

  // Set metric to the spindown metric
  gsl_matrix* GAMAT(metric, 3, 3);
  for (size_t i = 0; i < metric->size1; ++i) {
    for (size_t j = i; j < metric->size2; ++j) {
      const double Tspan = 432000;
      gsl_matrix_set(metric, i, j, (
                       4.0 * LAL_PI * LAL_PI * pow(Tspan, i + j + 2) * (i + 1) * (j + 1)
                       ) / (
                         LAL_FACT[i + 1] * LAL_FACT[j + 1] * (i + 2) * (j + 2) * (i + j + 3)
                         ));
      gsl_matrix_set(metric, j, i, gsl_matrix_get(metric, i, j));
    }
  }

  // Perform mismatch test
  XLAL_CHECK(MismatchTest(space, metric, 0.3, lattice, total_ref, mism_hist_ref) == XLAL_SUCCESS, XLAL_EFUNC);

  return XLAL_SUCCESS;

}

static int MismatchAgeBrakeTest(
  const TilingLattice lattice,
  const double freq,
  const double freqband,
  const UINT8 total_ref,
  const double mism_hist_ref[MISM_HIST_BINS+1]
  )
{

  // Create lattice tiling parameter space
  LatticeTilingSpace* space = XLALCreateLatticeTilingSpace(3);
  XLAL_CHECK(space != NULL, XLAL_EFUNC);

  // Add bounds
  printf("Bounds: freq=%0.3g, freqband=%0.3g\n", freq, freqband);
  XLAL_CHECK(XLALSetLatticeTilingConstantBound(space, 0, freq, freq + freqband) == XLAL_SUCCESS, XLAL_EFUNC);
  XLAL_CHECK(XLALSetLatticeTilingF1DotAgeBrakingBound(space, 0, 1, 1e11, 2, 5) == XLAL_SUCCESS, XLAL_EFUNC);
  XLAL_CHECK(XLALSetLatticeTilingF2DotBrakingBound(space, 0, 1, 2, 2, 5) == XLAL_SUCCESS, XLAL_EFUNC);

  // Set metric to the spindown metric
  gsl_matrix* GAMAT(metric, 3, 3);
  for (size_t i = 0; i < metric->size1; ++i) {
    for (size_t j = i; j < metric->size2; ++j) {
      const double Tspan = 1036800;
      gsl_matrix_set(metric, i, j, (
                       4.0 * LAL_PI * LAL_PI * pow(Tspan, i + j + 2) * (i + 1) * (j + 1)
                       ) / (
                         LAL_FACT[i + 1] * LAL_FACT[j + 1] * (i + 2) * (j + 2) * (i + j + 3)
                         ));
      gsl_matrix_set(metric, j, i, gsl_matrix_get(metric, i, j));
    }
  }

  // Perform mismatch test
  XLAL_CHECK(MismatchTest(space, metric, 0.3, lattice, total_ref, mism_hist_ref) == XLAL_SUCCESS, XLAL_EFUNC);

  return XLAL_SUCCESS;

}

static int SuperSkyTest(
  const double T,
  const double max_mismatch,
  const TilingLattice lattice,
  const double freq,
  const double freqband,
  const UINT8 total_ref,
  const double mism_hist_ref[MISM_HIST_BINS+1]
  )
{

  // Create lattice tiling parameter space
  LatticeTilingSpace *space = XLALCreateLatticeTilingSpace(3);
  XLAL_CHECK(space != NULL, XLAL_EFUNC);

  // Compute reduced super-sky metric
  const double Tspan = T * 86400;
  gsl_matrix *essky_metric = NULL;
  LIGOTimeGPS ref_time;
  XLALGPSSetREAL8(&ref_time, 900100100);
  LALSegList segments;
  {
    XLAL_CHECK(XLALSegListInit(&segments) == XLAL_SUCCESS, XLAL_EFUNC);
    LALSeg segment;
    LIGOTimeGPS start_time = ref_time, end_time = ref_time;
    XLALGPSAdd(&start_time, -0.5 * Tspan);
    XLALGPSAdd(&end_time, 0.5 * Tspan);
    XLAL_CHECK(XLALSegSet(&segment, &start_time, &end_time, 0) == XLAL_SUCCESS, XLAL_EFUNC);
    XLAL_CHECK(XLALSegListAppend(&segments, &segment) == XLAL_SUCCESS, XLAL_EFUNC);
  }
  MultiLALDetector detectors = {
    .length = 1,
    .sites = { lalCachedDetectors[LAL_LLO_4K_DETECTOR] }
  };
  EphemerisData* edat =  XLALInitBarycenter(TEST_DATA_DIR "earth00-19-DE405.dat.gz",
                                            TEST_DATA_DIR "sun00-19-DE405.dat.gz");
  XLAL_CHECK(edat != NULL, XLAL_EFUNC);
  XLAL_CHECK(XLALExpandedSuperSkyMetric(&essky_metric, 0, &ref_time, &segments, freq, &detectors,
                                        NULL, DETMOTION_SPIN | DETMOTION_PTOLEORBIT, edat)
             == XLAL_SUCCESS, XLAL_EFUNC);
  XLALSegListClear(&segments);
  XLALDestroyEphemerisData(edat);
  gsl_matrix *rssky_metric = NULL, *rssky_transf = NULL;
  XLAL_CHECK(XLALReducedSuperSkyMetric(&rssky_metric, &rssky_transf, essky_metric)
             == XLAL_SUCCESS, XLAL_EFUNC);
  GFMAT(essky_metric);

  // Add bounds
  printf("Bounds: super-sky, freq=%0.3g, freqband=%0.3g\n", freq, freqband);
  XLAL_CHECK(XLALSetLatticeTilingReducedSuperSkyBounds(space)
             == XLAL_SUCCESS, XLAL_EFUNC);
  XLAL_CHECK(XLALSetLatticeTilingPhysicalSpinBound(space, rssky_transf, 0, freq, freq + freqband)
             == XLAL_SUCCESS, XLAL_EFUNC);
  GFMAT(rssky_transf);

  // Perform mismatch test
  XLAL_CHECK(MismatchTest(space, rssky_metric, max_mismatch, lattice, total_ref, mism_hist_ref)
             == XLAL_SUCCESS, XLAL_EFUNC);

  return XLAL_SUCCESS;

}

int main(void) {

  // Perform basic tests
  XLAL_CHECK_MAIN(BasicTest(1, TILING_LATTICE_CUBIC,    93) == XLAL_SUCCESS, XLAL_EFUNC);
  XLAL_CHECK_MAIN(BasicTest(1, TILING_LATTICE_ANSTAR,   93) == XLAL_SUCCESS, XLAL_EFUNC);
  XLAL_CHECK_MAIN(BasicTest(2, TILING_LATTICE_CUBIC,   190) == XLAL_SUCCESS, XLAL_EFUNC);
  XLAL_CHECK_MAIN(BasicTest(2, TILING_LATTICE_ANSTAR,  144) == XLAL_SUCCESS, XLAL_EFUNC);
  XLAL_CHECK_MAIN(BasicTest(3, TILING_LATTICE_CUBIC,   583) == XLAL_SUCCESS, XLAL_EFUNC);
  XLAL_CHECK_MAIN(BasicTest(3, TILING_LATTICE_ANSTAR,  332) == XLAL_SUCCESS, XLAL_EFUNC);
  XLAL_CHECK_MAIN(BasicTest(4, TILING_LATTICE_CUBIC,  2543) == XLAL_SUCCESS, XLAL_EFUNC);
  XLAL_CHECK_MAIN(BasicTest(4, TILING_LATTICE_ANSTAR,  897) == XLAL_SUCCESS, XLAL_EFUNC);

  // Perform mismatch tests with a square parameter space
  XLAL_CHECK_MAIN(MismatchSquareTest(TILING_LATTICE_CUBIC,  0.03,     0,     0, 21460, Z1_A1s_mism_hist)
                  == XLAL_SUCCESS, XLAL_EFUNC);
  XLAL_CHECK_MAIN(MismatchSquareTest(TILING_LATTICE_CUBIC,  2e-4, -2e-9,     0, 23763,     Z2_mism_hist)
                  == XLAL_SUCCESS, XLAL_EFUNC);
  XLAL_CHECK_MAIN(MismatchSquareTest(TILING_LATTICE_CUBIC,  1e-4, -1e-9, 1e-17, 19550,     Z3_mism_hist)
                  == XLAL_SUCCESS, XLAL_EFUNC);
  XLAL_CHECK_MAIN(MismatchSquareTest(TILING_LATTICE_ANSTAR, 0.03,     0,     0, 21460, Z1_A1s_mism_hist)
                  == XLAL_SUCCESS, XLAL_EFUNC);
  XLAL_CHECK_MAIN(MismatchSquareTest(TILING_LATTICE_ANSTAR, 2e-4, -2e-9,     0, 18283,    A2s_mism_hist)
                  == XLAL_SUCCESS, XLAL_EFUNC);
  XLAL_CHECK_MAIN(MismatchSquareTest(TILING_LATTICE_ANSTAR, 1e-4, -2e-9, 2e-17, 20268,    A3s_mism_hist)
                  == XLAL_SUCCESS, XLAL_EFUNC);

  // Perform mismatch tests with an age--braking index parameter space
  XLAL_CHECK_MAIN(MismatchAgeBrakeTest(TILING_LATTICE_ANSTAR, 100, 4.0e-5, 37870, A3s_mism_hist)
                  == XLAL_SUCCESS, XLAL_EFUNC);
  XLAL_CHECK_MAIN(MismatchAgeBrakeTest(TILING_LATTICE_ANSTAR, 200, 1.5e-5, 37230, A3s_mism_hist)
                  == XLAL_SUCCESS, XLAL_EFUNC);
  XLAL_CHECK_MAIN(MismatchAgeBrakeTest(TILING_LATTICE_ANSTAR, 300, 1.0e-5, 37022, A3s_mism_hist)
                  == XLAL_SUCCESS, XLAL_EFUNC);

  // Perform mismatch tests with the reduced super-sky parameter space and metric
  XLAL_CHECK_MAIN(SuperSkyTest(2.0, 0.5, TILING_LATTICE_ANSTAR, 50, 1e-4, 111520, A3s_mism_hist)
                  == XLAL_SUCCESS, XLAL_EFUNC);

  return EXIT_SUCCESS;

}