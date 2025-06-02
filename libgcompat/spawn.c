#include <spawn.h>
#include <unistd.h>
#include <errno.h>
int posix_spawn_file_actions_addclosefrom_np(posix_spawn_file_actions_t *actions, int lowfd) {
    if (lowfd < 0) {
        return EBADF; // Consistent with posix_spawn_file_actions_addclose for invalid fd
    }

    long open_max = sysconf(_SC_OPEN_MAX);
    if (open_max == -1) {
        // Fallback if sysconf fails. 1024 is a common value for FD_SETSIZE.
        open_max = 1024;
    }

    // Cap the iteration to a very large but somewhat sane upper limit
    // to prevent excessive memory allocation if _SC_OPEN_MAX is huge.
    // This also implicitly limits the number of malloc calls.
    if (open_max > 4096) {
        open_max = 4096;
    }

    for (int fd = lowfd; fd < open_max; fd++) {
        int ret = posix_spawn_file_actions_addclose(actions, fd);
        if (ret != 0) {
            // An error occurred, likely ENOMEM if malloc failed within addclose.
            // The 'actions' object is now in a partially modified state.
            return ret;
        }
    }
    return 0;
}
