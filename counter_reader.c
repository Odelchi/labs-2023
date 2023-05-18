#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEVICE_PATH "/dev/chardev"
#define FILE_PATH "/var/log/character_driver.log"

int main()
{
    int fd = open(DEVICE_PATH, O_RDONLY); //открывает устройство
    if (!fd) {
        return EXIT_FAILURE;
    }

    FILE *out_file = fopen(FILE_PATH, "w"); // открывает файл в которые записывает логи
    if (!out_file) {
        close(fd);
        return EXIT_FAILURE;
    }

    while(1){
        char buf[32];
        ssize_t ret = read(fd, buf, sizeof(buf));
        if (ret < 0) {
            fprintf(out_file, "Failed to read from device");
            close(fd);
            fclose(out_file);
            return EXIT_FAILURE;
        }
        fprintf(out_file, "Read %zd bytes from device: %.*s\n", ret, (int)ret, buf);
        fflush(out_file);
        sleep(5); // каждые 5 секунд записывает в лог значение драйвера
    }

    close(fd);
    fclose(out_file);
    return EXIT_SUCCESS;
}
