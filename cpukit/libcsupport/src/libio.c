rtems_libio_t *rtems_libio_allocate(void)
{
    rtems_libio_t *iop;

    // 加锁，保护全局空闲链表。
    rtems_libio_lock();

    // 从空闲链表头获取一个可用的文件描述符结构。
    iop = rtems_libio_iop_free_head;

    if (iop != NULL)
    {
        void *next;

        // 获取下一个空闲节点。
        next = iop->data1;

        // 更新空闲链表头指针。
        rtems_libio_iop_free_head = next;

        // 如果空闲链表已空，更新尾指针。
        if (next == NULL)
        {
            rtems_libio_iop_free_tail = &rtems_libio_iop_free_head;
        }
    }

    // 解锁，释放对空闲链表的访问。
    rtems_libio_unlock();

    // 返回分配到的文件描述符结构（可能为 NULL）。
    return iop;
}
