//
// Copyright (C) 2011--2014 Karl Wette
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with with program; see the file COPYING. If not, write to the
// Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
// MA  02111-1307  USA
//

///
/// \defgroup SWIGLALOmega_i Interface SWIGLALOmega.i
/// \ingroup lal_swig
/// \brief SWIG code which must appear \e after the LAL headers.
/// \author Karl Wette
///

///
/// # Specialised wrapping of <tt>gsl_rng</tt>
///

///
/// Wrap the \c gsl_rng class.
typedef struct {
  %extend {
    /// <ul><li>

    /// Construct a new \c gsl_rng from another \c gsl_rng.
    gsl_rng(const gsl_rng* rng) {
      XLAL_CHECK_NULL(rng != NULL, XLAL_EFAULT);
      return gsl_rng_clone(rng);
    }

    /// </li><li>

    /// Construct a new \c gsl_rng from a generator name and random seed.
    gsl_rng(const char* name, unsigned long int seed) {
      XLAL_CHECK_NULL(name != NULL, XLAL_EFAULT, "Generator name must be non-NULL");
      gsl_rng_env_setup();
      const gsl_rng_type* T = NULL;
      if (strcmp(name, "default") == 0) {
        T = gsl_rng_default;
      } else {
        const gsl_rng_type **types = gsl_rng_types_setup();
        for (const gsl_rng_type **t = types; *t != NULL; ++t) {
          if (strcmp(name, (*t)->name) == 0) {
            T = *t;
            break;
          }
        }
      }
      XLAL_CHECK_NULL(T != NULL, XLAL_EINVAL, "Could not find generator named '%s'", name);
      gsl_rng* rng = gsl_rng_alloc(T);
      gsl_rng_set(rng, seed);
      return rng;
    }

    /// </li><li>

    /// Destroy a \c gsl_rng.
    ~gsl_rng() {
      %swiglal_struct_call_dtor(gsl_rng_free, $self);
    }

    /// </li><li>

    /// Properties and methods of a \c gsl_rng, most of which map to \c
    /// gsl_rng_...() functions.
    const char* name();
    double uniform();
    double uniform_pos();
    unsigned long int uniform_int(unsigned long int n);
    void set_seed(unsigned long int seed) {
      gsl_rng_set($self, seed);
    }
    unsigned long int get_value() {
      return gsl_rng_get($self);
    }
    unsigned long int max_value() {
      return gsl_rng_max($self);
    }
    unsigned long int min_value() {
      return gsl_rng_min($self);
    }

    /// </li></ul>
  }
} gsl_rng;
///

///
/// # Specialised wrapping of ::LIGOTimeGPS
///

///
/// Allocate a new ::LIGOTimeGPS.
///
#define %swiglal_new_LIGOTimeGPS() (LIGOTimeGPS*)(XLALCalloc(1, sizeof(LIGOTimeGPS)))

