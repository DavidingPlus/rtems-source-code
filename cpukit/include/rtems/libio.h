/**
 * @brief 初始化一个文件系统实例。
 *
 * 该函数负责初始化挂载表项中的文件系统根节点。
 *
 * @param[in] mt_entry 指向挂载表项的指针，表示要挂载的文件系统实例。
 * @param[in] data 用户提供的初始化数据，如设备路径或挂载选项。
 *
 * @retval 0 操作成功，文件系统实例初始化完成。
 * @retval -1 操作失败，设置 errno 以指示具体错误。
 */

// 定义函数指针类型 rtems_filesystem_fsmount_me_t，表示挂载文件系统的函数。
// 该函数接受挂载表项指针和用户数据作为参数，返回挂载结果。
// 返回 0 表示挂载成功，返回 -1 表示挂载失败且设置 errno。
typedef int (*rtems_filesystem_fsmount_me_t)(
    rtems_filesystem_mount_table_entry_t *mt_entry, // 指向挂载表项，表示要挂载的文件系统。
    const void *data                                // 用户传入的初始化数据。
);

/**
 * @brief File system operations table.
 */
struct _rtems_filesystem_operations_table
{
    // 挂载点加锁函数，防止并发访问挂载点结构。
    rtems_filesystem_mt_entry_lock_t lock_h;

    // 挂载点解锁函数，与 lock_h 成对使用。
    rtems_filesystem_mt_entry_unlock_t unlock_h;

    // 路径解析函数，将路径转换为文件系统节点。
    rtems_filesystem_eval_path_t eval_path_h;

    // 创建硬链接的函数。
    rtems_filesystem_link_t link_h;

    // 判断两个节点是否表示同一对象的函数。
    rtems_filesystem_are_nodes_equal_t are_nodes_equal_h;

    // 创建文件系统节点（如文件、目录、设备节点）的函数。
    rtems_filesystem_mknod_t mknod_h;

    // 删除文件系统节点的函数。
    rtems_filesystem_rmnod_t rmnod_h;

    // 更改节点权限的函数，相当于 chmod。
    rtems_filesystem_fchmod_t fchmod_h;

    // 更改节点所有者信息的函数，相当于 chown。
    rtems_filesystem_chown_t chown_h;

    // 克隆节点的函数，通常用于目录项引用增加时复制节点。
    rtems_filesystem_clonenode_t clonenod_h;

    // 释放节点资源的函数，通常在节点引用减少到 0 时调用。
    rtems_filesystem_freenode_t freenod_h;

    // 文件系统挂载处理函数，处理实际挂载逻辑。
    rtems_filesystem_mount_t mount_h;

    // 文件系统卸载处理函数，释放挂载相关资源。
    rtems_filesystem_unmount_t unmount_h;

    // 文件系统自定义卸载钩子，用于挂载入口被清理时的回调。
    rtems_filesystem_fsunmount_me_t fsunmount_me_h;

    // 修改节点时间戳信息的函数，相当于 utimensat。
    rtems_filesystem_utimens_t utimens_h;

    // 创建符号链接的函数。
    rtems_filesystem_symlink_t symlink_h;

    // 读取符号链接目标路径的函数。
    rtems_filesystem_readlink_t readlink_h;

    // 重命名文件或目录的函数。
    rtems_filesystem_rename_t rename_h;

    // 获取文件系统统计信息的函数，如空间大小、inode 数等。
    rtems_filesystem_statvfs_t statvfs_h;
};

/**
 * @brief File system node operations table.
 */
struct _rtems_filesystem_file_handlers_r
{
    // 打开文件的处理函数指针。
    rtems_filesystem_open_t open_h;

    // 关闭文件的处理函数指针。
    rtems_filesystem_close_t close_h;

    // 读取文件的处理函数指针。
    rtems_filesystem_read_t read_h;

    // 写入文件的处理函数指针。
    rtems_filesystem_write_t write_h;

    // 控制操作（如设备控制）的处理函数指针。
    rtems_filesystem_ioctl_t ioctl_h;

    // 文件位置指针移动（如 lseek）的处理函数指针。
    rtems_filesystem_lseek_t lseek_h;

    // 获取文件状态信息的处理函数指针。
    rtems_filesystem_fstat_t fstat_h;

