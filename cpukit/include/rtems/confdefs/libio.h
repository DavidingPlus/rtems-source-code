const rtems_filesystem_table_t rtems_filesystem_table[] = {
    {"/", IMFS_initialize_support},
#ifdef CONFIGURE_FILESYSTEM_DOSFS
    {RTEMS_FILESYSTEM_TYPE_DOSFS, rtems_dosfs_initialize},
#endif
#ifdef CONFIGURE_FILESYSTEM_FTPFS
    {RTEMS_FILESYSTEM_TYPE_FTPFS, rtems_ftpfs_initialize},
#endif
#ifdef CONFIGURE_FILESYSTEM_IMFS
    {RTEMS_FILESYSTEM_TYPE_IMFS, IMFS_initialize},
#endif
#ifdef CONFIGURE_FILESYSTEM_JFFS2
    {RTEMS_FILESYSTEM_TYPE_JFFS2, rtems_jffs2_initialize},
#endif
#ifdef CONFIGURE_FILESYSTEM_NFS
    {RTEMS_FILESYSTEM_TYPE_NFS, rtems_nfs_initialize},
#endif
#ifdef CONFIGURE_FILESYSTEM_RFS
    {RTEMS_FILESYSTEM_TYPE_RFS, rtems_rfs_rtems_initialise},
#endif
#ifdef CONFIGURE_FILESYSTEM_TFTPFS
    {RTEMS_FILESYSTEM_TYPE_TFTPFS, rtems_tftpfs_initialize},
#endif
    {NULL, NULL}};

const rtems_filesystem_mount_configuration
    rtems_filesystem_root_configuration = {
        NULL,
        NULL,
        "/",
        RTEMS_FILESYSTEM_READ_WRITE,
        &IMFS_root_mount_data};

#if CONFIGURE_MAXIMUM_FILE_DESCRIPTORS > 0
rtems_libio_t rtems_libio_iops[CONFIGURE_MAXIMUM_FILE_DESCRIPTORS];

const uint32_t rtems_libio_number_iops = RTEMS_ARRAY_SIZE(rtems_libio_iops);
#endif
