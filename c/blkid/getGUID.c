#include <stdio.h>
#include <string.h>
#include <err.h>
#include <blkid/blkid.h>


int main (int argc, char *argv[]) {
   blkid_probe pr;
   if (argc != 2) {
    fprintf(stderr, "Usage: %s devname\n", argv[0]);
    exit(1);
  }

   pr = blkid_new_probe_from_filename(argv[1]);
   if (!pr) {
    err(2, "Failed to open %s", argv[1]);
  }
   blkid_partlist ls;
   int nparts, i;

   ls = blkid_probe_get_partitions(pr);
   nparts = blkid_partlist_numof_partitions(ls);
   printf("Number of partitions:%d\n", nparts);

   if (nparts <= 0){
      printf("Please enter correct device name! e.g. \"/dev/sdc\"\n");
      return;
   }

   const char *guid;

   for (i = 0; i < nparts; i++) {

	   blkid_partition par = blkid_partlist_get_partition(ls, i);
	   guid = blkid_partition_get_type_string(par);
     	   printf("GUID=%s\n", guid);
   }

   blkid_free_probe(pr);

   return 0;
}