///
/// Extend the ::LIGOTimeGPS class.
%extend tagLIGOTimeGPS {
  /// <ul><li>

  /// Construct a new ::LIGOTimeGPS from another ::LIGOTimeGPS.
  tagLIGOTimeGPS(const LIGOTimeGPS* gps) {
    return (LIGOTimeGPS*)memcpy(%swiglal_new_LIGOTimeGPS(), gps, sizeof(*gps));
  }

  /// </li><li>

  /// Construct a new ::LIGOTimeGPS from a real number.
  tagLIGOTimeGPS(REAL8 t) {
    return XLALGPSSetREAL8(%swiglal_new_LIGOTimeGPS(), t);
  }

  /// </li><li>

  /// Construct a new ::LIGOTimeGPS from integer seconds and nanoseconds.
  tagLIGOTimeGPS(INT4 gpssec) {
    return XLALGPSSet(%swiglal_new_LIGOTimeGPS(), gpssec, 0);
  }
  tagLIGOTimeGPS(INT4 gpssec, INT8 gpsnan) {
    return XLALGPSSet(%swiglal_new_LIGOTimeGPS(), gpssec, gpsnan);
  }

  /// </li><li>

  /// Construct a new ::LIGOTimeGPS from a string
  tagLIGOTimeGPS(const char *str) {
    LIGOTimeGPS *gps = %swiglal_new_LIGOTimeGPS();
    char *end = NULL;
    if (XLALStrToGPS(gps, str, &end) < 0 || *end != '\0') {
      XLALFree(gps);
      XLAL_ERROR_NULL(XLAL_EINVAL, "'%s' is not a valid LIGOTimeGPS", str);
    }
    return gps;
  }

  /// </li><li>

  /// Operators are implemented by defining Python-style <tt>__operator__</tt>
  /// methods (since LAL is C99, we don't have C++ operators available).  Many
  /// SWIG language modules will automatically map these functions to
  /// scripting-language operations in their runtime code. In some cases
  /// (ironically, Python itself when using <tt>-builtin</tt>!) additional
  /// directives are needed in the scripting-language-specific interface.

  /// </li><li>

  /// Return new ::LIGOTimeGPS which are the positive and negative values of
  /// <tt>$self</tt>.
  LIGOTimeGPS* __pos__() {
    return XLALINT8NSToGPS(%swiglal_new_LIGOTimeGPS(), +XLALGPSToINT8NS($self));
  }
  LIGOTimeGPS* __neg__() {
    return XLALINT8NSToGPS(%swiglal_new_LIGOTimeGPS(), -XLALGPSToINT8NS($self));
  }

  /// </li><li>

  /// Return a new ::LIGOTimeGPS which is the absolute value of <tt>$self</tt>.
  LIGOTimeGPS* __abs__() {
    return XLALINT8NSToGPS(%swiglal_new_LIGOTimeGPS(), llabs(XLALGPSToINT8NS($self)));
  }

  /// </li><li>

  /// Return whether a ::LIGOTimeGPS is non-zero.
  bool __nonzero__() {
    return $self->gpsSeconds || $self->gpsNanoSeconds;
  }

  /// </li><li>

  /// Return integer representations of the ::LIGOTimeGPS seconds.
  int __int__() {
    return $self->gpsSeconds;
  }
  long __long__() {
    return $self->gpsSeconds;
  }

  /// </li><li>

  /// Return a floating-point representation of a ::LIGOTimeGPS.
  double __float__() {
    return XLALGPSGetREAL8($self);
  }

  /// </li><li>

  /// Return a string representation of a ::LIGOTimeGPS. Because XLALGPSToStr()
  /// allocates a new string using LAL memory, %newobject is used to make SWIG
  /// use a 'newfree' typemap, where the string is freed; SWIG will have already
  /// copied it to a native scripting-language string to return as output.
  %newobject __str__;
  %typemap(newfree) char* __str__ "XLALFree($1);";
  char* __str__() {
    return XLALGPSToStr(NULL, $self);
  }
  %newobject __repr__;
  %typemap(newfree) char* __repr__ "XLALFree($1);";
  char* __repr__() {
    return XLALGPSToStr(NULL, $self);
  }

  /// </li><li>

  /// Return the hash value of a ::LIGOTimeGPS.
  long __hash__() {
    long hash = (long)$self->gpsSeconds ^ (long)$self->gpsNanoSeconds;
    return hash == -1 ? -2 : hash;
  }

  /// </li><li>

  /// Binary operators need only be implemented for two ::LIGOTimeGPS arguments;
  /// the specialised input typemaps defined above will then allow other
  /// suitable types to be substituted as arguments.

  /// </li><li>

  /// Return the addition of two ::LIGOTimeGPS.
  LIGOTimeGPS* __add__(LIGOTimeGPS* gps) {
    LIGOTimeGPS* retn = %swiglal_new_LIGOTimeGPS();
    *retn = *$self;
    return XLALGPSAddGPS(retn, gps);
  }
  LIGOTimeGPS* __radd__(LIGOTimeGPS* gps) {
    LIGOTimeGPS* retn = %swiglal_new_LIGOTimeGPS();
    *retn = *gps;
    return XLALGPSAddGPS(retn, $self);
  }

  /// </li><li>

  /// Return the subtraction of two ::LIGOTimeGPS.
  LIGOTimeGPS* __sub__(LIGOTimeGPS* gps) {
    LIGOTimeGPS* retn = %swiglal_new_LIGOTimeGPS();
    *retn = *$self;
    return XLALGPSSetREAL8(retn, XLALGPSDiff(retn, gps));
  }
  LIGOTimeGPS* __rsub__(LIGOTimeGPS* gps) {
    LIGOTimeGPS* retn = %swiglal_new_LIGOTimeGPS();
    *retn = *gps;
    return XLALGPSSetREAL8(retn, XLALGPSDiff(retn, $self));
  }

  /// </li><li>

  /// Return the multiplication of two ::LIGOTimeGPS.
  LIGOTimeGPS* __mul__(LIGOTimeGPS* gps) {
    LIGOTimeGPS* retn = %swiglal_new_LIGOTimeGPS();
    *retn = *$self;
    return XLALGPSMultiply(retn, XLALGPSGetREAL8(gps));
  }
  LIGOTimeGPS* __rmul__(LIGOTimeGPS* gps) {
    LIGOTimeGPS* retn = %swiglal_new_LIGOTimeGPS();
    *retn = *gps;
    return XLALGPSMultiply(retn, XLALGPSGetREAL8($self));
  }

  /// </li><li>

  /// Return the floating-point division of two ::LIGOTimeGPS.
  LIGOTimeGPS* __div__(LIGOTimeGPS* gps) {
    LIGOTimeGPS* retn = %swiglal_new_LIGOTimeGPS();
    *retn = *$self;
    return XLALGPSDivide(retn, XLALGPSGetREAL8(gps));
  }
  LIGOTimeGPS* __rdiv__(LIGOTimeGPS* gps) {
    LIGOTimeGPS* retn = %swiglal_new_LIGOTimeGPS();
    *retn = *gps;
    return XLALGPSDivide(retn, XLALGPSGetREAL8($self));
  }

  /// </li><li>

  /// Return the integer division of two ::LIGOTimeGPS.
  LIGOTimeGPS* __floordiv__(LIGOTimeGPS* gps) {
    LIGOTimeGPS* retn = %swiglal_new_LIGOTimeGPS();
    *retn = *$self;
    return XLALGPSSetREAL8(retn, floor(XLALGPSGetREAL8(XLALGPSDivide(retn, XLALGPSGetREAL8(gps)))));
  }
  LIGOTimeGPS* __rfloordiv__(LIGOTimeGPS* gps) {
    LIGOTimeGPS* retn = %swiglal_new_LIGOTimeGPS();
    *retn = *gps;
    return XLALGPSSetREAL8(retn, floor(XLALGPSGetREAL8(XLALGPSDivide(retn, XLALGPSGetREAL8($self)))));
  }

  /// </li><li>

  /// Return the modulus of two ::LIGOTimeGPS.
  LIGOTimeGPS* __mod__(LIGOTimeGPS* gps) {
    LIGOTimeGPS* retn = %swiglal_new_LIGOTimeGPS();
    *retn = *$self;
    return XLALGPSSetREAL8(retn, fmod(XLALGPSGetREAL8(retn), XLALGPSGetREAL8(gps)));
  }
  LIGOTimeGPS* __rmod__(LIGOTimeGPS* gps) {
    LIGOTimeGPS* retn = %swiglal_new_LIGOTimeGPS();
    *retn = *gps;
    return XLALGPSSetREAL8(retn, fmod(XLALGPSGetREAL8(retn), XLALGPSGetREAL8($self)));
  }

  /// </li><li>

  /// Comparison operators between two ::LIGOTimeGPS are generated by the
  /// following SWIG macro. NAME is the name of the Python operator and OP is
  /// the C operator. The correct comparison NAME is obtained by comparing the
  /// result of XLALGPSCmp() against zero using OP.
  %define %swiglal_LIGOTimeGPS_comparison_operator(NAME, OP)
    bool __##NAME##__(LIGOTimeGPS *gps) {
      return XLALGPSCmp($self, gps) OP 0;
    }
  %enddef
  %swiglal_LIGOTimeGPS_comparison_operator(lt, < );
  %swiglal_LIGOTimeGPS_comparison_operator(le, <=);
  %swiglal_LIGOTimeGPS_comparison_operator(eq, ==);
  %swiglal_LIGOTimeGPS_comparison_operator(ne, !=);
  %swiglal_LIGOTimeGPS_comparison_operator(gt, > );
  %swiglal_LIGOTimeGPS_comparison_operator(ge, >=);

  /// </li><li>

  /// Return the number of nanoseconds in a ::LIGOTimeGPS.
  INT8 ns() {
    return XLALGPSToINT8NS($self);
  }

  /// </li></ul>
}
///

