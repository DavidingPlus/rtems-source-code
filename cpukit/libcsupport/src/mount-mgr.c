// 定义一个结构体类型 filesystem_node，用于表示文件系统链表中的一个节点。
typedef struct
{
    // RTEMS 提供的双向链表节点结构，用于将多个文件系统节点连接成链表。
    rtems_chain_node node;

    // 文件系统表项，包含该文件系统的初始化函数、挂载函数等描述信息。
    rtems_filesystem_table_t entry;
} filesystem_node;

// 迭代所有已注册文件系统，包括静态表和动态注册链表。
// 对每个文件系统调用用户提供的 routine 回调函数，当回调返回 true 时停止迭代。
bool rtems_filesystem_iterate(
    rtems_per_filesystem_routine routine, // 用户提供的回调函数指针。
    void *routine_arg                     // 传递给回调函数的用户参数指针。
)
{
    // 获取全局动态注册的文件系统链表控制结构指针。
    rtems_chain_control *chain = &filesystem_chain;

    // 指向静态文件系统表的第一个表项，用于预先注册的文件系统。
    const rtems_filesystem_table_t *table_entry = &rtems_filesystem_table[0];

    // 链表遍历时的当前节点指针，初始化为空。
    rtems_chain_node *node = NULL;

    // 标记是否需要停止继续迭代，初始为 false。
    bool stop = false;

    // 遍历静态文件系统表，直到遇到表项 type 为 NULL 或者回调要求停止。
    while (table_entry->type && !stop)
    {
        // 调用用户回调，将当前静态表项和用户参数一起传递。
        stop = (*routine)(table_entry, routine_arg);

        // 移动到下一个静态表项。
        ++table_entry;
    }

    // 如果静态表项遍历完成且回调未要求停止，则遍历动态链表中的注册项。
    if (!stop)
    {
        // 进入临界区，锁定全局文件系统链表，防止并发修改。
        rtems_libio_lock();

        // 从链表头开始遍历，直到到达链表尾或回调要求停止。
        for (
            node = rtems_chain_first(chain);
            !rtems_chain_is_tail(chain, node) && !stop;
            node = rtems_chain_next(node))
        {
            // 将链表节点转换为文件系统节点结构指针，以获取 entry 字段。
            const filesystem_node *fsn = (filesystem_node *)node;

            // 调用用户回调，将动态注册表项 entry 和用户参数一起传递。
            stop = (*routine)(&fsn->entry, routine_arg);
        }

        // 遍历完成或停止后，退出临界区，解锁链表。
        rtems_libio_unlock();
    }

    // 返回回调最终是否请求停止迭代的标志。
    return stop;
}

typedef struct
{
    const char *type;
    rtems_filesystem_fsmount_me_t mount_h;
} find_arg;

static bool find_handler(const rtems_filesystem_table_t *entry, void *arg)
{
    find_arg *fa = arg;

    if (strcmp(entry->type, fa->type) != 0)
    {
        return false;
    }
    else
    {
        fa->mount_h = entry->mount_h;

        return true;
    }
}

rtems_filesystem_fsmount_me_t
rtems_filesystem_get_mount_handler(
    const char *type)
{
    find_arg fa = {
        .type = type,
        .mount_h = NULL};

    if (type != NULL)
    {
        rtems_filesystem_iterate(find_handler, &fa);
    }

    return fa.mount_h;
}

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
