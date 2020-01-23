/*

  file reading

 */

#include <stdio.h>
#include <string.h>
#include "diskimage.h"


static char usage[] = "test_read <diskimage> <filename>";


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
  ImageFile *infile;
  FILE *outfile;
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

  /* Open file for reading */
  if ((infile = di_open(di, rawname, T_PRG, "rb")) == NULL) {
    puts("Couldn't open file for reading");
    goto CloseImage;
  }

  /* Open file for writing */
  if ((outfile = fopen(argv[2], "wb")) == NULL) {
    puts("Couldn't open file for writing");
    di_close(infile);
    goto CloseImage;
  }

  while ((len = di_read(infile, buffer, 4096)) > 0) {
    if (fwrite(buffer, 1, len, outfile) != len) {
      perror("Write error");
      goto CloseFiles;
    }
    size += len;
  }
  printf("Read %d bytes from %s\n", size, argv[1]);

 CloseFiles:
  /* Close file */
  fclose(outfile);
  di_close(infile);

 CloseImage:
  /* Release image */
  di_free_image(di);

  /* Done */
  return(0);
}
