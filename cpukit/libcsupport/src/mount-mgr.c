// 定义一个结构体类型 filesystem_node，用于表示文件系统链表中的一个节点。
typedef struct
{
    // RTEMS 提供的双向链表节点结构，用于将多个文件系统节点连接成链表。
    rtems_chain_node node;

    // 文件系统表项，包含该文件系统的初始化函数、挂载函数等描述信息。
    rtems_filesystem_table_t entry;
} filesystem_node;

// 注册文件系统类型及挂载处理函数。注册成功返回 0，失败返回错误码。
int rtems_filesystem_register(
    // 文件系统类型标识（如 "imfs", "dosfs"）。
    const char *type,
    // 文件系统挂载操作函数指针。
    rtems_filesystem_fsmount_me_t mount_h)
{
    rtems_chain_control *chain = &filesystem_chain; // 全局文件系统链表。
    size_t type_size = strlen(type) + 1;            // 类型字符串长度（含结束符）。
    size_t fsn_size = sizeof(filesystem_node) + type_size;
    filesystem_node *fsn = malloc(fsn_size); // 分配节点+类型字符串内存。

    // 内存分配失败处理。
    if (fsn == NULL)
    {
        rtems_set_errno_and_return_minus_one(ENOMEM); // ENOMEM。
    }

    // 节点数据初始化。
    char *type_storage = (char *)fsn + sizeof(*fsn); // 类型字符串存储位置。
    memcpy(type_storage, type, type_size);           // 拷贝类型字符串。
    fsn->entry.type = type_storage;                  // 关联类型字符串指针。
    fsn->entry.mount_h = mount_h;                    // 绑定挂载处理函数。

    // ===== 临界区开始（全局链表操作）=====
    rtems_libio_lock();

    // 关键查重逻辑：检查类型是否已注册。
    if (rtems_filesystem_get_mount_handler(type) == NULL)
    {
        rtems_chain_initialize_node(&fsn->node);           // 初始化链表节点。
        rtems_chain_append_unprotected(chain, &fsn->node); // 加入全局链表。
    }
    else
    {
        // 类型重复处理：释放资源并报错。
        rtems_libio_unlock(); // 先解锁再释放内存。
        free(fsn);
        rtems_set_errno_and_return_minus_one(EINVAL); // EINVAL。
    }

    rtems_libio_unlock();
    // ===== 临界区结束 =====

    return 0; // 注册成功。
}
