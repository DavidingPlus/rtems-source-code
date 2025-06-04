/*
 *  Data types
 */

struct IMFS_jnode_tt;
typedef struct IMFS_jnode_tt IMFS_jnode_t;

/**
 * @addtogroup IMFSGenericNodes
 */
/**@{*/

/**
 * @brief Initializes an IMFS node.
 *
 * @param[in] node The IMFS node.
 * @param[in] arg The user provided argument pointer.  It may contain node
 *   specific initialization information.
 *
 * @retval node Successful operation.
 * @retval NULL An error occurred.  The @c errno indicates the error.  This
 * will abort the make operation.
 *
 * @see IMFS_node_control, IMFS_node_initialize_default(), and
 * IMFS_node_initialize_generic().
 */
typedef IMFS_jnode_t *(*IMFS_node_control_initialize)(
    IMFS_jnode_t *node,
    void *arg);

/**
 * @brief Prepares the removal of an IMFS node from its parent directory.
 *
 * @param[in] node The IMFS node.
 *
 * @retval node Successful operation.
 * @retval NULL An error occurred.  The @c errno indicates the error.  This
 * will abort the removal operation.
 *
 * @see IMFS_node_control and IMFS_node_remove_default().
 */
typedef IMFS_jnode_t *(*IMFS_node_control_remove)(
    IMFS_jnode_t *node);

/**
 * @brief Returns the node and does nothing else.
 *
 * @param[in] node The IMFS node.
 *
 * @retval node Returns always the node passed as parameter.
 *
 * @see IMFS_node_control.
 */
IMFS_jnode_t *IMFS_node_remove_default(
    IMFS_jnode_t *node);

/**
 * @brief Destroys an IMFS node.
 *
 * @param[in] node The IMFS node.
 *
 * @see IMFS_node_control and IMFS_node_destroy_default().
 */
typedef void (*IMFS_node_control_destroy)(IMFS_jnode_t *node);

/**
 * @brief Frees the node.
 *
 * @param[in] node The IMFS node.
 *
 * @see IMFS_node_control.
 */
void IMFS_node_destroy_default(IMFS_jnode_t *node);

/**
 * @brief IMFS node control.
 */
// IMFS_node_control：定义 IMFS 节点类型的操作控制器。
// 每种节点类型（如普通文件、目录、符号链接等）可以拥有自己的 handlers 和初始化、销毁等函数指针。
// 该结构体允许 IMFS 在操作不同类型节点时通过回调机制实现多态行为。
typedef struct
{
    // 文件操作处理器集合，包含 open、read、write、ioctl 等函数指针。
    // 每种节点类型可以定义不同的 handlers。
    const rtems_filesystem_file_handlers_r *handlers;

    // 节点初始化函数，在创建该类型节点时调用。
    // 通常在 IMFS_initialize_node 中调用，用于执行类型特定的初始化逻辑。
    IMFS_node_control_initialize node_initialize;

    // 节点删除函数，在执行 unlink/remove 操作时调用。
    // 用于处理节点类型特定的清理逻辑，如从父结构中移除等。
    IMFS_node_control_remove node_remove;

    // 节点销毁函数，在释放节点内存或引用计数归零时调用。
    // 通常用于释放节点中私有数据或执行资源回收。
    IMFS_node_control_destroy node_destroy;
} IMFS_node_control;

typedef struct
{
    IMFS_node_control node_control;
    size_t node_size;
} IMFS_mknod_control;

/*
 *  The control structure for an IMFS jnode.
 */

// IMFS_jnode_tt 是 IMFS 文件系统中用于表示一个文件或目录的节点结构体。这是内存文件系统（IMFS）中最核心的数据结构之一，包含名称、权限、所有者、时间戳等元数据，以及指向父节点和控制操作的指针。
struct IMFS_jnode_tt
{
    // 用于将该节点链接入链表中。
    rtems_chain_node Node;

    // 指向父节点的指针。
    IMFS_jnode_t *Parent;

    // 节点名称，不以 \0 结尾（即不是 C 字符串）。
    const char *name;

    // 节点名称的长度（对应上面的 name）。
    uint16_t namelen;

    // 文件类型和权限信息（如目录、常规文件、权限位）。
    mode_t st_mode;

    // 节点的引用计数，用于资源管理。
    unsigned short reference_count;

    // 硬链接数量（链接计数）。
    nlink_t st_nlink;

    // 拥有者的用户 ID。
    uid_t st_uid;

    // 拥有者的组 ID。
    gid_t st_gid;

    // 最后一次访问时间。
    time_t stat_atime;

    // 最后一次修改内容的时间。
    time_t stat_mtime;

    // 最后一次属性更改（如权限、所有者等）的时间。
    time_t stat_ctime;

    // 节点控制器，定义节点的行为和操作函数。
    const IMFS_node_control *control;
};

/*
 *  Routines
 */

extern int IMFS_initialize(
    rtems_filesystem_mount_table_entry_t *mt_entry,
    const void *data);

extern int IMFS_initialize_support(
    rtems_filesystem_mount_table_entry_t *mt_entry,
    const void *data);

extern IMFS_jnode_t *IMFS_initialize_node(
    IMFS_jnode_t *node,
    const IMFS_node_control *node_control,
    const char *name,
    size_t namelen,
    mode_t mode,
    void *arg);
