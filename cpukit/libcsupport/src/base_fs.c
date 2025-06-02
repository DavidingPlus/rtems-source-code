/*
 *  Default mode for created files.
 */
void rtems_filesystem_initialize(void)
{
    int rv = 0;

    // 获取根文件系统的挂载配置信息（通常是 IMFS）。
    const rtems_filesystem_mount_configuration *root_config =
        &rtems_filesystem_root_configuration;

    // 挂载根文件系统（通常是内存文件系统 IMFS）。
    rv = mount(
        root_config->source,         // 挂载源（IMFS 为 NULL）。
        root_config->target,         // 挂载点（根目录 "/"）。
        root_config->filesystemtype, // 文件系统类型（如 "imfs"）。
        root_config->options,        // 挂载选项（一般为 0）。
        root_config->data            // 传递给文件系统的私有数据（通常为 NULL）。
    );

    // 如果挂载失败，触发致命错误并停止系统。
    if (rv != 0)
        rtems_fatal_error_occurred(0xABCD0002);

    /*
     * 传统 UNIX 系统将设备节点放在 "/dev" 目录，
     * 所以我们手动在根文件系统中创建 "/dev" 目录。
     * 权限为 0755：所有者可读写执行，组用户和其他用户可读执行。
     */
    rv = mkdir("/dev", S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

    // 如果 "/dev" 目录创建失败，也触发致命错误。
    if (rv != 0)
        rtems_fatal_error_occurred(0xABCD0003);

    /*
     * 到此为止，根文件系统（IMFS）和 /dev 目录已经建立。
     *
     * 下面这段注释说明：
     * - 如果你想挂载其他文件系统（如 FAT、NFS 等），你必须先创建挂载点目录。
     * - 如果该文件系统依赖设备（如块设备 /dev/sd0），则必须等设备驱动初始化完成。
     * - 所以此函数只做了最基本的文件系统初始化，并没有自动挂载其他文件系统。
     * - 后续可以手动使用 mount() 挂载其他子文件系统。
     */
}
