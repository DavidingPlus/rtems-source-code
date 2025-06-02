// 初始化路径解析上下文，并以给定的 root 和 current 为起点解析路径。
rtems_filesystem_location_info_t *
rtems_filesystem_eval_path_start_with_root_and_current(
    rtems_filesystem_eval_path_context_t *ctx,
    const char *path,
    size_t pathlen,
    int eval_flags,
    rtems_filesystem_global_location_t *const *global_root_ptr,
    rtems_filesystem_global_location_t *const *global_current_ptr)
{
    // 将整个上下文结构清零，确保起始状态干净。
    memset(ctx, 0, sizeof(*ctx));

    // 设置路径字符串及其长度。
    ctx->path = path;
    ctx->pathlen = pathlen;

    // 设置路径解析标志（如是否跟随符号链接、是否创建等）。
    ctx->flags = eval_flags;

    // 根据传入的根目录和当前目录，初始化起始路径位置。
    set_startloc(ctx, global_root_ptr, global_current_ptr);

    // 为路径解析的起点加锁，避免多线程同时访问。
    rtems_filesystem_instance_lock(&ctx->startloc->location);

    // 拷贝起始位置为当前解析位置，作为路径遍历起点。
    rtems_filesystem_location_clone(
        &ctx->currentloc,
        &ctx->startloc->location);

    // 开始路径的逐层解析过程。
    rtems_filesystem_eval_path_continue(ctx);

    // 返回当前解析到的位置（可表示最终文件、目录或中间节点）。
    return &ctx->currentloc;
}

// 初始化路径解析上下文，并从根目录或当前目录开始解析路径。
rtems_filesystem_location_info_t *
rtems_filesystem_eval_path_start(
    rtems_filesystem_eval_path_context_t *ctx,
    const char *path,
    int eval_flags)
{
    // 实际调用带 root 和 current 参数的版本，使用全局根目录和当前目录。
    return rtems_filesystem_eval_path_start_with_root_and_current(
        ctx,
        path,
        strlen(path), // 计算路径长度。
        eval_flags,
        &rtems_filesystem_root,   // 指向全局根目录。
        &rtems_filesystem_current // 指向全局当前目录。
    );
}
