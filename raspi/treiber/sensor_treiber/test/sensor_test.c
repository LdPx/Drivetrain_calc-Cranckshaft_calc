#include <unistd.h>
#include <fcntl.h>       
#include <stdio.h>  
#include <time.h>     

#define SENSOR_FILE "/dev/sensor"


int main(int argc, char **argv){
	int sensor = open(SENSOR_FILE, O_RDONLY);
	if(sensor < 0){
		perror("sensor open()");
		return 1;
	}
	unsigned long dist;
	read(sensor, &dist, sizeof(dist));
	printf("read distance %lu\n", dist);
	return 0;
}

