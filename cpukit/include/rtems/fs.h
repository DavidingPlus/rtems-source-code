/* FIXME: shouldn't this better not be here? */
typedef struct rtems_libio_tt rtems_libio_t;

struct rtems_filesystem_mount_table_entry_tt;
typedef struct rtems_filesystem_mount_table_entry_tt
    rtems_filesystem_mount_table_entry_t;

typedef struct _rtems_filesystem_file_handlers_r
    rtems_filesystem_file_handlers_r;

/**
 * @brief File system location.
 *
 * @ingroup LibIO
 */
// 表示文件系统中一个节点（如文件或目录）的位置及其访问信息。
typedef struct rtems_filesystem_location_info_tt
{
    // 用于将该节点插入到挂载点的链表中（如目录项列表）。
    rtems_chain_node mt_entry_node;

    // 指向具体节点的访问结构，一般是与具体文件系统实现相关的 inode 或数据结构。
    void *node_access;

    // 可选的第二个访问字段，供文件系统使用，如软链接或扩展元数据。
    void *node_access_2;

    // 指向该节点所使用的文件操作处理器集合（如 open、read、write、close 等函数指针）。
    const rtems_filesystem_file_handlers_r *handlers;

    // 当前节点所属的挂载表条目，表示该节点来自哪个挂载的文件系统。
    rtems_filesystem_mount_table_entry_t *mt_entry;

} rtems_filesystem_location_info_t;
