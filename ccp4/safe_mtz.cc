
// Modified version of phaser\codebase\phaser\lib\safe_mtz.cc
// for reading column data into array of floats

#include "safe_mtz.h"
#include "cmtzlib.h"
#include "mtzdata.h"



int MtzReadRefl(CMtz::MTZ *mtz, std::vector< std::vector<float> > &columns, int logmss[], 
  CMtz::MTZCOL *lookup[],  int ncols, int nrefs) 
{
  int icol(0);
  unsigned int colin(0);
  float refldata[MCOLUMNS];
  union float_uint_uchar uf;

  columns.clear();
  columns.resize(ncols);
  for (int icol = 0; icol < ncols; icol++)
  {
    columns[icol].resize(nrefs);
  }

  for (int iref = 0; iref < nrefs; iref++)
  {
    /* If we are past the last reflection, indicate this with return value. */
    if (iref > mtz->nref_filein || iref <= 0)
      continue;

    /* If reflections not in memory, read next record from file. */
    if (!mtz->refs_in_memory) 
    {
      if (CMtz::MtzRrefl(mtz->filein, mtz->ncol_read, refldata) == EOF) return 1;
    }

    if (strncmp(mtz->mnf.amnf, "NAN", 3) == 0) 
    {
      uf = CCP4::ccp4_nan();
    }
    else {
      uf.f = mtz->mnf.fmnf;
    }

    /* loop over columns requested in lookup array. */
    for (icol = 0; icol < ncols; icol++) 
    {
      logmss[icol] = 1;
      if (lookup[icol]) {
        if (mtz->refs_in_memory) 
        {
          columns[icol][iref] = lookup[icol]->ref[iref - 1];
          logmss[icol] = CMtz::ccp4_ismnf(mtz, columns[icol][iref]);
        }
        else 
        {
          colin = lookup[icol]->source;
          if (colin) 
          {
            columns[icol][iref] = refldata[colin - 1];
            logmss[icol] = CMtz::ccp4_ismnf(mtz, columns[icol][iref]);
          }
          else 
          {
            columns[icol][iref] = uf.f;
            logmss[icol] = 1;
          }
        }
      }
    }
  }

  return 0;
}

