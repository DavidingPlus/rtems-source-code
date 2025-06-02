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
