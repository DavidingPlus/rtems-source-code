/**
 * @brief 挂载文件系统的函数。
 *
 * 根据指定的源设备/路径、目标挂载点、文件系统类型和挂载选项，
 * 尝试完成文件系统的挂载操作。
 *
 * @param source         挂载源，通常是设备路径或远程文件系统地址。
 * @param target         挂载目标路径，挂载点目录。若为 NULL，则挂载为根文件系统。
 * @param filesystemtype 文件系统类型字符串，用于获取对应的挂载处理函数。
 * @param options        挂载选项，支持只读或读写模式。
 * @param data           传递给文件系统挂载函数的额外数据指针，可为 NULL。
 *
 * @return 挂载结果，0 表示成功，非0表示失败，失败时设置 errno。
 */
int mount(
    const char *source,
    const char *target,
    const char *filesystemtype,
    rtems_filesystem_options_t options,
    const void *data)
{
    int rv = 0; // 返回值，默认成功。

    // 检查挂载选项是否有效，只支持只读和读写两种。
    if (
        options == RTEMS_FILESYSTEM_READ_ONLY || options == RTEMS_FILESYSTEM_READ_WRITE)
    {
        // 根据文件系统类型获取对应的挂载处理函数指针。
        rtems_filesystem_fsmount_me_t fsmount_me_h =
            rtems_filesystem_get_mount_handler(filesystemtype);

        // 如果找到了对应的挂载函数。
        if (fsmount_me_h != NULL)
        {
            size_t target_length = 0;
            // 分配并初始化一个挂载表项结构，包含源、目标和类型信息。
            rtems_filesystem_mount_table_entry_t *mt_entry = alloc_mount_table_entry(
                source,
                target,
                filesystemtype,
                &target_length);

            // 如果挂载表项分配成功。
            if (mt_entry != NULL)
            {
                // 设置挂载表项的可写权限标志。
                mt_entry->writeable = options == RTEMS_FILESYSTEM_READ_WRITE;

                // 调用具体文件系统的挂载函数完成挂载。
                // 666，整个挂载流程从 rtems_fsmount() -> mount() -> fsmount_me_h。fsmount_me_h 就是 rtems_filesystem_register() 的第二个函数参数。。。。
                rv = (*fsmount_me_h)(mt_entry, data);
                if (rv == 0)
                {
                    // 挂载成功，注册挂载点到文件系统层次。
                    if (target != NULL)
                    {
                        rv = register_subordinate_file_system(mt_entry, target);
                    }
                    else
                    {
                        rv = register_root_file_system(mt_entry);
                    }

                    // 如果注册失败，则卸载已经挂载的文件系统。
                    if (rv != 0)
                    {
                        (*mt_entry->ops->fsunmount_me_h)(mt_entry);
                    }
                }

                // 如果挂载或注册失败，释放挂载表项内存。
                if (rv != 0)
                {
                    free(mt_entry);
                }
            }
            else
            {
                // 挂载表项分配失败，设置内存不足错误。
                errno = ENOMEM;
                rv = -1;
            }
        }
        else
        {
            // 未找到对应的挂载处理函数，设置参数无效错误。
            errno = EINVAL;
            rv = -1;
        }
    }
    else
    {
        // 挂载选项无效，设置参数无效错误。
        errno = EINVAL;
        rv = -1;
    }

    // 返回挂载结果。
    return rv;
}
