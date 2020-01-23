/*

  format a disk

 */

#include <stdio.h>
#include <string.h>
#include "diskimage.h"


static char usage[] = "test_format <diskimage> <name> [id]";


void atop(unsigned char *s) {
  unsigned char c;

  while ((c = *s)) {
    c &= 0x7f;
    if (c >= 'A' && c <= 'Z') {
      c += 32;
    } else if (c >= 'a' && c <= 'z') {
      c -= 32;
    }
    *s++ = c;
  }
}


int main(int argc, char *argv[]) {
  unsigned char name[17];
  unsigned char rawname[16];
  unsigned char rawid[3];
  DiskImage *di;
  TrackSector ts;
  int size = 0;
  int l;

  /* Check usage */
  if (argc < 3 || argc > 4) {
    puts(usage);
    return(1);
  }

  /* Guess the image type */
  if ((l = strlen(argv[1])) > 4) {
    if ((argv[1][l - 4] == '.') && ((argv[1][l - 3] & 0xdf) == 'D')) {
      switch (argv[1][l - 2]) {
      case '6': // d64
	size = 174848;
	break;
      case '7': // d71
	size = 349696;
	break;
      case '8': //d81
	size = 819200;
	break;
      default:
	puts("unknown image type");
	return(1);
      }
    }
  }

  /* Load image into ram */
  if ((di = di_load_image(argv[1])) == NULL) {
    if ((di = di_create_image(argv[1], size)) == NULL) {
      puts("di_create_image failed");
      return(1);
    }
  }

  /* Convert title */
  strncpy(name, argv[2], 16);
  name[16] = 0;
  atop(name);
  di_rawname_from_name(rawname, name);

  /* Convert ID, if present */
  if (argc == 4) {
    strncpy(rawid, argv[3], 2);
    rawid[2] = 0;
    atop(rawid);
    di_format(di, rawname, rawid);
  } else {
    di_format(di, rawname, NULL);
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
