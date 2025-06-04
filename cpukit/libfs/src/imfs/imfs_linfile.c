static const rtems_filesystem_file_handlers_r IMFS_linfile_handlers = {
    .open_h = IMFS_linfile_open,
    .close_h = rtems_filesystem_default_close,
    .read_h = IMFS_linfile_read,
    .write_h = rtems_filesystem_default_write,
    .ioctl_h = rtems_filesystem_default_ioctl,
    .lseek_h = rtems_filesystem_default_lseek_file,
    .fstat_h = IMFS_stat_file,
    .ftruncate_h = rtems_filesystem_default_ftruncate,
    .fsync_h = rtems_filesystem_default_fsync_or_fdatasync_success,
    .fdatasync_h = rtems_filesystem_default_fsync_or_fdatasync_success,
    .fcntl_h = rtems_filesystem_default_fcntl,
    .kqfilter_h = rtems_filesystem_default_kqfilter,
    .mmap_h = rtems_filesystem_default_mmap,
    .poll_h = rtems_filesystem_default_poll,
    .readv_h = rtems_filesystem_default_readv,
    .writev_h = rtems_filesystem_default_writev};

// IMFS 线性文件节点初始化回调函数。
// 将传入的上下文信息（文件大小和数据指针）填充到线性文件节点结构中。
static IMFS_jnode_t *IMFS_node_initialize_linfile(
    IMFS_jnode_t *node, // 通用 IMFS 节点指针。
    void *arg           // 指向 IMFS_linearfile_context 的上下文指针。
)
{
    // 线性文件专用节点结构体指针，用于访问特有字段。
    IMFS_linearfile_t *linfile;
    // 线性文件上下文指针，用于传递 size 和 data。
    IMFS_linearfile_context *ctx;

    // 将通用节点指针转换为线性文件节点，以访问其特有字段。
    linfile = (IMFS_linearfile_t *)node;
    // 将传入的上下文指针转换为线性文件上下文类型。
    ctx = arg;
    // 从上下文中获取文件大小并存储到节点的 File.size 字段。
    linfile->File.size = ctx->size;
    // 将上下文中的数据缓冲区指针赋给节点的 direct 字段（去除 const 限定）。
    linfile->direct = RTEMS_DECONST(void *, ctx->data);

    // 返回已初始化的节点给 IMFS 框架继续使用。
    return node;
}

const IMFS_node_control IMFS_node_control_linfile = {
    .handlers = &IMFS_linfile_handlers,
    .node_initialize = IMFS_node_initialize_linfile,
    .node_remove = IMFS_node_remove_default,
    .node_destroy = IMFS_node_destroy_default};
