/*
  flagchecker by stypr

  - https://harold.kim/
  - https://github.com/stypr/flagchecker
*/
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/syscalls.h>
#include <linux/kallsyms.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/kern_levels.h>
#include <linux/gfp.h>
#include <linux/fcntl.h>
#include <asm/unistd.h>
#include <asm/paravirt.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#define array_length(a) (sizeof(a) / sizeof(*a))

MODULE_LICENSE("GPL");
MODULE_AUTHOR("stypr");
MODULE_DESCRIPTION("flagchecker");
MODULE_VERSION("1.3");

// Flag Table. File I/O is discouraged in LKM so we put it directly here
// https://stackoverflow.com/questions/1184274/read-write-files-within-a-linux-kernel-module
const char *flag_table[][2] = {
    {"guestbook",   "Bingo{b1d7d0c5b30fe7f8273dd54579cebd9953d903d45ad713c624c05034e7e0f083}"},
    {"example",     "Bingo{d3eb8c2988184fe0f6d4533aa13c91efedb3565f41b639d1109abd79c98be468}"},
    {"stypr_chall", "Bingo{766aba68df48169bf6eacb57a54916d0768be3b1f2f57bf9aa1191174c174831}"},
};

// Linux Kernel 5.x blocks write cr0, We're switching to 4.x (Ubuntu 18.04)
unsigned long **SYS_CALL_TABLE;

void enable_page_writing(void){
    write_cr0(read_cr0() & (~0x10000));
}

void disable_page_writing(void){
    write_cr0(read_cr0() | 0x10000);
}

// Generate 64 byte random string
void generate_random(char *s) {
    int rand = 0;
    static const char alphanum[] = "0123456789abcdef";
    for (int i = 0; i < 64; ++i) {
        get_random_bytes(&rand, sizeof(int)-1);
        s[i] = alphanum[rand % (sizeof(alphanum) - 1)];
    }
    s[64] = 0;
}

// Copied from stackoverflow. I don't want to code it :(
void str_replace(char *target, const char *needle, const char *replacement){
    char buffer[1024] = {0,};
    char *insert_point = &buffer[0];
    const char *tmp = target;
    size_t needle_len = strlen(needle);
    size_t repl_len = strlen(replacement);
    while(1){
        const char *p = strstr(tmp, needle);
        if (p == NULL) {
            strcpy(insert_point, tmp);
            break;
        }
        memcpy(insert_point, tmp, p - tmp);
        insert_point += p - tmp;
        memcpy(insert_point, replacement, repl_len);
        insert_point += repl_len;
        tmp = p + needle_len;
    }
    strcpy(target, buffer);
}

// Load system command.
// I know it's dangerous but I am tired of kernel panics :(
void generate_flag(char *target, const char *challenge_name){
    int rc;
    char flag_str   [128] = {0,};
    char random_str [65]  = {0,};
    char *argv[5];

    // generate flag
    generate_random(random_str);
    strcat(flag_str, "Bingo{");
    strcat(flag_str, random_str);
    strcat(flag_str, "}\0");

    // run flag
    argv[0] = "/usr/bin/python3";
    argv[1] = "/root/generate_flag.py";
    argv[2] = (char *)challenge_name;
    argv[3] = (char *)flag_str;
    argv[4] = NULL;

    // Userland execve
    // You may also do UMH_NO_WAIT to improve the speed, but it is not recommended.
    rc = call_usermodehelper(argv[0], argv, NULL, UMH_WAIT_EXEC);
    strcpy(target, flag_str);
}

// Original Data
asmlinkage long (*original_read)(unsigned int fd, char __user *buf, size_t count);

// Hooking sys_read
asmlinkage long hook_read(unsigned int fd, char __user *buf, size_t count){
    char temp_buf   [256] = {0,};
    char flag_str   [128] = {0,};
    long bufsize, remaining_bufsize;

    // First, use original_read and get buf
    bufsize = original_read(fd, buf, count);

    // https://www.fsl.cs.sunysb.edu/kernel-api/re257.html
    // Returns number of bytes that could not be copied. On success, this will be zero.
    remaining_bufsize = copy_from_user(temp_buf, buf, 255);

    // Look for a magic string within 255
    // Just to keep <?php echo "Bingo{kusaskusas}"; ?>
    for(int i = 0; i < array_length(flag_table); i++){
        if(strstr(temp_buf, flag_table[i][1]) != NULL){
            // Search and Replace
            generate_flag(flag_str, flag_table[i][0]);
            str_replace(temp_buf, flag_table[i][1], flag_str);
            // Replace done? put it back to user level...
            copy_to_user(buf, temp_buf, 255 - remaining_bufsize);
            break;
        }
    }
    return bufsize;
}

static int __init start_check(void) {
    SYS_CALL_TABLE = (unsigned long**)kallsyms_lookup_name("sys_call_table");
    enable_page_writing();
    original_read   = (void*)SYS_CALL_TABLE[__NR_read];
    SYS_CALL_TABLE[__NR_read]   = (unsigned long*)hook_read;
    disable_page_writing();
    return 0;
}

static void __exit clean_check(void) {
    // Hooks read syscall
    enable_page_writing();
    SYS_CALL_TABLE[__NR_read] = (unsigned long*)original_read;
    disable_page_writing();
}

module_init(start_check);
module_exit(clean_check);
