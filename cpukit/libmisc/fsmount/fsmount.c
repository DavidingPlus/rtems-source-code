// 批量挂载文件系统的函数。
// 根据传入的 fstab 表顺序创建挂载点并执行 mount 操作。
// 挂载成功或失败根据用户提供的 report/abort 标志控制终止行为。
// 返回 0 表示全部挂载成功，返回 -1 表示失败并设置 errno。
int rtems_fsmount(
    const rtems_fstab_entry *fstab_ptr, // 指向文件系统挂载表的首项。
    size_t fstab_count,                 // 表项数量。
    size_t *fail_idx                    // 如果挂载失败，记录失败的索引。
)
{
    int rc = 0;             // 函数最终返回值，0 表示成功。
    int tmp_rc;             // 临时返回值，用于每一项操作的状态。
    size_t fstab_idx = 0;   // 当前处理的表项索引。
    bool terminate = false; // 是否中止挂载过程。

    // 遍历挂载表中的所有条目，逐项处理。
    while (!terminate && (fstab_idx < fstab_count))
    {
        tmp_rc = 0;

        // 步骤 1：尝试创建挂载点路径（如 /mnt/sdcard）。
        if (tmp_rc == 0)
        {
            tmp_rc = rtems_mkdir(fstab_ptr->target, S_IRWXU | S_IRWXG | S_IRWXO);
            if (tmp_rc != 0)
            {
                // 若启用错误报告标志，打印错误信息。
                if (0 != (fstab_ptr->report_reasons & FSMOUNT_MNTPNT_CRTERR))
                {
                    fprintf(stdout, "fsmount: creation of mount point \"%s\" failed: %s\n",
                            fstab_ptr->target,
                            strerror(errno));
                }
                // 若启用失败终止标志，设置中止挂载流程。
                if (0 != (fstab_ptr->abort_reasons & FSMOUNT_MNTPNT_CRTERR))
                {
                    terminate = true;
                    rc = tmp_rc;
                }
            }
        }

        // 步骤 2：尝试执行挂载操作。
        if (tmp_rc == 0)
        {
            tmp_rc = mount(fstab_ptr->source,
                           fstab_ptr->target,
                           fstab_ptr->type,
                           fstab_ptr->options,
                           NULL);
            if (tmp_rc != 0)
            {
                // 挂载失败，打印错误信息（若配置了报告标志）。
                if (0 != (fstab_ptr->report_reasons & FSMOUNT_MNT_FAILED))
                {
                    fprintf(stdout, "fsmount: mounting of \"%s\" to \"%s\" failed: %s\n",
                            fstab_ptr->source,
                            fstab_ptr->target,
                            strerror(errno));
                }
                // 挂载失败，根据配置判断是否终止流程。
                if (0 != (fstab_ptr->abort_reasons & FSMOUNT_MNT_FAILED))
                {
                    terminate = true;
                    rc = tmp_rc;
                }
            }
            else
            {
                // 挂载成功，根据配置打印信息。
                if (0 != (fstab_ptr->report_reasons & FSMOUNT_MNT_OK))
                {
                    fprintf(stdout, "fsmount: mounting of \"%s\" to \"%s\" succeeded\n",
                            fstab_ptr->source,
                            fstab_ptr->target);
                }
                // 挂载成功，根据配置判断是否终止流程。
                if (0 != (fstab_ptr->abort_reasons & FSMOUNT_MNT_OK))
                {
                    terminate = true;
                }
            }
        }

        // 步骤 3：准备处理下一条挂载表项。
        if (!terminate)
        {
            // 逆天，还支持批量挂载？？？rtems_fstab_entry 里面没有 next 指针，意思是我要手动预先构造一个 rtems_fstab_entry 数组代表多个挂载项。。。
            fstab_ptr++;
            fstab_idx++;
        }
    }

    // 如果用户提供了失败索引记录指针，则写入最后处理的索引。
    if (fail_idx != NULL)
    {
        *fail_idx = fstab_idx;
    }

    // 返回整体挂载操作的状态。
    return rc;
}
