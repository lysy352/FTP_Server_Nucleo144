#include "test.h"
#include "memory_access.h"
#include "ftp_server.h"
#include "fatfs.h"

void test() {
	printf("TEST BEGIN\r\n");


	printf("%s\n", USBHPath);
	FATFS *fs;

	fs = malloc(sizeof(FATFS));
	if(f_mount(fs, USBHPath, 1) != FR_OK) {
		printf("usb mount error\r\n");
		free(fs);
		return;
	}  

	mount_usb();
	printf("%s\r\n", get_current_directory());
	printf("%s\r\n", list_directory());

	FIL *f = open_file("/123.txt");

	uint16_t buf_size = 1024;
	char *buf = malloc(sizeof(char)*buf_size);
	uint16_t rb = read_file(f, buf, buf_size);
	printf("Read %u\r\n%s\r\n", rb, buf);
	free(buf);

	unmount_usb();
	printf("TEST END\r\n");
}
