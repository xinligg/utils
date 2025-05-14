#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <stdint.h>
#include <arpa/inet.h> // 用于字节序转换

// 检查文件系统类型
char* detect_filesystem(const char* partition) {
    static char fs_type[32] = {0};
    char command[256];
    FILE *fp;

    snprintf(command, sizeof(command), "blkid -o value -s TYPE %s 2>/dev/null", partition);
    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen failed");
        return NULL;
    }
    
    if (fgets(fs_type, sizeof(fs_type), fp) == NULL) {
        pclose(fp);
        return NULL;
    }
    
    // 去除换行符
    fs_type[strcspn(fs_type, "\n")] = '\0';
    pclose(fp);
    
    return fs_type;
}

// 检查命令是否存在
bool command_exists(const char *cmd) {
    char command[256];
    snprintf(command, sizeof(command), "command -v %s >/dev/null 2>&1", cmd);
    return system(command) == 0;
}

// 设置NTFS卷序列号 (位于0x48偏移处，4字节)
int set_ntfs_uuid(const char* partition, const char* uuid_str) {
    uint32_t serial;
    char *endptr;
    int fd;
    
    // 将16进制字符串转换为32位整数
    serial = strtoul(uuid_str, &endptr, 16);
    if (*endptr != '\0') {
        fprintf(stderr, "无效的NTFS序列号格式(需要8位16进制数)\n");
        return -1;
    }
    
    // 转换为小端字节序
    serial = htole32(serial);

    // 打开分区设备
    fd = open(partition, O_RDWR);
    if (fd == -1) {
        perror("无法打开分区设备");
        return -1;
    }

    if (lseek(fd, 0x48, SEEK_SET) == -1) {
        perror("lseek failed");
        close(fd);
        return -1;
    }
    
    if (write(fd, &serial, sizeof(serial)) != sizeof(serial)) {
        perror("write failed");
        close(fd);
        return -1;
    }

    close(fd);

    return 0;
}

// 设置FAT32卷ID (位于0x43偏移处，4字节)
int set_fat32_uuid(const char* partition, const char* uuid_str) {
    uint32_t volume_id;
    char *endptr;
    int fd;

    volume_id = strtoul(uuid_str, &endptr, 16);
    if (*endptr != '\0') {
        fprintf(stderr, "无效的FAT32卷ID格式(需要8位16进制数)\n");
        return -1;
    }
    
    // FAT32使用小端字节序
    volume_id = htole32(volume_id);
    
    // 打开分区设备
    fd = open(partition, O_RDWR);
    if (fd == -1) {
        perror("无法打开分区设备");
        return -1;
    }

    if (lseek(fd, 0x43, SEEK_SET) == -1) {
        perror("lseek failed");
        close(fd);
        return -1;
    }
    
    if (write(fd, &volume_id, sizeof(volume_id)) != sizeof(volume_id)) {
        perror("write failed");
        close(fd);
        return -1;
    }

    close(fd);

    return 0;
}

// 设置ext4 UUID (位于0x468偏移处，16字节)
int set_ext4_uuid(const char* partition, const char* uuid_str) {
    uint8_t uuid[16];
    int i;
    unsigned int byte;
    char *pos = (char*)uuid_str;

    // 解析标准UUID格式 (如 a1b2c3d4-e5f6-7890-1234-567890abcdef)
    for (i = 0; i < 16; i++) {
//        if (i == 4 || i == 6 || i == 8 || i == 10) {
            if (*pos == '-') pos++;
//            else {
//                fprintf(stderr, "无效的UUID格式(缺少分隔符)\n");
//                return -1;
//            }
//        }
        
        if (sscanf(pos, "%2x", &byte) != 1) {
            fprintf(stderr, "无效的UUID格式(非16进制字符)\n");
            return -1;
        }
        uuid[i] = (uint8_t)byte;
        pos += 2;
    }
        
    // 打开分区设备
    int fd = open(partition, O_RDWR);
    if (fd == -1) {
        perror("无法打开分区设备");
        return -1;
    }

    if (lseek(fd, 0x468, SEEK_SET) == -1) {
        perror("lseek failed");
        close(fd);
        return -1;
    }
    
    if (write(fd, uuid, sizeof(uuid)) != sizeof(uuid)) {
        perror("write failed");
        close(fd);
        return -1;
    }
    
    close(fd);

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "用法: %s <分区设备> <新UUID>\n", argv[0]);
        fprintf(stderr, "示例: %s /dev/sda1 2E24EC2E\n", argv[0]);
        fprintf(stderr, "注意: FAT32使用卷标(最多11字符), NTFS使用8字符序列号, ext4使用标准UUID格式\n");
        return EXIT_FAILURE;
    }
    
    const char* partition = argv[1];
    const char* new_uuid = argv[2];
    
    // 检查root权限
    if (geteuid() != 0) {
        fprintf(stderr, "错误: 此程序需要root权限运行\n");
        return EXIT_FAILURE;
    }
    
    // 检查分区是否存在
    if (access(partition, F_OK) != 0) {
        perror("分区不存在");
        return EXIT_FAILURE;
    }
    
    if (!command_exists("blkid")) {
        fprintf(stderr, "blkid 命令未找到，请安装 blkid 包\n");
        return EXIT_FAILURE;
    }

    // 获取文件系统类型
    char* fs_type = detect_filesystem(partition);
    if (fs_type == NULL) {
        fprintf(stderr, "无法确定文件系统类型\n");
        return EXIT_FAILURE;
    }
    
    printf("检测到文件系统类型: %s\n", fs_type);
    
    int result = -1;
    if (strcmp(fs_type, "ntfs") == 0) {
        result = set_ntfs_uuid(partition, new_uuid);
    } else if (strcmp(fs_type, "vfat") == 0) {
        result = set_fat32_uuid(partition, new_uuid);
    } else if (strcmp(fs_type, "ext4") == 0) {
        result = set_ext4_uuid(partition, new_uuid);
    } else {
        fprintf(stderr, "不支持的文件系统类型: %s\n", fs_type);
        return EXIT_FAILURE;
    }

    if (result != 0) {
        fprintf(stderr, "修改UUID失败\n");
        return EXIT_FAILURE;
    }

    printf("成功将分区 %s 的UUID设置为 %s\n", partition, new_uuid);
    
    // 建议用户更新文件系统缓存
    printf("\n注意: 您可能需要运行以下命令更新系统缓存:\n");
    printf("partprobe %s\n", argv[1]);
    printf("或者重启系统以确保更改生效\n");
    
    return EXIT_SUCCESS;
}
