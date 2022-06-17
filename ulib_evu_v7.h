#ifndef ULIB_EVU_V7_H_
#define ULIB_EVU_V7_H_

#define VRTDEVICE_FILENAME "/dev/eduv7-0"

/*our file operations supported by our driver
.open = eduv7cdev_open,
.release = eduv7cdev_release,
.read = eduv7cdev_read,
.write = eduv7cdev_write
};
*/

/*Open the device*/
int eduv7_open(void);

int eduv7_read(void* dst, int32_t size);
int eduv7_write(void* data, int32_t size);
int eduv7_close(void);

#endif // ULIB_EVU_V7_H_
