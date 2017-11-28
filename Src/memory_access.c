#include "memory_access.h"

FATFS *fs;
TCHAR *current_dir;

int _is_full_path(TCHAR *path) {
	if(path[0] == '/') {
		return 1;
	}
	return 0;
}

TCHAR *_get_full_path(TCHAR *path) {
	TCHAR *new_path = malloc(sizeof(TCHAR)*MAX_PATH);

	if(_is_full_path(path)) {
		strcpy(new_path, USBHPath);
		strcat(new_path, path[1]);
	} else {
		strcpy(new_path, current_dir);
		strcat(new_path, "/");
		strcat(new_path, path);
	}
	return new_path;
}

int mount_usb() {
	fs = malloc(sizeof(FATFS));
	if(f_mount(fs, USBHPath, 1) != FR_OK) {
		printf("usb mount error\r\n");
		free(fs);
		return -1;
	}

	current_dir = malloc(sizeof(TCHAR)*MAX_PATH);
	strcpy(current_dir, USBHPath);

	printf("Mounted USB\r\n");
	return 1;
}

int unmount_usb() {
	if(f_mount(0, USBHPath, 0) != FR_OK) {
		printf("usb unmount error\r\n");
		return -1;
	}
	free(current_dir);
	free(fs);

	printf("Unmounted USB\r\n");
	return 1;
}

TCHAR *get_current_directory() {
	return current_dir;
}

TCHAR *change_directory(TCHAR *path) {
	TCHAR *full_path = _get_full_path(path);

	DIR dir;
	if(f_opendir(&dir, full_path) != FR_OK) {
		printf("cannot change directory: %s\n", full_path);
		free(full_path);
		return NULL;
	}

	f_closedir(&dir);
	free(current_dir);
	current_dir = full_path;
	return current_dir;
}

TCHAR *list_directory() {
	DIR dir;
	if(f_opendir(&dir, current_dir) != FR_OK) {
		printf("cannot open directory %s\r\n", current_dir);
		return NULL;
	}

	TCHAR *list = malloc(sizeof(TCHAR)*MAX_LIST);
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
	TCHAR *path = _get_full_path(filename);
		FIL *file = malloc(sizeof(FIL));

		if(f_open(file, path, FA_OPEN_EXISTING | FA_READ) != FR_OK ) {
			printf("cannot open file %s\r\n", path);
			free(path);
			free(file);
			return NULL;
		}
		free(path);
		return file;
}


FIL *create_file(char *filename) {
	TCHAR *path = _get_full_path(filename);
	FIL *file = malloc(sizeof(FIL));

	if(f_open(file, path, FA_CREATE_NEW | FA_WRITE) != FR_OK ) {
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

int write_to_file(FIL *file, char *buf, int size) {
	int bw;
	if(f_write(file, buf, size, &bw) != FR_OK) {
		printf("write to file error\r\n");
		return -1;
	}
	return bw;
}

int read_file(FIL *file, char *buf, int size) {
	int br;
	if(f_read(file, buf, size, &br) != FR_OK) {
		printf("read file error\r\n");
		return -1;
	}
	return br;
}

int delete_file(TCHAR *filename) {
	TCHAR *full_path = _get_full_path(filename);

	if(f_unlink(full_path) != FR_OK) {
		printf("cannot delete file: %s\r\n", full_path);
		free(full_path);
		return -1;
	}
	
	printf("deleted file: %s\r\n", full_path);
	free(full_path);
	return 1;
}

