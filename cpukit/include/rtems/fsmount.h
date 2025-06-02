/**
 * File system table entry.
 */
// 文件系统挂载表条目结构体。
// 用于描述一个文件系统的挂载信息，包括挂载源、挂载点、文件系统类型及挂载选项。
typedef struct
{
    // 挂载源，表示要挂载的设备、分区或路径。
    const char *source;

    // 挂载目标，即挂载点路径，文件系统将挂载到该目录下。
    const char *target;

    // 文件系统类型名称，如 "imfs"、"dosfs" 等。
    const char *type;

    // 文件系统挂载选项，包含挂载时的参数配置。
    // // 文件系统挂载选项枚举类型，定义了文件系统挂载时支持的不同访问权限模式。
    // typedef enum
    // {
    //     // 只读模式，文件系统以只读方式挂载。
    //     RTEMS_FILESYSTEM_READ_ONLY,

    //     // 读写模式，文件系统以读写方式挂载。
    //     RTEMS_FILESYSTEM_READ_WRITE,

    //     // 无效或错误的挂载选项。
    //     RTEMS_FILESYSTEM_BAD_OPTIONS
    // } rtems_filesystem_options_t;
    rtems_filesystem_options_t options;

    // 报告条件标志，用于指定哪些情况需要报告错误或信息。
    uint16_t report_reasons;

    // 终止条件标志，用于指定哪些情况会导致挂载流程终止。
    uint16_t abort_reasons;
} rtems_fstab_entry;

/**
 * @brief Mounts the file systems listed in the file system mount table.
 *
 * Mounts the file systems listed in the file system mount table @a fstab of
 * size @a size.
 *
 * Each file system will be mounted according to its table entry parameters.
 * In case of an abort condition the corresponding table index will be reported
 * in @a abort_index.  The pointer @a abort_index may be @c NULL.  The mount
 * point paths will be created with rtems_mkdir() and need not exist
 * beforehand.
 *
 * On success, zero is returned.  On error, -1 is returned, and @c errno is set
 * appropriately.
 *
 * @see rtems_bdpart_register_from_disk().
 *
 * The following example code tries to mount a FAT file system within a SD
 * Card.  Some cards do not have a partition table so at first it tries to find
 * a file system inside the hole disk.  If this is successful the mount process
 * will be aborted because the @ref RTEMS_FSTAB_OK condition is true.  If this
 * did not work it tries to mount the file system inside the first partition.
 * If this fails the mount process will not be aborted (this is already the
 * last entry), but the last error status will be returned.
 *
 * @code
 * #include <stdio.h>
 * #include <string.h>
 * #include <errno.h>
 *
 * #include <rtems.h>
 * #include <rtems/bdpart.h>
 * #include <rtems/error.h>
 * #include <rtems/fsmount.h>
 *
 * static const rtems_fstab_entry fstab [] = {
 *   {
 *     .source = "/dev/sd-card-a",
 *     .target = "/mnt",
 *     .type = "dosfs",
 *     .options = RTEMS_FILESYSTEM_READ_WRITE,
 *     .report_reasons = RTEMS_FSTAB_ANY,
 *     .abort_reasons = RTEMS_FSTAB_OK
 *   }, {
 *     .source = "/dev/sd-card-a1",
 *     .target = "/mnt",
 *     .type = "dosfs",
 *     .options = RTEMS_FILESYSTEM_READ_WRITE,
 *     .report_reasons = RTEMS_FSTAB_ANY,
 *     .abort_reasons = RTEMS_FSTAB_NONE
 *   }
 * };
 *
 * static void my_mount(void)
 * {
 *   rtems_status_code sc = RTEMS_SUCCESSFUL;
 *   int rv = 0;
 *   size_t abort_index = 0;
 *
 *   sc = rtems_bdpart_register_from_disk("/dev/sd-card-a");
 *   if (sc != RTEMS_SUCCESSFUL) {
 *     printf("read partition table failed: %s\n", rtems_status_text(sc));
 *   }
 *
 *   rv = rtems_fsmount(fstab, sizeof(fstab) / sizeof(fstab [0]), &abort_index);
 *   if (rv != 0) {
 *     printf("mount failed: %s\n", strerror(errno));
 *   }
 *   printf("mount aborted at %zu\n", abort_index);
 * }
 * @endcode
 */
int rtems_fsmount(
    const rtems_fstab_entry *fstab,
    size_t size,
    size_t *abort_index);