    // 截断文件大小的处理函数指针。
    rtems_filesystem_ftruncate_t ftruncate_h;

    // 将文件缓冲区数据同步到存储设备的处理函数指针。
    rtems_filesystem_fsync_t fsync_h;

    // 同步文件数据（但不一定包括元数据）的处理函数指针。
    rtems_filesystem_fdatasync_t fdatasync_h;

    // 文件控制（如修改文件描述符属性）的处理函数指针。
    rtems_filesystem_fcntl_t fcntl_h;

    // 轮询文件状态（如是否可读写）的处理函数指针。
    rtems_filesystem_poll_t poll_h;

    // 用于事件过滤（BSD kqueue）的处理函数指针。
    rtems_filesystem_kqfilter_t kqfilter_h;

    // 读取多个缓冲区（向量读）的处理函数指针。
    rtems_filesystem_readv_t readv_h;

    // 写入多个缓冲区（向量写）的处理函数指针。
    rtems_filesystem_writev_t writev_h;

    // 内存映射文件的处理函数指针。
    rtems_filesystem_mmap_t mmap_h;
};

/**
 * @brief An open file data structure.
 *
 * It will be indexed by 'fd'.
 *
 * @todo Should really have a separate per/file data structure that this points
 * to (eg: offset, driver, pathname should be in that)
 */
struct rtems_libio_tt
{
    // 文件状态标志，使用原子类型以支持线程安全操作。
    // 可能标志：是否打开、读/写权限、文件类型等。
    Atomic_Uint flags;

    // 当前文件偏移量，用于读写操作时定位文件指针位置。
    off_t offset;

    // 文件路径定位信息，类似于 inode。
    // 包含挂载点、节点、驱动等信息，用于实际文件访问。
    rtems_filesystem_location_info_t pathinfo;

    // 驱动或文件系统使用的私有字段。
    // 通常用于存储轻量级状态、句柄或标志值。
    uint32_t data0;

    // 驱动或文件系统使用的扩展字段。
    // 可指向任意类型数据，支持更复杂的上下文管理。
    void *data1;
};

/**
 *  @brief Base File System Initialization
 *
 *  Initialize the foundation of the file system.  This is specified
 *  by the structure rtems_filesystem_mount_table.  The usual
 *  configuration is a single instantiation of the IMFS or miniIMFS with
 *  a single "/dev" directory in it.
 */
void rtems_filesystem_initialize(void);

/**
 * @brief Mount table entry.
 */
// 表示一个挂载的文件系统实例，是 Rtems 文件系统挂载表中的一项。
struct rtems_filesystem_mount_table_entry_tt
{
    // 用于将该挂载点插入全局挂载链表。
    rtems_chain_node mt_node;

    // 文件系统私有信息，由具体文件系统实现定义，如 ext2 的 superblock 信息。
    void *fs_info;

    // 指向文件系统操作函数表，定义如 mount、unmount、eval_path 等。
    const rtems_filesystem_operations_table *ops;

    // 文件系统的常量信息，不可变，例如初始挂载参数。
    const void *immutable_fs_info;

    // 该文件系统中所有节点的全局链表，便于遍历。
    rtems_chain_control location_chain;

    // 表示该文件系统挂载在哪个目录（挂载点）上。
    rtems_filesystem_global_location_t *mt_point_node;

    // 表示该文件系统的根节点位置。
    rtems_filesystem_global_location_t *mt_fs_root;

    // 是否已挂载成功。
    bool mounted;

    // 是否支持写操作。
    bool writeable;

    // 是否禁止创建设备节点和普通文件（mknod）。
    bool no_regular_file_mknod;

    // 该文件系统的路径名限制和选项。
    const rtems_filesystem_limits_and_options_t *pathconf_limits_and_options;

    // 挂载点路径字符串，例如 "/mnt/usb"。
    const char *target;

    // 文件系统类型名称，例如 "imfs"、"devfs"、"nfs" 等。
    const char *type;

    // 设备名称，如 "/dev/sda1"，以字符串形式表示，供底层文件系统使用。
    char *dev;

    // 发起卸载操作的任务 ID，卸载完成后通过事件通知该任务。
    rtems_id unmount_task;
};

