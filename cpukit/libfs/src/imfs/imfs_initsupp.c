// 初始化 IMFS（In-Memory File System）文件系统的挂载支持。成功返回 0，若发生错误应返回对应的错误码（此本函数实现中永远成功）。
int IMFS_initialize_support(
    rtems_filesystem_mount_table_entry_t *mt_entry, // 指向 RTEMS 文件系统挂载表项的指针。该结构将被填充以描述文件系统的状态，包括根节点位置、操作函数集、路径配置等，是输出参数。
    const void *data                                // 通用类型指针，实际应为 const IMFS_mount_data * 类型，包含初始化文件系统所需的参数，例如操作函数、控制信息等。
)
{
    const IMFS_mount_data *mount_data;     // 挂载参数结构，包含 fs_info、操作函数集等。
    IMFS_fs_info_t *fs_info;               // 文件系统私有信息结构。
    const IMFS_node_control *node_control; // 根目录节点控制器。
    IMFS_jnode_t *root_node;               // 根目录节点。

    // 转换传入的 data 参数为具体类型。
    mount_data = data;

    // 提取文件系统信息和 mknod 控制器。
    fs_info = mount_data->fs_info;
    fs_info->mknod_controls = mount_data->mknod_controls;

    // 获取根目录所用的节点控制器。
    node_control = &mount_data->mknod_controls->directory->node_control;

    // 获取根节点结构体的地址。
    root_node = &fs_info->Root_directory.Node;

    // 设置挂载点结构 mt_entry 的基本信息。
    mt_entry->fs_info = fs_info;                                      // 文件系统私有数据。
    mt_entry->ops = mount_data->ops;                                  // 文件系统操作集。
    mt_entry->pathconf_limits_and_options = &IMFS_LIMITS_AND_OPTIONS; // 路径相关限制。

    // 设置挂载根目录的节点访问和操作处理函数。
    mt_entry->mt_fs_root->location.node_access = root_node;
    mt_entry->mt_fs_root->location.handlers = node_control->handlers;

    // 初始化根目录节点：名称为空，大小为 0，类型为目录，权限为 0755，无父节点（为根）。
    root_node = IMFS_initialize_node(
        root_node,
        node_control,
        "",
        0,
        (S_IFDIR | 0755),
        NULL);

    // 确保初始化成功。
    IMFS_assert(root_node != NULL);

    // 返回成功。
    return 0;
}
