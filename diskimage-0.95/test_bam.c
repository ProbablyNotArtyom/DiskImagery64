/*

  BAM printout

 */

#include <stdio.h>
#include "diskimage.h"


static char usage[] = "test_bam <diskimage>";


int main(int argc, char *argv[]) {
  DiskImage *di;
  TrackSector ts;

  /* Check usage */
  if (argc != 2) {
    puts(usage);
    return(1);
  }

  /* Load image into ram */
  if ((di = di_load_image(argv[1])) == NULL) {
    puts("di_load_image failed");
    return(1);
  }

  /* Print BAM */
  puts("TRK  FREE  MAP");
  for (ts.track = 1; ts.track <= di_tracks(di->type); ++ts.track) {
    printf("%3d: %2d/%d ", ts.track, di_track_blocks_free(di, ts.track), di_sectors_per_track(di->type, ts.track));
    for (ts.sector = 0; ts.sector < di_sectors_per_track(di->type, ts.track); ++ts.sector) {
      printf("%d", di_is_ts_free(di, ts));
    }
    puts("");
  }
  puts("");

  /* Print number of blocks free */
  printf("%d blocks free\n", di->blocksfree);

  /* Release image */
  di_free_image(di);

  /* Done */
  return(0);
}
