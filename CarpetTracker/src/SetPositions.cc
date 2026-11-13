#include <cassert>
#include <sstream>
#include <string>

#include "cctk.h"
#include "cctk_Arguments.h"
#include "cctk_Functions.h"
#include "cctk_Parameters.h"

namespace CarpetTracker {

using namespace std;

// Maximum number of tracked surfaces
int const num_surfaces = 10;

extern "C" {
void CarpetTracker_SetPositions(CCTK_ARGUMENTS);
}

void CarpetTracker_SetPositions(CCTK_ARGUMENTS) {
  DECLARE_CCTK_ARGUMENTS_CarpetTracker_SetPositions;
  DECLARE_CCTK_PARAMETERS;

  static int cctk_iteration_done = -1;

  if (cctk_iteration == cctk_iteration_done)
    return;
  cctk_iteration_done = cctk_iteration;

  for (int n = 0; n < num_surfaces; ++n) {
    int const sn = sf_IdFromName(surface[n], surface_name[n]);
    if (sn >= 0) {
      assert(sn >= 0 and sn < nsurfaces);

      if (sf_active[sn]) {
        /*
         * Allow *active* surfaces with sf_valid == 0 to be tracked,
         * so that thorns can use this value to move the centroid of a surface
         * without setting other properties that sf_valid > 0 implies.
         * Check that the centroid is at least finite in that case, since
         * if sf_valid == 0 for other reasons, it may contain stale values.
         */
        if (sf_valid[sn] > 0 ||
            (sf_valid[sn] == 0 && 
            isfinite(sf_centroid_x[sn]) && 
            isfinite(sf_centroid_y[sn]) && 
            isfinite(sf_centroid_z[sn]))) {

          if (verbose) {
            CCTK_VInfo(CCTK_THORNSTRING, "Setting position of refined region "
                                         "#%d from surface #%d to (%g,%g,%g)",
                       n + 1, sn, static_cast<double>(sf_centroid_x[sn]),
                       static_cast<double>(sf_centroid_y[sn]),
                       static_cast<double>(sf_centroid_z[sn]));
          }

          // Activate region
          active[n] = 1;

          // Set position in CarpetRegrid2
          position_x[n] = sf_centroid_x[sn];
          position_y[n] = sf_centroid_y[sn];
          position_z[n] = sf_centroid_z[sn];

        } else { // sf_valid < 0 || sf_valid == 0 but centroid not finite

          if (verbose) {
            CCTK_VInfo(CCTK_THORNSTRING, "No position information available "
                                         "(invalid or non-finite centroid) "
                                         "for refined region #%d from surface "
                                         "#%d",
                       n + 1, sn);
          }

        } // if not valid

      } else {

        if (verbose) {
          CCTK_VInfo(
              CCTK_THORNSTRING,
              "Refined region #%d (depending on surface #%d) is inactive",
              n + 1, sn);
        }

        // Deactivate region
        active[n] = 0;

      } // if not active

    } // if sn > 0
  }   // for n
}

} // namespace CarpetTracker
