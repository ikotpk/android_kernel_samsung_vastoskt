#
#  Makefile for the kernel Sensor device drivers.
#

obj-$(CONFIG_SENSORS_GP2A) 	+= gp2a.o
obj-$(CONFIG_SENSORS_CORE)		+= sensors_core.o
obj-$(CONFIG_GYRO_K3G)			+= k3g.o
obj-$(CONFIG_SENSORS_AK8975)	+= ak8975.o
obj-$(CONFIG_SENSORS_K3DH)		+= k3dh.o
obj-$(CONFIG_OPTICAL_BH1721)	+=bh1721.o
obj-$(CONFIG_ECOMPASS)		+= mecs.o

obj-$(CONFIG_SENSORS_MMC328X_MAG)	+= mmc328x.o mecs.o
obj-$(CONFIG_SENSORS_BMA_ACCEL)	+= bma023_dev.o bma_accel_driver.o bma222.o

