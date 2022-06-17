#include <stdlib.h>
#include <stdio.h>
#include "ulib_evu_v7.h"
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

static int virtual_device_fd = -1;
int eduv7_open(void)
{

    virtual_device_fd = open(VRTDEVICE_FILENAME, O_RDWR);
    if (virtual_device_fd < 0){
        printf("Can't open device file: %s\n",
            VRTDEVICE_FILENAME);
        exit(1);
    }
}
/*buf must be allocated with count size*/
int eduv7_read(void* dst, int32_t size)
{
    assert(dst);
    assert(size);
    if(read(virtual_device_fd, dst, size) < 0){
        printf("read went wrong with this read 0x%08x\n",*(int*)dst);
    }
}

int eduv7_write(void* data, int32_t size)
{
    assert(data);
    assert(size);
    if(write(virtual_device_fd, data,size) < 0){
        printf("write went wrong with this write 0x%08x\n",*(int*)data);
    }
}

int eduv7_close(void)
{
    close(virtual_device_fd);
}
