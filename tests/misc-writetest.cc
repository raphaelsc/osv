#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <float.h>
#include <chrono>
#include <memory>

using Clock = std::chrono::high_resolution_clock;
static Clock s_clock;

template<typename T>
static float to_seconds(T duration)
{
    return std::chrono::duration<float>(duration).count();
}

static void random_memset(unsigned char *s, ssize_t n)
{
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        perror("open");
        _exit(-1);
    }
    off_t offset = 0;
    ssize_t to_read = n;
    while (to_read > 0) {
        ssize_t ret = read(fd, s + offset, to_read);
        assert(ret != -1);
        assert(ret <= to_read);

        offset += ret;
        to_read -= ret;
    }
    if (close(fd) == -1) {
        perror("close");
        _exit(-1);
    }
}

int main(int argc, char **argv)
{
    constexpr long int file_sizes[] = {
        25 * 1024L, 50 * 1024L, 100 * 1024L, 250 * 1024L, // kb
        1 * 1024L * 1024L, 2 * 1024L * 1024L, 5 * 1024L * 1024L }; // mb
    constexpr long int buffer_sizes[] = { 1024L, 2 * 1024L, 4 * 1024L, 8 * 1024L };
    constexpr int max_rounds = 3;

    int file_sizes_length = sizeof(file_sizes) / sizeof(file_sizes[0]);
    int buffer_sizes_length = sizeof(buffer_sizes) / sizeof(buffer_sizes[0]);

    // file flags
    int oflags = O_CREAT | O_WRONLY;
    if (argc == 2 && !strcmp(argv[1], "sync")) {
        oflags |= O_SYNC;
    }

    sleep(2);

    printf("file size\tbuffer size\tavg duration\n");
    for (auto i = 0; i < file_sizes_length; i++) {
        long int file_size = file_sizes[i];
        unsigned char *buf = new unsigned char[file_size];
        random_memset(buf, file_size);

        for (auto j = 0; j < buffer_sizes_length; j++) {
            long int buffer_size = buffer_sizes[j];
            float total_duration = 0;

            for (auto round = 0; round < max_rounds; round++) {
                char file_name[64] = "/tmpfileXXXXXX";
                mktemp(file_name);
                int fd = open(file_name, oflags);
                if (fd == -1) {
                    perror("open");
                    return -1;
                }

                auto start = s_clock.now();
                for (auto bytes_written = 0; bytes_written < file_size; bytes_written += buffer_size) {
                    auto ret = write(fd, buf + bytes_written, buffer_size);
                    assert(ret == buffer_size);
                }
                auto end = s_clock.now();

                float duration = to_seconds(end - start);
                total_duration += duration;

                if (close(fd) == -1) {
                    perror("close");
                    return -1;
                }
                assert(unlink(file_name) == 0);
            }
            printf("%-10ld\t%-10ld\t%.4fms\n", file_size, buffer_size, total_duration * 1000 / max_rounds);
        }
        delete buf;
    }

    return 0;
}
