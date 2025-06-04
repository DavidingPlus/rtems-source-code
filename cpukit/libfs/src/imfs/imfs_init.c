static const rtems_filesystem_operations_table IMFS_ops = {
    .lock_h = rtems_filesystem_default_lock,
    .unlock_h = rtems_filesystem_default_unlock,
    .eval_path_h = IMFS_eval_path,
    .link_h = IMFS_link,
    .are_nodes_equal_h = rtems_filesystem_default_are_nodes_equal,
    .mknod_h = IMFS_mknod,
    .rmnod_h = IMFS_rmnod,
    .fchmod_h = IMFS_fchmod,
    .chown_h = IMFS_chown,
    .clonenod_h = IMFS_node_clone,
    .freenod_h = IMFS_node_free,
    .mount_h = IMFS_mount,
    .unmount_h = IMFS_unmount,
    .fsunmount_me_h = IMFS_fsunmount,
    .utimens_h = IMFS_utimens,
    .symlink_h = IMFS_symlink,
    .readlink_h = IMFS_readlink,
    .rename_h = IMFS_rename,
    .statvfs_h = rtems_filesystem_default_statvfs};

// 初始化 IMFS 文件系统并挂载到指定挂载点。成功返回 0，失败返回 -1 并设置 errno。
int IMFS_initialize(
    rtems_filesystem_mount_table_entry_t *mt_entry, // 挂载点表项，描述挂载目标。
    const void *data                                // 可选的初始化数据（未使用）。
)
{
    // 为 IMFS 文件系统信息结构分配并清零内存。
    IMFS_fs_info_t *fs_info = calloc(1, sizeof(*fs_info));

    // 构造挂载所需的初始化数据结构，包括操作集和创建节点控制表。
    IMFS_mount_data mount_data = {
        .fs_info = fs_info,                            // 文件系统内部信息。
        .ops = &IMFS_ops,                              // 文件系统操作函数集合。
        .mknod_controls = &IMFS_default_mknod_controls // 创建节点控制信息。
    };

    // 内存分配失败，返回 ENOMEM。
    if (fs_info == NULL)
    {
        rtems_set_errno_and_return_minus_one(ENOMEM);
    }

    // 调用实际支持函数完成文件系统的初始化和挂载。
    return IMFS_initialize_support(mt_entry, &mount_data);
}
