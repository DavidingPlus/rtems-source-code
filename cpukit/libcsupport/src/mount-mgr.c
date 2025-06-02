// 定义一个结构体类型 filesystem_node，用于表示文件系统链表中的一个节点。
typedef struct
{
    // RTEMS 提供的双向链表节点结构，用于将多个文件系统节点连接成链表。
    rtems_chain_node node;

    // 文件系统表项，包含该文件系统的初始化函数、挂载函数等描述信息。
    rtems_filesystem_table_t entry;
} filesystem_node;

int rtems_filesystem_register(
    // 文件系统类型字符串，例如 "imfs"、"dosfs" 等。
    const char *type,
    // 挂载处理函数指针，用于挂载该类型的文件系统。
    rtems_filesystem_fsmount_me_t mount_h)
{
    // 获取全局文件系统链表控制结构的指针。
    rtems_chain_control *chain = &filesystem_chain;

    // 计算文件系统类型字符串长度（包含字符串结束符）。
    size_t type_size = strlen(type) + 1;

    // 计算要分配的内存大小：filesystem_node 结构体大小 + 文件系统类型字符串大小。
    size_t fsn_size = sizeof(filesystem_node) + type_size;

    // 动态分配内存用于存储新文件系统节点和类型字符串。
    filesystem_node *fsn = malloc(fsn_size);

    // 指向分配内存中，存储类型字符串的位置（紧接在 filesystem_node 结构体之后）。
    char *type_storage = (char *)fsn + sizeof(*fsn);

    // 如果内存分配失败。
    if (fsn == NULL)
        // 设置错误码为“内存不足”，返回 -1。
        rtems_set_errno_and_return_minus_one(ENOMEM);

    // 将传入的文件系统类型字符串拷贝到刚分配的内存中。
    memcpy(type_storage, type, type_size);

    // 设置节点中的 type 指针指向存储的类型字符串。
    fsn->entry.type = type_storage;

    // 设置节点中的挂载处理函数指针。
    fsn->entry.mount_h = mount_h;

    // 加锁，防止多线程环境下链表操作冲突。
    rtems_libio_lock();

    // 检查链表中是否已注册过该类型的文件系统。
    if (rtems_filesystem_get_mount_handler(type) == NULL)
    {
        // 初始化新节点的链表节点结构。
        rtems_chain_initialize_node(&fsn->node);

        // 将新节点追加到文件系统链表中（无锁版本，锁由外部保证）。
        rtems_chain_append_unprotected(chain, &fsn->node);
    }
    else
    {
        // 如果已注册，解锁。
        rtems_libio_unlock();

        // 释放刚分配的内存。
        free(fsn);

        // 设置错误码为“无效参数”（文件系统类型重复），返回 -1。
        rtems_set_errno_and_return_minus_one(EINVAL);
    }

    // 解锁。
    rtems_libio_unlock();

    // 成功返回 0。
    return 0;
}
