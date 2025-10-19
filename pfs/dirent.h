// dirent.h - Direcctory routines

#ifndef PFS_DIRENT_H
#define PFS_DIRENT_H

#include <sys/types.h>

#ifndef FF_DEFINED
// fatfs has its own definition of DIR
#define DIR void
#endif

struct dirent {
    ino_t          d_ino;       /* Inode number */
    off_t          d_off;       /* Not an offset */
    unsigned short d_reclen;    /* Length of this record */
    unsigned char  d_type;      /* Type of file; not supported by all filesystem types */
    char           d_name[256]; /* Null-terminated filename */
};

/* File types for `d_type'.  */
enum
  {
    DT_UNKNOWN = 0,
# define DT_UNKNOWN	DT_UNKNOWN
    DT_FIFO = 1,
# define DT_FIFO	DT_FIFO
    DT_CHR = 2,
# define DT_CHR		DT_CHR
    DT_DIR = 4,
# define DT_DIR		DT_DIR
    DT_BLK = 6,
# define DT_BLK		DT_BLK
    DT_REG = 8,
# define DT_REG		DT_REG
    DT_LNK = 10,
# define DT_LNK		DT_LNK
    DT_SOCK = 12,
# define DT_SOCK	DT_SOCK
    DT_WHT = 14
# define DT_WHT		DT_WHT
  };

#ifdef __cplusplus
extern "C" {
#endif

DIR *opendir (const char *name);
struct dirent *readdir (DIR *dirp);
int closedir (DIR *dirp);

#ifdef __cplusplus
}
#endif

#endif
