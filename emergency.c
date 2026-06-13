#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct kobject *trap_kobj;

/* Yazma callback: echo 1 > /sys/kernel/trap/trap_event */
static ssize_t trap_store(struct kobject *kobj,
                          struct kobj_attribute *attr,
                          const char *buf, size_t count)
{
    if (buf[0] == '1') {
        printk(KERN_EMERG "Trap triggered!\n");

        // 1. Root FS remount RO
        call_usermodehelper("/bin/mount",
            (char *[]){"mount","-o","remount,ro","/",NULL},
            NULL, UMH_WAIT_PROC);

        // 2. Sync
        call_usermodehelper("/bin/sync",
            (char *[]){"sync",NULL},
            NULL, UMH_WAIT_PROC);

        // 3. Log dump
        call_usermodehelper("/bin/dmesg",
            (char *[]){"dmesg",NULL},
            NULL, UMH_WAIT_PROC);

        // 4. Ağ interface down
        call_usermodehelper("/sbin/ip",
            (char *[]){"ip","link","set","eth0","down",NULL},
            NULL, UMH_WAIT_PROC);

        // 5. Panic
        panic("Trap triggered!");
    }
    return count;
}

/* Okuma callback: cat /sys/kernel/trap/trap_event */
static ssize_t trap_show(struct kobject *kobj,
                         struct kobj_attribute *attr,
                         char *buf)
{
    return sprintf(buf, "Trap sysfs entry, echo 1 ile tetiklenir.\n");
}

/* Attribute tanımı */
static struct kobj_attribute trap_attr =
    __ATTR(trap_event, 0644, trap_show, trap_store);

static int __init trap_init(void)
{
    trap_kobj = kobject_create_and_add("trap", kernel_kobj);
    if (!trap_kobj)
        return -ENOMEM;

    if (sysfs_create_file(trap_kobj, &trap_attr.attr))
        kobject_put(trap_kobj);

    printk(KERN_INFO "Module Steghide loaded and sysfs entry ready.\n");
    return 0;
}

static void __exit trap_exit(void)
{
    kobject_put(trap_kobj);
    printk(KERN_INFO "Trap sysfs entry kaldırıldı\n");
}

module_init(trap_init);
module_exit(trap_exit);

MODULE_LICENSE("CUSTOM");
MODULE_AUTHOR("Steghidee");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("SELinux add-on");
