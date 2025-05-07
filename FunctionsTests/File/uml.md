+-------------------------+
|         mg_fs           | <<Interface>>
+-------------------------+
| + int (*st)(...)        |
| + void (*ls)(...)       |
| + void *(*op)(...)      |
| + void (*cl)(...)       |
| + size_t (*rd)(...)     |
| + size_t (*wr)(...)     |
| + size_t (*sk)(...)     |
| + bool (*mv)(...)       |
| + bool (*rm)(...)       |
| + bool (*mkd)(...)      |
+-------------------------+
           ▲
           | implements
+-------------------------+
|      mg_fs_win          | <<Instance>>
+-------------------------+
| + win_stat()            |
| + win_ls()              |
| + win_open()            |
| + win_close()           |
| + win_read()            |
| + win_write()           |
| + win_seek()            |
| + win_rename()          |
| + win_remove()          |
| + win_mkdir()           |
+-------------------------+


+-------------------------+
|         mg_fd           |
+-------------------------+
| + void *fd              |
| + mg_fs *fs             |
+-------------------------+

<<Uses>>
mg_fs_open()
mg_fs_close()


+-------------------------+
|         mg_str          |
+-------------------------+
| + char *buf             |
| + size_t len            |
+-------------------------+

<<Uses>>
mg_file_read()


+-------------------------+
|   File Operations (Helpers) |
+-------------------------+
| + mg_fs_open()          |
| + mg_fs_close()         |
| + mg_file_read()        |
| + mg_file_write()       |
| + mg_file_printf()      |
| + mg_fs_ls()            |
+-------------------------+

<<Main program>>
+-------------------------+
|         main            |
+-------------------------+
| + Test write (mg_file_printf) |
| + Test read (mg_file_read) |
| + Test list (mg_fs_ls) |
| + Test remove (mg_fs_win.rm) |
+-------------------------+
mg_fs → Abstract interface for filesystem drivers (virtual functions)

mg_fs_win → Implements mg_fs with Windows API (FindFirstFile, fopen, remove, etc)

mg_fd → File descriptor wrapper, holds void* native fd and back-pointer to mg_fs

mg_str → Used for file read result (buffer + length)

File Operations Helpers → Functions that use mg_fs (read, write, printf, list, remove)


[ main() ]
     |
[ File Helpers (mg_file_printf, read, write, ls) ]
     |
[ mg_fs interface ]<----->[ mg_fs_win implementation ]
     |
[ mg_fd (open file handle) ]
