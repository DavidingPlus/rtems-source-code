void *rtems_libio_iop_free_head;

void **rtems_libio_iop_free_tail = &rtems_libio_iop_free_head;

static void rtems_libio_init(void)
{
    uint32_t i;
    rtems_libio_t *iop;

    // 如果 I/O 对象数量大于 0，才进行初始化。
    if (rtems_libio_number_iops > 0)
    {
        // 把空闲链表的头指针指向数组中第一个 I/O 对象。
        iop = rtems_libio_iop_free_head = &rtems_libio_iops[0];

        // 把当前 I/O 对象的 data1 成员指向数组中的下一个 I/O 对象，实现链表链接。
        for (i = 0; (i + 1) < rtems_libio_number_iops; i++, iop++)
            iop->data1 = iop + 1;

        // 最后一个 I/O 对象的 data1 设置为 NULL，表示链表末尾。
        iop->data1 = NULL;

        // 记录链表尾部指针，指向最后一个 I/O 对象的 data1 成员地址。
        rtems_libio_iop_free_tail = &iop->data1;
    }
}