///
/// # Specialised wrapping of ::LALUnit
///

///
/// Allocate a new ::LALUnit.
///
#define %swiglal_new_LALUnit() (LALUnit*)(XLALCalloc(1, sizeof(LALUnit)))

///
/// Extend the ::LALUnit class.
%extend tagLALUnit {
  /// <ul><li>

  /// Construct a new ::LALUnit from another ::LALUnit.
  tagLALUnit(const LALUnit* unit) {
    return (LALUnit*)memcpy(%swiglal_new_LALUnit(), unit, sizeof(*unit));
  }

  /// </li><li>

  /// Construct a new ::LALUnit class from a string.
  tagLALUnit(const char* str) {
    LALUnit* unit = %swiglal_new_LALUnit();
    if (XLALParseUnitString(unit, str) == NULL) {
      XLALFree(unit);
      xlalErrno = XLAL_EFUNC; /* Silently signal an error to constructor */
      return NULL;
    }
    return unit;
  }

  /// </li><li>

  /// Return whether a ::LALUnit is non-zero, i.e. dimensionless.
  bool __nonzero__() {
    return !XLALUnitIsDimensionless($self);
  }

  /// </li><li>

  /// Return integer representations of a ::LALUnit.
  int __int__() {
    return (int)XLALUnitPrefactor($self);
  }
  long __long__() {
    return (long)XLALUnitPrefactor($self);
  }

  /// </li><li>

  /// Return a floating-point representation of a ::LALUnit.
  double __float__() {
    return XLALUnitPrefactor($self);
  }

  /// </li><li>

  /// Return a string representation of a ::LALUnit. Because
  /// XLALUnitToString() allocates a new string using LAL memory,
  /// %newobject is used to make SWIG use a 'newfree' typemap,
  /// where the string is freed; SWIG will have already copied it
  /// to a native scripting-language string to return as output.
  %newobject __str__;
  %typemap(newfree) char* __str__ "XLALFree($1);";
  char* __str__() {
    LALUnit norm = *$self;
    assert(XLALUnitNormalize(&norm) == XLAL_SUCCESS);
    return XLALUnitToString(&norm);
  }
  %newobject __repr__;
  %typemap(newfree) char* __repr__ "XLALFree($1);";
  char* __repr__() {
    LALUnit norm = *$self;
    assert(XLALUnitNormalize(&norm) == XLAL_SUCCESS);
    return XLALUnitToString(&norm);
  }

  /// </li><li>

  /// Return the hash value of a ::LALUnit.
  long __hash__() {
    long hash = (long)$self->powerOfTen;
    for (size_t i = 0; i < LALNumUnits; ++i) {
      hash ^= (long)$self->unitNumerator;
      hash ^= (long)$self->unitDenominatorMinusOne;
    }
    return hash == -1 ? -2 : hash;
  }

  /// </li><li>

  /// Return the integer exponentiation of a ::LALUnit.
  LALUnit* __pow__(INT2 n, void* SWIGLAL_OP_POW_3RDARG) {
    LALUnit* retn = %swiglal_new_LALUnit();
    return XLALUnitRaiseINT2(retn, $self, n);
  }

  /// </li><li>

  /// Return the rational exponentiation of a ::LALUnit.
  LALUnit* __pow__(INT2 r[2], void* SWIGLAL_OP_POW_3RDARG) {
    if (r[1] == 0) {
      xlalErrno = XLAL_EDOM; /* Silently signal an error to caller */
      return NULL;
    }
    RAT4 rat;
    rat.numerator = (r[1] < 0) ? -r[0] : r[0];
    rat.denominatorMinusOne = abs(r[1]) - 1;
    LALUnit* retn = %swiglal_new_LALUnit();
    return XLALUnitRaiseRAT4(retn, $self, &rat);
  }

  /// </li><li>

  /// Return the multiplication of two ::LALUnit.
  LALUnit* __mul__(LALUnit* unit) {
    LALUnit* retn = %swiglal_new_LALUnit();
    return XLALUnitMultiply(retn, $self, unit);
  }
  LALUnit* __rmul__(LALUnit* unit) {
    LALUnit* retn = %swiglal_new_LALUnit();
    return XLALUnitMultiply(retn, unit, $self);
  }

  /// </li><li>

  /// Return the division of two ::LALUnit.
  LALUnit* __div__(LALUnit* unit) {
    LALUnit* retn = %swiglal_new_LALUnit();
    return XLALUnitDivide(retn, $self, unit);
  }
  LALUnit* __rdiv__(LALUnit* unit) {
    LALUnit* retn = %swiglal_new_LALUnit();
    return XLALUnitDivide(retn, unit, $self);
  }

  /// </li><li>

  /// Comparison operators between two ::LALUnit.
  bool __eq__(LALUnit* unit) {
    return XLALUnitCompare($self, unit) == 0;
  }
  bool __ne__(LALUnit* unit) {
    return XLALUnitCompare($self, unit) != 0;
  }

  /// </li><li>

  /// Return a normalised ::LALUnit.
  %newobject norm;
  LALUnit* norm() {
    LALUnit* retn = %swiglal_new_LALUnit();
    *retn = *$self;
    assert(XLALUnitNormalize(retn) == XLAL_SUCCESS);
    return retn;
  }

  /// </li></ul>
}
///

// Local Variables:
// mode: c
// End: