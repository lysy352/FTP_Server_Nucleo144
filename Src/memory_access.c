#include "memory_access.h"

FATFS *fs;
char *current_dir;

int _is_full_path(char *path) {
	if(path[0] == '/' ) {
		return 1;
	}
	return 0;
}

char *_get_full_path(char *path) {
	char *new_path;
	if(_is_full_path(path)) {
		new_path = malloc(sizeof(char)*strlen(path) + 2);
		sprintf(new_path, "%s/", path);
	} else {
		new_path = malloc(sizeof(char)*(strlen(current_dir) + strlen(path) + 2));
		sprintf(new_path, "%s%s/", current_dir, path);
	}
	return new_path;
}

int mount_usb() {
	fs = malloc(sizeof(FATFS));

	if(f_mount(fs, "", 1) != FR_OK) {
		printf("usb mount error\r\n");
		free(fs);
		return -1;
	}

	current_dir = malloc(sizeof(char)*2);
	strcpy(current_dir, "/");

	printf("mounted USB\r\n");
	return 1;
}

int unmount_usb() {
	if(f_mount(0, "", 0) != FR_OK) {
		printf("usb unmount error\r\n");
		return -1;
	}
	free(current_dir);
	free(fs);

	printf("unmounted USB\r\n");
	return 1;
}

char *get_current_directory() {
	return current_dir;
}

char *change_directory(char *path) {
	char *full_path = _get_full_path(path);

	DIR dir;
	if(f_opendir(&dir, full_path+1) != FR_OK) {
		printf("cannot change directory: %s\n", full_path);
		free(full_path);
		return NULL;
	}

	f_closedir(&dir);
	free(current_dir);
	current_dir = full_path;
	return current_dir;
}

char *list_directory() {
	DIR dir;
	if(f_opendir(&dir, current_dir+1) != FR_OK) {
		printf("cannot open directory %s\r\n", current_dir);
		return NULL;
	}

	char *list = malloc(sizeof(char)*MAX_LIST);
	list[0] = 0;
	FILINFO finfo;
	while(1) {
		if(f_readdir(&dir, &finfo) != FR_OK || finfo.fname[0] == 0)
			break;

		if(finfo.fattrib & AM_DIR )
			strcat(list, "+/");
		else {
			strcat(list, "+r,s");
			sprintf(list + strlen(list), finfo.fsize);
		}
		strcat(list,",\t");
		strcat(list, finfo.fname);
		strcat(list, "\r\n" );
	}
	f_closedir(&dir);
	return list;
}

FIL *open_file(char *filename) {
	char *path = _get_full_path(filename);
	FIL *file = malloc(sizeof(FIL));

	if(f_open(file, path+1, FA_OPEN_EXISTING | FA_READ) != FR_OK ) {
		printf("cannot open file %s\r\n", path);
		free(path);
		free(file);
		return NULL;
	}
	free(path);
	return file;
}

FIL *create_file(char *filename) {
	char *path = _get_full_path(filename);
	FIL *file = malloc(sizeof(FIL));

	if(f_open(file, path+1, FA_CREATE_NEW | FA_WRITE) != FR_OK ) {
		printf("cannot create file %s\r\n", path);
		free(path);
		free(file);
		return NULL;
	}
	free(path);
	return file;
}

void close_file(FIL *file) {
	f_close(file);
	free(file);
}

uint16_t write_to_file(FIL *file, char *buf, uint16_t size) {
	uint16_t bw;
	if(f_write(file, buf, size, &bw) != FR_OK) {
		printf("write to file error\r\n");
		return -1;
	}
	return bw;
}

uint16_t read_file(FIL *file, char *buf, uint16_t size) {
	uint16_t br;
	if(f_read(file, buf, size, &br) != FR_OK) {
		printf("read file error\r\n");
		return -1;
	}
	return br;
}

int delete_file(char *filename) {
	char *full_path = _get_full_path(filename);

	if(f_unlink(full_path+1) != FR_OK) {
		printf("cannot delete file: %s\r\n", full_path);
		free(full_path);
		return -1;
	}
	
	printf("deleted file: %s\r\n", full_path);
	free(full_path);
	return 1;
}

void USB_Process(ApplicationTypeDef Appli_state) {
	 switch(Appli_state) {
		 case APPLICATION_START:
			 printf("Device connected.\r\n");
			 break;
		 case APPLICATION_READY:
			 mount_usb();
			 printf("Device ready.\r\n");
			 break;
		 case APPLICATION_DISCONNECT:
			 unmount_usb();
			 printf("Device disconnected.\r\n");
			 break;
		 default:
		 break;
	}
}
