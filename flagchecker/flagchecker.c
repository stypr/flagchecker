/*
  flagchecker by stypr

  - https://harold.kim/
  - https://github.com/stypr/flagchecker
*/

#define pr_fmt(fmt) "flagchecker: " fmt
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ftrace.h>
#include <linux/linkage.h>
#include <linux/version.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>


MODULE_AUTHOR("stypr <root@stypr.com>");
MODULE_DESCRIPTION("flagchecker");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.4");


/*
  TODO: Set the flag table.

  File I/O is discouraged in LKM so we put values here directly.
  Reference: https://stackoverflow.com/questions/1184274/read-write-files-within-a-linux-kernel-module

  Note that the flag format has to be Flag{64digit_alphanumeric}

  The length of the flag can be changed by modifying values from generate_random() and generate_flag(),
  But it is good to note that the length of the original flag needs to be the same size as
  the randomly generated flag. Otherwise it can cause a kernel panic or create other sorts of issues.
*/
const char *flag_table[][2] = {
    {"guestbook",   "Bingo{b1d7d0c5b30fe7f8273dd54579cebd9953d903d45ad713c624c05034e7e0f083}"},
    {"example",     "Bingo{d3eb8c2988184fe0f6d4533aa13c91efedb3565f41b639d1109abd79c98be468}"},
    {"stypr_chall", "Bingo{766aba68df48169bf6eacb57a54916d0768be3b1f2f57bf9aa1191174c174831}"},
};


// ftrace hooking stuff starts from here.
// Derived from https://github.com/ilammy/ftrace-hook
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,7,0)
static unsigned long lookup_name(const char *name){
    struct kprobe kp = {
        .symbol_name = name
    };
    unsigned long retval;
    if(register_kprobe(&kp) < 0) return 0;
    retval = (unsigned long) kp.addr;
    unregister_kprobe(&kp);
    return retval;
}
#else
static unsigned long lookup_name(const char *name){
    return kallsyms_lookup_name(name);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,11,0)
#define FTRACE_OPS_FL_RECURSION FTRACE_OPS_FL_RECURSION_SAFE
#define ftrace_regs pt_regs
static __always_inline struct pt_regs *ftrace_get_regs(struct ftrace_regs *fregs) {
    return fregs;
}
#endif

// detect recusion using function return address (USE_FENTRY_OFFSET = 0)
// avoid recusion by jumping over the ftrace call (USE_FENTRY_OFFSET = 1)
#define USE_FENTRY_OFFSET 0

struct ftrace_hook {
    const char *name;
    void *function;
    void *original;
    unsigned long address;
    struct ftrace_ops ops;
};

static int fh_resolve_hook_address(struct ftrace_hook *hook){
    hook->address = lookup_name(hook->name);
    if(!hook->address){
        pr_debug("unresolved symbol: %s\n", hook->name);
        return -ENOENT;
    }
#if USE_FENTRY_OFFSET
    *((unsigned long*) hook->original) = hook->address + MCOUNT_INSN_SIZE;
#else
    *((unsigned long*) hook->original) = hook->address;
#endif
    return 0;
}

static void notrace fh_ftrace_thunk(unsigned long ip, unsigned long parent_ip,
        struct ftrace_ops *ops, struct ftrace_regs *fregs)
{
    struct pt_regs *regs = ftrace_get_regs(fregs);
    struct ftrace_hook *hook = container_of(ops, struct ftrace_hook, ops);
#if USE_FENTRY_OFFSET
    regs->ip = (unsigned long)hook->function;
#else
    if(!within_module(parent_ip, THIS_MODULE))
        regs->ip = (unsigned long)hook->function;
#endif
}

int fh_install_hook(struct ftrace_hook *hook){
    int err;
    err = fh_resolve_hook_address(hook);
    if(err)
        return err;
    hook->ops.func = fh_ftrace_thunk;
    hook->ops.flags = FTRACE_OPS_FL_SAVE_REGS
                    | FTRACE_OPS_FL_RECURSION
                    | FTRACE_OPS_FL_IPMODIFY;
    err = ftrace_set_filter_ip(&hook->ops, hook->address, 0, 0);
    if(err){
        pr_debug("ftrace_set_filter_ip() failed: %d\n", err);
        return err;
    }
    err = register_ftrace_function(&hook->ops);
    if(err){
        pr_debug("register_ftrace_function() failed: %d\n", err);
        ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
        return err;
    }
    return 0;
}

void fh_remove_hook(struct ftrace_hook *hook){
    int err;
    err = unregister_ftrace_function(&hook->ops);
    if(err){
        pr_debug("unregister_ftrace_function() failed: %d\n", err);
    }
    err = ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
    if(err){
        pr_debug("ftrace_set_filter_ip() failed: %d\n", err);
    }
}

