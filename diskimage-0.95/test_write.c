/*

  file writing

 */

#include <stdio.h>
#include <string.h>
#include "diskimage.h"


static char usage[] = "test_write <diskimage> <filename>";


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
  DiskImage *di;
  unsigned char buffer[4096];
  FILE *infile;
  ImageFile *outfile;
  int len;
  unsigned char rawname[16];
  char name[17];
  int size = 0;

  /* Check usage */
  if (argc != 3) {
    puts(usage);
    return(1);
  }

  /* Load image into ram */
  if ((di = di_load_image(argv[1])) == NULL) {
    puts("di_load_image failed");
    return(1);
  }

  /* Convert filename */
  strncpy(name, argv[2], 16);
  name[16] = 0;
  atop(name);
  di_rawname_from_name(rawname, name);

  /* Open file for writing */
  if ((outfile = di_open(di, rawname, T_PRG, "wb")) == NULL) {
    puts("Couldn't open file for writing");
    goto CloseImage;
  }

  /* Open file for reading */
  if ((infile = fopen(argv[2], "rb")) == NULL) {
    puts("Couldn't open file for reading");
    di_close(outfile);
    goto CloseImage;
  }

  while ((len = fread(buffer, 1, 4096, infile)) > 0) {
    if (di_write(outfile, buffer, len) != len) {
      perror("Write error");
      goto CloseFiles;
    }
    size += len;
  }
  printf("Wrote %d bytes to %s\n", size, argv[1]);

 CloseFiles:
  /* Close file */
  fclose(infile);
  di_close(outfile);

 CloseImage:
  /* Release image */
  di_free_image(di);

  /* Done */
  return(0);
}
