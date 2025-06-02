/**
 *  POSIX 1003.1b 6.4.1 - Read From a File
 */
ssize_t read(
    int fd,       // 文件描述符，标识要读取的文件或设备。
    void *buffer, // 指向用户提供的内存缓冲区。
    size_t count  // 期望读取的字节数。
)
{
    // 指向文件描述符对应的 I/O 对象结构体。
    rtems_libio_t *iop;

    // 实际读取的字节数或错误代码。
    ssize_t n;

    // 检查 buffer 是否为 NULL，防止非法内存访问。
    rtems_libio_check_buffer(buffer);

    // 检查读取字节数是否为 0 或超出合理范围。
    rtems_libio_check_count(count);

    /*
     * 获取对应的 I/O 对象，并检查是否具有可读权限。
     * 若无效或不可读，则自动设置 errno = EBADF 并返回 -1。
     */
    LIBIO_GET_IOP_WITH_ACCESS(fd, iop, LIBIO_FLAGS_READ, EBADF);

    /*
     * 正式执行读取操作：
     * 调用底层文件系统或设备驱动提供的 read 函数。
     * 由 handlers->read_h 函数指针调用完成具体的读取逻辑。
     */
    n = (*iop->pathinfo.handlers->read_h)(iop, buffer, count);

    // 读取完成后释放 I/O 对象（减少引用计数等）。
    rtems_libio_iop_drop(iop);

    // 返回实际读取的字节数，若出错则为负值并设置 errno。
    return n;
}
