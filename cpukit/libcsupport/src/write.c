/**
 *  POSIX 1003.1b 6.4.2 - Write to a File
 *
 *  This routine writes count bytes from from buffer pointed to by buffer
 *  to the file associated with the open file descriptor, fildes.
 */
ssize_t write(
    int fd,             // 文件描述符，表示要写入的目标文件或设备。
    const void *buffer, // 用户数据缓冲区的地址。
    size_t count        // 要写入的字节数。
)
{
    // 指向文件描述符关联的 I/O 对象。
    rtems_libio_t *iop;

    // 实际写入的字节数或错误码。
    ssize_t n;

    // 检查 buffer 是否为 NULL，防止非法内存访问。
    rtems_libio_check_buffer(buffer);

    // 检查写入的字节数是否合理（非零、未超限）。
    rtems_libio_check_count(count);

    /*
     * 获取 I/O 对象，并检查是否具有写权限。
     * 若非法或不可写，则设置 errno = EBADF，并返回 -1。
     */
    LIBIO_GET_IOP_WITH_ACCESS(fd, iop, LIBIO_FLAGS_WRITE, EBADF);

    /*
     * 调用底层设备或文件系统提供的写入实现。
     * 实际写入的逻辑由 write_h 函数指针指定。
     */
    n = (*iop->pathinfo.handlers->write_h)(iop, buffer, count);

    // 操作完成后释放 I/O 对象（例如减少引用计数）。
    rtems_libio_iop_drop(iop);

    // 返回写入的字节数，失败时为负值并设置 errno。
    return n;
}