/**
 * @brief File system table entry.
 */
// 定义一个结构体类型 rtems_filesystem_table_t，用于描述一个可挂载的文件系统类型。
typedef struct rtems_filesystem_table_t
{
    // 文件系统的类型名称，通常为字符串形式，例如 "imfs" 或 "dosfs"。
    const char *type;

    // 文件系统的挂载函数指针，用于挂载该类型的文件系统。
    rtems_filesystem_fsmount_me_t mount_h;
} rtems_filesystem_table_t;

/**
 * @brief Static table of file systems.
 *
 * Externally defined by confdefs.h or the user.
 */
extern const rtems_filesystem_table_t rtems_filesystem_table[];

/**
 * @brief Registers a file system @a type.
 *
 * The @a mount_h handler will be used to mount a file system of this @a type.
 *
 * @retval 0 Successful operation.
 * @retval -1 An error occurred.  The @c errno indicates the error.
 */
int rtems_filesystem_register(
    const char *type,
    rtems_filesystem_fsmount_me_t mount_h);

/**
 * @brief Mounts a file system instance at the specified target path.
 *
 * To mount a standard file system instance one of the following defines should
 * be used to select the file system type
 * - RTEMS_FILESYSTEM_TYPE_DOSFS,
 * - RTEMS_FILESYSTEM_TYPE_FTPFS,
 * - RTEMS_FILESYSTEM_TYPE_IMFS,
 * - RTEMS_FILESYSTEM_TYPE_JFFS2,
 * - RTEMS_FILESYSTEM_TYPE_NFS,
 * - RTEMS_FILESYSTEM_TYPE_RFS, or
 * - RTEMS_FILESYSTEM_TYPE_TFTPFS.
 *
 * Only configured or registered file system types are available.  You can add
 * file system types to your application configuration with the following
 * configuration options
 * - CONFIGURE_FILESYSTEM_DOSFS,
 * - CONFIGURE_FILESYSTEM_FTPFS,
 * - CONFIGURE_FILESYSTEM_IMFS,
 * - CONFIGURE_FILESYSTEM_JFFS2,
 * - CONFIGURE_FILESYSTEM_NFS,
 * - CONFIGURE_FILESYSTEM_RFS, and
 * - CONFIGURE_FILESYSTEM_TFTPFS.
 *
 * In addition to these configuration options file system types can be
 * registered with rtems_filesystem_register().
 *
 * @param[in] source The source parameter will be forwarded to the file system
 * initialization handler.  Usually the source is a path to the corresponding
 * device file, or @c NULL in case the file system does not use a device file.
 * @param[in] target The target path must lead to an existing directory, or
 * must be @c NULL.  In case the target is @c NULL, the root file system will
 * be mounted.
 * @param[in] filesystemtype This string selects the file system type.
 * @param[in] options The options specify if the file system instance allows
 * read-write or read-only access.
 * @param[in] data The data parameter will be forwarded to the file system
 * initialization handler.  It can be used to pass file system specific mount
 * options.  The data structure for mount options is file system specific.  See
 * also in the corresponding file system documentation.
 *
 * @retval 0 Successful operation.
 * @retval -1 An error occurred.  The @c errno indicates the error.
 *
 * @see rtems_filesystem_register(), mount_and_make_target_path(), @ref DOSFS
 * and @ref JFFS2.
 */
int mount(
    const char *source,
    const char *target,
    const char *filesystemtype,
    rtems_filesystem_options_t options,
    const void *data);

typedef struct
{
    // 描述挂载源，通常是设备路径，如 "/dev/sd0"；对 IMFS 等内存文件系统可为 NULL。
    const char *source;

    // 挂载目标目录，必须是系统中已存在的路径，如 "/" 或 "/mnt/usb"。
    const char *target;

    // 文件系统类型的名称字符串，如 "imfs"、"dosfs"、"devfs" 等。
    const char *filesystemtype;

    // 挂载选项，定义为 rtems_filesystem_options_t 类型，控制如只读、读写等行为。
    rtems_filesystem_options_t options;

    // 指向文件系统特定的附加数据，一般为 NULL，某些文件系统可能使用此字段传递配置。
    const void *data;
} rtems_filesystem_mount_configuration;
