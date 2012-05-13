#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace std;

#define FILENAME_LEN_MAX   28
#define DIR_ENTRIES_MAX    128
#define OPEN_FILES_MAX     8
#define SECTOR_SIZE        512
#define DEVICE_SIZE_MAX   (1024 * 1048576)
#define DEVICE_SIZE_MIN   (   8 * 1048576)

struct TFile
{
	char m_FileName[FILENAME_LEN_MAX + 1];
	int m_FileSize;
};

struct TBlkDev
{
	int m_Sectors;
	int (*m_Read) (int, void *, int);
	int (*m_Write) (int, const void *, int);
};

int  FsCreate       (struct TBlkDev *dev);
int  FsMount        (struct TBlkDev *dev);
int  FsUmount       (void);

int  FileOpen       (const char *fileName, int writeMode);
int  FileRead       (int fd, void *buffer, int len);
int  FileWrite      (int fd, const void *buffer, int len);
int  FileClose      (int fd); 

int  FileDelete     (const char *fileName);

int  FileFindFirst  (struct TFile *info);
int  FileFindNext   (struct TFile *info);
int  FileSize       (const char *fileName);

