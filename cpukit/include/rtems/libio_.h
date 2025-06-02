/**
 * This routine searches the IOP Table for an unused entry.  If it
 * finds one, it returns it.  Otherwise, it returns NULL.
 */
rtems_libio_t *rtems_libio_allocate(void);

rtems_filesystem_location_info_t *
rtems_filesystem_eval_path_start(
    rtems_filesystem_eval_path_context_t *ctx,
    const char *path,
    int eval_flags);

rtems_filesystem_location_info_t *
rtems_filesystem_eval_path_start_with_root_and_current(
    rtems_filesystem_eval_path_context_t *ctx,
    const char *path,
    size_t pathlen,
    int eval_flags,
    rtems_filesystem_global_location_t *const *global_root_ptr,
    rtems_filesystem_global_location_t *const *global_current_ptr);

void rtems_filesystem_initialize(void);
