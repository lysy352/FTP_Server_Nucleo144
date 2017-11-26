/*
 * memory_access.h
 *
 *  Created on: 23.11.2017
 *      Author: Dawid
 */

#ifndef MEMORY_ACCESS_H_
#define MEMORY_ACCESS_H_


#include "fatfs.h"
#include "ff.h"

int mount_usb();

int unmount_usb();

TCHAR *get_current_directory();

TCHAR *change_directory(TCHAR *path) ;

TCHAR *list_directory() ;

FIL *open_file(char *filename);


FIL *create_file(char *filename) ;

void close_file(FIL *file) ;
int write_to_file(FIL *file, char *buf, int size);

int read_file(FIL *file, char *buf, int size) ;

int delete_file(TCHAR *filename);

#define MAX_PATH 128
#define MAX_LIST 1024


#endif /* MEMORY_ACCESS_H_ */
