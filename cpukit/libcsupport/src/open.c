/**
 *  POSIX 1003.1 5.3.1 - Open a File
 */
int open(const char *path, int oflag, ...)
{
    int rv = 0;
    va_list ap;
    mode_t mode = 0;
    rtems_libio_t *iop = NULL;

    // 处理可变参数，获取文件创建模式（mode）。
    va_start(ap, oflag);
    mode = va_arg(ap, mode_t);

    // 分配一个文件描述符结构。
    iop = rtems_libio_allocate();
    if (iop != NULL)
    {
        // 调用底层实现打开文件。
        rv = do_open(iop, path, oflag, mode);
    }
    else
    {
        // 文件描述符耗尽，设置错误码。
        errno = ENFILE;
        rv = -1;
    }

    va_end(ap);

    return rv;
}
