// 执行打开文件操作的核心函数。
static int do_open(
    rtems_libio_t *iop, // I/O 控制块，表示打开的文件或设备。
    const char *path,   // 要打开的文件路径。
    int oflag,          // 打开标志（如只读、只写、创建等）。
    mode_t mode         // 创建文件时的权限模式。
)
{
    // 返回值，初始化为 0，后续存储函数调用结果。
    int rv = 0;

    // 将 iop 转换成文件描述符。
    // #define rtems_libio_iop_to_descriptor(_iop) ((_iop) - &rtems_libio_iops[0])
    // 直接数组地址相减就拿到了文件描述符，666。
    int fd = rtems_libio_iop_to_descriptor(iop);

    // 计算读写标志的辅助变量。
    int rwflag = oflag + 1;

    // 是否有读权限。
    bool read_access = (rwflag & _FREAD) == _FREAD;

    // 是否有写权限。
    bool write_access = (rwflag & _FWRITE) == _FWRITE;

    // 是否需要创建新文件。
    bool make = (oflag & O_CREAT) == O_CREAT;

    // 是否独占创建。
    bool exclusive = (oflag & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL);

    // 是否需要截断文件。
    bool truncate = (oflag & O_TRUNC) == O_TRUNC;

    // 是否以目录方式打开文件。
    bool open_dir;

#ifdef O_NOFOLLOW
    // 是否允许跟随符号链接，O_NOFOLLOW 表示不跟随。
    int follow = (oflag & O_NOFOLLOW) == O_NOFOLLOW ? 0 : RTEMS_FS_FOLLOW_LINK;
#else
    // 默认允许跟随符号链接。
    int follow = RTEMS_FS_FOLLOW_LINK;
#endif

    // 组合权限和标志，用于路径解析。
    int eval_flags = follow | (read_access ? RTEMS_FS_PERMS_READ : 0) | (write_access ? RTEMS_FS_PERMS_WRITE : 0) | (make ? RTEMS_FS_MAKE : 0) | (exclusive ? RTEMS_FS_EXCLUSIVE : 0);

    // 路径解析上下文。
    rtems_filesystem_eval_path_context_t ctx;

    // 当前路径位置结构体指针。
    const rtems_filesystem_location_info_t *currentloc;

    // 是否创建普通文件。
    bool create_reg_file;

    // 启动路径解析，准备解析文件路径。
    rtems_filesystem_eval_path_start(&ctx, path, eval_flags);

    // 获取解析后的当前路径位置信息。
    currentloc = rtems_filesystem_eval_path_get_currentloc(&ctx);

    // 判断当前路径所在文件系统是否允许创建普通文件。
    create_reg_file = !currentloc->mt_entry->no_regular_file_mknod;

    // 如果允许创建普通文件且路径中还有后续路径分段，则创建普通文件。
    if (create_reg_file && rtems_filesystem_eval_path_has_token(&ctx))
    {
        create_regular_file(&ctx, mode);
    }

#ifdef O_DIRECTORY
    // 判断是否以目录方式打开。
    open_dir = (oflag & O_DIRECTORY) == O_DIRECTORY;
#else
    open_dir = false;
#endif

    // 如果有写权限或以目录方式打开，需要额外检查路径类型和权限。
    if (write_access || open_dir)
    {
        // 获取当前路径类型。
        mode_t type = rtems_filesystem_location_type(currentloc);

        // 如果创建普通文件时路径是目录，禁止写操作，返回 EISDIR 错误。
        if (create_reg_file && write_access && S_ISDIR(type))
        {
            rtems_filesystem_eval_path_error(&ctx, EISDIR);
        }

        // 如果以目录方式打开，但路径不是目录，返回 ENOTDIR 错误。
        if (open_dir && !S_ISDIR(type))
        {
            rtems_filesystem_eval_path_error(&ctx, ENOTDIR);
        }
    }

    // 将路径解析得到的当前路径信息保存到 iop 的 pathinfo 中。
    rtems_filesystem_eval_path_extract_currentloc(&ctx, &iop->pathinfo);

    // 清理路径解析上下文，释放资源。
    rtems_filesystem_eval_path_cleanup(&ctx);

    // 设置 iop 的文件控制标志，转换 POSIX oflag 为 LibIO 内部标志。
    rtems_libio_iop_flags_set(iop, rtems_libio_fcntl_flags(oflag));

    // 调用底层文件系统的 open 函数打开文件。
    rv = (*iop->pathinfo.handlers->open_h)(iop, path, oflag, mode);

    // 如果打开成功。
    if (rv == 0)
    {
        /*
         * 延迟设置 LIBIO_FLAGS_OPEN 标志，直到文件截断完成。
         * 这样可避免在截断过程中被其他线程使用或关闭文件描述符。
         */
        if (truncate)
        {
            // 如果有写权限，调用底层 ftruncate 截断文件。
            if (write_access)
            {
                rv = (*iop->pathinfo.handlers->ftruncate_h)(iop, 0);
            }
            else
            {
                // 如果无写权限，截断操作非法，返回错误。
                rv = -1;
                errno = EINVAL;
            }

            // 如果截断失败，则关闭文件。
            if (rv != 0)
            {
                (*iop->pathinfo.handlers->close_h)(iop);
            }
        }

        // 如果截断成功或不需要截断。
        if (rv == 0)
        {
            // 设置文件已打开的标志。
            rtems_libio_iop_flags_set(iop, LIBIO_FLAGS_OPEN);

            // 返回文件描述符。
            rv = fd;
        }
        else
        {
            // 失败返回 -1。
            rv = -1;
        }
    }

    // 如果打开失败，释放 iop 资源。
    if (rv < 0)
    {
        rtems_libio_free(iop);
    }

    // 返回文件描述符或错误码。
    return rv;
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
