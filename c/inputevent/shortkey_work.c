#include <termio.h>
#include <stdio.h>
#include <linux/input.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <string.h>

#define KEY_PRINTSCREEN 99
#define DEVICE_PATH "/dev/input/"

char* get_device_file(){
        FILE* fp = popen("ls /dev/input/by-id/ -l|grep event-kbd|head -n1|cut -d'/' -f2", "r");
        char buffer[1024];
        char *fullfile;
	if ((fullfile = malloc(sizeof(buffer))) == NULL){
		printf("malloc error\n");
        	return -1;
	}
	memset(buffer, 0, sizeof(buffer));
	memset(fullfile, 0, sizeof(buffer));
	strcpy(fullfile, DEVICE_PATH);
        int bytes_read = fread(buffer, 1, sizeof(buffer), fp);
        pclose(fp);
        if(bytes_read ==0){
                printf("read error! \n");
                return -2;
        }
        else{
                buffer[ bytes_read - 1] = '\0';
		strcat(fullfile, buffer);
                printf("the device is %s\n", fullfile);
		return fullfile;
        }
}

int main ()
{
  int keys_fd;
  char ret[2];
  struct input_event t;
  char *device_file = get_device_file();

  keys_fd = open ( device_file, O_RDONLY);
  if (keys_fd <= 0)
    {
      printf ("open %s device error!\n", device_file);
      return 0;
    }

  while (1)
    {
      if (read (keys_fd, &t, sizeof (t)) == sizeof (t))
	{
	  if (t.type == EV_KEY)
	    if (t.value == 0 || t.value == 1)
	      {
		printf ("key %d %s\n", t.code,
			(t.value) ? "Pressed" : "Released");
		//if(t.code==KEY_ESC)
		//break;
		if (t.code == KEY_PRINTSCREEN && t.value == 1)
		  system ("myscreenshot &");
	      }
	}
    }
  close (keys_fd);
  free(device_file);

  return 0;
}
