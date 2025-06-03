// 打开文件的核心实现函数。
static int do_open(
    rtems_libio_t *iop, // I/O 控制块，表示文件或设备。
    const char *path,   // 文件路径。
    int oflag,          // 打开标志。
    mode_t mode         // 创建模式。
)
{
    int rv = 0;                                  // 返回值。
    int fd = rtems_libio_iop_to_descriptor(iop); // 获取文件描述符。
    int rwflag = oflag + 1;

    // 访问权限标志。
    bool read_access = (rwflag & _FREAD) == _FREAD;
    bool write_access = (rwflag & _FWRITE) == _FWRITE;
    bool make = (oflag & O_CREAT) == O_CREAT;
    bool exclusive = (oflag & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL);
    bool truncate = (oflag & O_TRUNC) == O_TRUNC;

    // 链接跟随控制。
#ifdef O_NOFOLLOW
    int follow = (oflag & O_NOFOLLOW) ? 0 : RTEMS_FS_FOLLOW_LINK;
#else
    int follow = RTEMS_FS_FOLLOW_LINK;
#endif

    // 路径解析标志组合。
    int eval_flags = follow |
                     (read_access ? RTEMS_FS_PERMS_READ : 0) |
                     (write_access ? RTEMS_FS_PERMS_WRITE : 0) |
                     (make ? RTEMS_FS_MAKE : 0) |
                     (exclusive ? RTEMS_FS_EXCLUSIVE : 0);

    rtems_filesystem_eval_path_context_t ctx;
    const rtems_filesystem_location_info_t *currentloc;
    bool create_reg_file;

    // 路径解析启动。
    rtems_filesystem_eval_path_start(&ctx, path, eval_flags);
    currentloc = rtems_filesystem_eval_path_get_currentloc(&ctx);

    // 判断是否支持创建普通文件。
    create_reg_file = !currentloc->mt_entry->no_regular_file_mknod;

    // 创建普通文件（如果需要）。
    if (create_reg_file && rtems_filesystem_eval_path_has_token(&ctx))
    {
        create_regular_file(&ctx, mode);
    }

    // 目录打开检查。
#ifdef O_DIRECTORY
    bool open_dir = (oflag & O_DIRECTORY) == O_DIRECTORY;
#else
    bool open_dir = false;
#endif

    if (write_access || open_dir)
    {
        mode_t type = rtems_filesystem_location_type(currentloc);

        if (create_reg_file && write_access && S_ISDIR(type))
        {
            rtems_filesystem_eval_path_error(&ctx, EISDIR);
        }

        if (open_dir && !S_ISDIR(type))
        {
            rtems_filesystem_eval_path_error(&ctx, ENOTDIR);
        }
    }

    // 提取路径信息并清理上下文。
    rtems_filesystem_eval_path_extract_currentloc(&ctx, &iop->pathinfo);
    rtems_filesystem_eval_path_cleanup(&ctx);

    // 设置文件控制标志。
    rtems_libio_iop_flags_set(iop, rtems_libio_fcntl_flags(oflag));

    // 调用底层 open 函数。
    rv = (*iop->pathinfo.handlers->open_h)(iop, path, oflag, mode);

    if (rv == 0)
    {
        // 若需截断，执行 ftruncate。
        if (truncate)
        {
            if (write_access)
            {
                rv = (*iop->pathinfo.handlers->ftruncate_h)(iop, 0);
            }
            else
            {
                rv = -1;
                errno = EINVAL;
            }

            if (rv != 0)
            {
                (*iop->pathinfo.handlers->close_h)(iop);
            }
        }

        if (rv == 0)
        {
            // 设置为打开状态。
            rtems_libio_iop_flags_set(iop, LIBIO_FLAGS_OPEN);
            rv = fd; // 返回文件描述符。
        }
        else
        {
            rv = -1;
        }
    }

    // 打开失败则释放资源。
    if (rv < 0)
    {
        rtems_libio_free(iop);
    }

    return rv; // 返回文件描述符或错误码。
}

/**
 *  POSIX 1003.1 5.3.1 - Open a File
 */
int open(const char *path, int oflag, ...)
{
    int rv = 0;
    va_list ap;
    mode_t mode = 0;
    rtems_libio_t *iop = NULL;

    // 处理可变参数，获取文件创建模式（mode）。
    va_start(ap, oflag);
    mode = va_arg(ap, mode_t);

    // 分配一个文件描述符结构。
    iop = rtems_libio_allocate();
    if (iop != NULL)
    {
        // 调用底层实现打开文件。
        rv = do_open(iop, path, oflag, mode);
    }
    else
    {
        // 文件描述符耗尽，设置错误码。
        errno = ENFILE;
        rv = -1;
    }

    va_end(ap);

    return rv;
}
