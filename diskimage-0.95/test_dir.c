/*

  directory listing

 */

#include <stdio.h>
#include <string.h>
#include "diskimage.h"

static char *ftype[] = {
  "del",
  "seq",
  "prg",
  "usr",
  "rel",
  "cbm",
  "dir",
  "???"
};

static char usage[] = "test_dir <diskimage>";


void ptoa(unsigned char *s) {
  unsigned char c;

  while ((c = *s)) {
    c &= 0x7f;
    if (c >= 'A' && c <= 'Z') {
      c += 32;
    } else if (c >= 'a' && c <= 'z') {
      c -= 32;
    } else if (c == 0x7f) {
      c = 0x3f;
    }
    *s++ = c;
  }
}


int main(int argc, char *argv[]) {
  DiskImage *di;
  unsigned char buffer[254];
  ImageFile *dh;
  int offset;
  char quotename[19];
  char name[17];
  char id[6];
  int type;
  int closed;
  int locked;
  int size;

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

  /* Open directory for reading */
  if ((dh = di_open(di, "$", T_PRG, "rb")) == NULL) {
    puts("Couldn't open directory");
    goto CloseImage;
  }

  /* Convert title to ascii */
  di_name_from_rawname(name, di_title(di));
  ptoa(name);

  /* Convert ID to ascii */
  memcpy(id, di_title(di) + 18, 5);
  id[5] = 0;
  ptoa(id);

  /* Print title and disk ID */
  printf("0 \"%-16s\" %s\n", name, id);

  /* Read first block into buffer */
  if (di_read(dh, buffer, 254) != 254) {
    printf("BAM read failed\n");
    goto CloseDir;
  }

  /* Read directory blocks */
  while (di_read(dh, buffer, 254) == 254) {
    for (offset = -2; offset < 254; offset += 32) {

      /* If file type != 0 */
      if (buffer[offset+2]) {

	di_name_from_rawname(name, buffer + offset + 5);
	type = buffer[offset + 2] & 7;
	closed = buffer[offset + 2] & 0x80;
	locked = buffer[offset + 2] & 0x40;
	size = buffer[offset + 31]<<8 | buffer[offset + 30];

	/* Convert to ascii and add quotes */
	ptoa(name);
	sprintf(quotename, "\"%s\"", name);

	/* Print directory entry */
	printf("%-4d  %-18s%c%s%c\n", size, quotename, closed ? ' ' : '*', ftype[type], locked ? '<' : ' ');
      }
    }
  }

  /* Print number of blocks free */
  printf("%d blocks free\n", di->blocksfree);

 CloseDir:
  /* Close file */
  di_close(dh);

 CloseImage:
  /* Release image */
  di_free_image(di);

  /* Done */
  return(0);
}
