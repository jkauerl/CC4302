/* Necessary includes for device drivers */
#include <linux/init.h>
/* #include <linux/config.h> */
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <linux/uaccess.h> /* copy_from/to_user */

#include "kmutex.h"

MODULE_LICENSE("Dual BSD/GPL");

/* Declaration of syncread.c functions */
int remate_open(struct inode *inode, struct file *filp);
int remate_release(struct inode *inode, struct file *filp);
ssize_t remate_read(struct file *filp, char *buf, size_t count, loff_t *f_pos);
ssize_t remate_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
void remate_exit(void);
int remate_init(void);

/* Structure that declares the usual file */
/* access functions */
struct file_operations remate_fops = {
  read: remate_read,
  write: remate_write,
  open: remate_open,
  release: remate_release
};

/* Declaration of the init and exit functions */
module_init(remate_init);
module_exit(remate_exit);

/*** El driver para lecturas sincronas *************************************/

#define TRUE 1
#define FALSE 0

/* Global variables of the driver */

int remate_major = 61;     /* Major number */

/* Buffer to store data */
#define MAX_SIZE 8192

static char *remate_buffer;
static ssize_t curr_size;
static int reader;
static int pend_open_write;
static int current_count;
static int empty_buff;

/* El mutex y la condicion para remate */
static KMutex mutex;
static KCondition cond;

int remate_init(void) {
  int rc;

  /* Registering device */
  rc = register_chrdev(remate_major, "remate", &remate_fops);
  if (rc < 0) {
    printk(
      "<1>remate: cannot obtain major number %d\n", remate_major);
    return rc;
  }

  reader=0;
  empty_buff=TRUE;
  current_count=0;
  pend_open_write=0;
  curr_size=0;
  m_init(&mutex);
  c_init(&cond);

  /* Allocating remate_buffer */
  remate_buffer = kmalloc(MAX_SIZE, GFP_KERNEL);
  if (remate_buffer==NULL) {
    remate_exit();
    return -ENOMEM;
  }
  memset(remate_buffer, 0, MAX_SIZE);

  printk("<1>Inserting remate module\n");
  return 0;
}

void remate_exit(void) {
  /* Freeing the major number */
  unregister_chrdev(remate_major, "remate");

  /* Freeing buffer remate */
  if (remate_buffer) {
    kfree(remate_buffer);
  }

  printk("<1>Removing remate module\n");
}

int remate_open(struct inode *inode, struct file *filp) {

  if (filp->f_mode & FMODE_WRITE) {
    printk("<1>open for write successful\n");
  }
  else if (filp->f_mode & FMODE_READ) {
    printk("<1>open for read\n");
  }
  return 0;
}

int remate_release(struct inode *inode, struct file *filp) {
  m_lock(&mutex);

  if (filp->f_mode & FMODE_WRITE) {
    c_broadcast(&cond);
    printk("<1>close for write successful\n");
  }
  else if (filp->f_mode & FMODE_READ) {
    reader=0;
    empty_buff=TRUE;
    c_broadcast(&cond);
    printk("<1>close for read (readers remaining=%d)\n", reader);
  }

  m_unlock(&mutex);
  return 0;
}

ssize_t remate_read(struct file *filp, char *buf,
                    size_t count, loff_t *f_pos) {
  ssize_t rc;
  m_lock(&mutex);

  // Esperar si no se ha escrito nada
  reader=1;
  while (empty_buff) {
    if (c_wait(&cond, &mutex)) {
      printk("<1>read interrupted\n");
      rc=-EINTR;
      goto epilog;
    }
  }

  if (count > curr_size-*f_pos) {
    count= curr_size-*f_pos;
  }

  printk("<1>read %d bytes at %d\n", (int)count, (int)*f_pos);

  /* Transfiriendo datos hacia el espacio del usuario */
  if (copy_to_user(buf, remate_buffer+*f_pos, count)!=0) {
    /* el valor de buf es una direccion invalida */
    rc= -EFAULT;
    goto epilog;
  }

  *f_pos+= count;
  rc= count;

epilog:
  c_broadcast(&cond);
  m_unlock(&mutex);
  return rc;
}

ssize_t remate_write( struct file *filp, const char *buf,
                      size_t count, loff_t *f_pos) {
  ssize_t rc;
  loff_t last;

  m_lock(&mutex);

  if(current_count > count) {
    rc=-ECANCELED;
    goto epilog;
  }
  current_count=count;
  empty_buff=0;
  
  last= *f_pos + count;
  if (last>MAX_SIZE) {
    count -= last-MAX_SIZE;
  }
  printk("<1>write %d bytes at %d\n", (int)count, (int)*f_pos);

  /* Transfiriendo datos desde el espacio del usuario */
  if (copy_from_user(remate_buffer+*f_pos, buf, count)!=0) {
    /* el valor de buf es una direccion invalida */
    rc= -EFAULT;
    goto epilog;
  }

  *f_pos += count;
  curr_size= *f_pos;
  rc=count;
  c_broadcast(&cond);
  
  while(!reader) {
    if(count < current_count) {
      rc=-ECANCELED;
      goto epilog;
    }
    if (c_wait(&cond, &mutex)) {
      empty_buff=1;
      rc=-EINTR;
      goto epilog;
    }
  }
  current_count=0;
  
epilog:
  m_unlock(&mutex);
  return rc;
}