int fh_install_hooks(struct ftrace_hook *hooks, size_t count){
    int err;
    size_t i;
    for(i = 0; i < count; i++) {
        err = fh_install_hook(&hooks[i]);
        if(err)
            goto error;
    }
    return 0;
error:
    while(i != 0){
        fh_remove_hook(&hooks[--i]);
    }
    return err;
}

void fh_remove_hooks(struct ftrace_hook *hooks, size_t count){
    size_t i;
    for(i = 0; i < count; i++)
        fh_remove_hook(&hooks[i]);
}

#ifndef CONFIG_X86_64
#error Currently only x86_64 architecture is supported
#endif
#if defined(CONFIG_X86_64) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0))
#define PTREGS_SYSCALL_STUBS 1
#endif
#if !USE_FENTRY_OFFSET
#pragma GCC optimize("-fno-optimize-sibling-calls")
#endif
#ifdef PTREGS_SYSCALL_STUBS
#define SYSCALL_NAME(name) ("__x64_" name)
#else
#define SYSCALL_NAME(name) (name)
#endif
#define HOOK(_name, _function, _original){	\
    .name = SYSCALL_NAME(_name),		\
    .function = (_function),			\
    .original = (_original),			\
}

// Generate 64 byte random string
void generate_random(char *s) {
    int rand = 0;
    static const char alphanum[] = "0123456789abcdef";
    for(int i = 0; i < 64; ++i) {
        get_random_bytes(&rand, sizeof(int)-1);
        s[i] = alphanum[rand % (sizeof(alphanum) - 1)];
    }
    s[64] = 0;
}

// Copied from stackoverflow. I don't want to code it :(
void str_replace(char *target, const char *needle, const char *replacement){
    char buffer [1024] = {0,};
    char *insert_point = &buffer[0];
    const char *tmp = target;
    size_t needle_len = strlen(needle);
    size_t repl_len = strlen(replacement);
    while(1){
        const char *p = strstr(tmp, needle);
        if(p == NULL) {
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

// Load system command to record the generated flag.
void generate_flag(char *target, const char *challenge_name){
    int rc;
    char flag_str   [128] = {0,};
    char random_str [65]  = {0,};
    char *argv[5];

    // Generate flag
    generate_random(random_str);
    strcat(flag_str, "Bingo{");
    strcat(flag_str, random_str);
    strcat(flag_str, "}\0");

    // Run a python script to record the flag
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

#ifdef PTREGS_SYSCALL_STUBS

// Need to call by regs. Ordered based on the calling conventions.
asmlinkage long (*original_read)(struct pt_regs *regs);
asmlinkage long hook_read(struct pt_regs *regs){
    char temp_buf   [256] = {0,};
    char flag_str   [128] = {0,};
    long bufsize, remaining_bufsize;

    // First, use original_read and get buf
    bufsize = original_read(regs);

    // https://www.kernel.org/doc/htmldocs/kernel-api/API---copy-from-user.html
    // Returns number of bytes that could not be copied. On success, this will be zero.
    remaining_bufsize = copy_from_user(temp_buf, (void*)regs->si, 255);

    for(int i = 0; i < ARRAY_SIZE(flag_table); i++){
        if(strstr(temp_buf, flag_table[i][1]) != NULL){
            // Search and Replace
            generate_flag(flag_str, flag_table[i][0]);
            str_replace(temp_buf, flag_table[i][1], flag_str);
            // Replace done? put it back to user level...
            copy_to_user((void*)regs->si, temp_buf, 255 - remaining_bufsize);
            break;
        }
    }
    return bufsize;
}

#else

asmlinkage long (*original_read)(unsigned int fd, char __user *buf, size_t count);
asmlinkage long hook_read(unsigned int fd, char __user *buf, size_t count){
    char temp_buf   [256] = {0,};
    char flag_str   [128] = {0,};
    long bufsize, remaining_bufsize;

    // First, use original_read and get buf
    bufsize = original_read(fd, buf, count);

    // https://www.kernel.org/doc/htmldocs/kernel-api/API---copy-from-user.html
    // Returns number of bytes that could not be copied. On success, this will be zero.
    remaining_bufsize = copy_from_user(temp_buf, buf, 255);

    // Look for a magic string within 255
    // Just to keep <?php echo "Bingo{kusaskusas}"; ?>
    for(int i = 0; i < ARRAY_SIZE(flag_table); i++){
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

#endif

static struct ftrace_hook hook_list[] = {
    HOOK("sys_read",  hook_read,  &original_read),
};

static int __init start_check(void){
    int err;
    err = fh_install_hooks(hook_list, ARRAY_SIZE(hook_list));
    if(err)
        return err;
    pr_info("module loaded\n");
    return 0;
}

static void __exit clean_check(void){
    // Hooks read syscall
    fh_remove_hooks(hook_list, ARRAY_SIZE(hook_list));
    pr_info("module unloaded\n");
}

module_init(start_check);
module_exit(clean_check);
