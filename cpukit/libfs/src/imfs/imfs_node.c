// 初始化一个 IMFS 节点（内存文件系统中的目录或文件节点）。
// 根据传入的信息填充节点结构体的基本字段，并调用节点类型控制器提供的初始化回调。
// 成功返回已初始化的节点指针，失败返回 NULL 并设置 errno。
IMFS_jnode_t *IMFS_initialize_node(
    IMFS_jnode_t *node,                    // 要初始化的节点结构体指针，由调用者分配内存。
    const IMFS_node_control *node_control, // 控制节点行为的结构体，提供回调函数等。
    const char *name,                      // 节点名称，调用者保证其生命周期。
    size_t namelen,                        // 节点名称长度，避免多余的 strlen 调用。
    mode_t mode,                           // 节点类型和权限，如 S_IFDIR | 0755。
    void *arg                              // 传递给初始化回调函数的可选参数。
)
{
    time_t now;

    // 若名称长度超过限制，则设置错误码并返回 NULL。
    if (namelen > IMFS_NAME_MAX)
    {
        errno = ENAMETOOLONG;
        return NULL;
    }

    // 填充节点的基本元信息。
    node->name = name;
    node->namelen = namelen;
    node->reference_count = 1;
    node->st_nlink = 1;
    node->control = node_control;

    // 设置权限与属主信息。
    node->st_mode = mode;
    node->st_uid = geteuid();
    node->st_gid = getegid();

    // 获取当前时间并设置为节点的访问、修改和创建时间。
    now = _IMFS_get_time();
    node->stat_atime = now;
    node->stat_mtime = now;
    node->stat_ctime = now;

    // 调用节点控制器中定义的初始化函数以执行具体类型的初始化。
    return (*node_control->node_initialize)(node, arg);
}

IMFS_jnode_t *IMFS_node_remove_default(
    IMFS_jnode_t *node)
{
    return node;
}
