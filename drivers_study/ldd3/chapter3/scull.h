#ifndef __SCULL_H__
#define __SCULL_H__

#define QSET        (1000)
#define QUANNUM     (4000)

typedef struct scull_qset
{
	void **data;
	struct scull_qset *next;
}scull_qset_t;

typedef struct scull_dev
{
	scull_qset_t *data;
	uint size;
	uint qset;
	uint quannum;
	struct semaphore sema;
	struct cdev cdev;
	struct class *class;
	struct device *device;
}scull_dev_t;

#endif
