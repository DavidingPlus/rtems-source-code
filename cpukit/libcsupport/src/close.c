int close(int fd)
{
    // 指向文件描述符对应的 I/O 对象。
    rtems_libio_t *iop;

    // 当前 I/O 对象的状态标志位。
    unsigned int flags;

    // 用于保存最终返回值。
    int rc;

    // 检查文件描述符是否越界。
    if ((uint32_t)fd >= rtems_libio_number_iops)
    {
        // 错误：Bad file descriptor。
        rtems_set_errno_and_return_minus_one(EBADF);
    }

    /*
     * 根据 fd 获取对应的 I/O 对象。
     * 实现类似于：
     * static inline rtems_libio_t *rtems_libio_iop(int fd)
     * {
     *     return &rtems_libio_iops[fd];
     * }
     */
    iop = rtems_libio_iop(fd);

    // 读取该对象当前的标志。
    flags = rtems_libio_iop_flags(iop);

    /*
     * 这段循环代码的作用是：在线程安全的前提下，
     * 把 I/O 对象的 “打开” 标志（LIBIO_FLAGS_OPEN）清除掉，
     * 并且检测有没有正在并发操作导致的冲突。
     */
    while (true)
    {
        // 期望写入的新标志。
        unsigned int desired;

        // CAS 成功与否。
        bool success;

        // 如果文件未被标记为已打开，返回 EBADF。
        if ((flags & LIBIO_FLAGS_OPEN) == 0)
        {
            // #define EBADF 9 /* Bad file number */
            rtems_set_errno_and_return_minus_one(EBADF);
        }

        // 清除引用计数部分，仅保留控制标志。
        flags &= LIBIO_FLAGS_REFERENCE_INC - 1U;

        // 构造期望的新状态：去掉 OPEN 标志（标记为关闭）。
        desired = flags & ~LIBIO_FLAGS_OPEN;

        // 使用原子操作尝试替换标志，确保线程安全。
        success = _Atomic_Compare_exchange_uint(
            &iop->flags,          // 要更新的目标变量。
            &flags,               // 当前预期值，会被更新为实际值（若失败）。
            desired,              // 想要写入的新值。
            ATOMIC_ORDER_ACQ_REL, // 成功时的内存顺序。
            ATOMIC_ORDER_RELAXED  // 失败时的内存顺序。
        );

        // 成功清除 OPEN 标志，跳出循环。
        if (success)
        {
            break;
        }

        // 如果标志中有非法或冲突的状态，返回 EBUSY。
        if ((flags & ~(LIBIO_FLAGS_REFERENCE_INC - 1U)) != 0)
        {
            rtems_set_errno_and_return_minus_one(EBUSY);
        }
    }

    // 调用具体文件系统提供的 close 方法。
    // 关闭文件，通常会执行文件系统特定的清理工作。
    rc = (*iop->pathinfo.handlers->close_h)(iop);

    // 释放 I/O 对象资源，回收到 I/O 对象池中以供复用。
    rtems_libio_free(iop);

    // 返回关闭操作的结果，通常为 0 表示成功，-1 表示失败（并设置 errno）。
    return rc;
}
