#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/serial.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial_core.h>
#include <linux/delay.h>

#define SERIAL_PORT "/dev/ttyS0" // Serial port
#define BAUD_RATE 9600 // Baud rate

/* Global variables */
static struct kobject *gps_kobj; // Kobject for sysfs entry
static char gps_data[128]; // Buffer to store GPS data

/* Configure the serial port settings */
static void configure_serial_port(void) {
    struct tty_struct *tty;
    struct ktermios settings;

    tty = get_current_tty(); // Get current tty structure
    if (!tty) {
        printk(KERN_ERR "GPS Driver: Unable to get tty structure\n");
        return;
    }

    memset(&settings, 0, sizeof(settings)); // Clear settings structure
    settings.c_cflag = B9600 | CS8 | CLOCAL | CREAD; // Configure baud rate, character size, and enable read
    settings.c_iflag = IGNPAR; // Ignore parity errors
    settings.c_oflag = 0; // No output flags
    settings.c_lflag = 0; // No local flags

    tty_set_termios(tty, &settings); // Apply the settings
   
    printk(KERN_INFO "GPS Driver: Serial port configured\n");
}

/* Read GPS time data from the serial port */
static ssize_t gps_time_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    struct file *file;
    char buffer[256]; // Buffer for reading data
    ssize_t bytes_read;
    char *gprmc_line;
    char *token;
    char *utc_time, *latitude, *longitude, *date;

    file = filp_open(SERIAL_PORT, O_RDONLY, 0); // Open the serial port
    if (IS_ERR(file)) {
        printk(KERN_ERR "GPS DRIVER: Failed to open %s\n", SERIAL_PORT);
        return PTR_ERR(file);
    }

    bytes_read = kernel_read(file, buffer, sizeof(buffer) - 1, &file->f_pos); // Read data from the port
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0'; // Null-terminate the buffer
        gprmc_line = strstr(buffer, "$GPRMC"); // Find the GPRMC line
        if (gprmc_line) {
            token = strsep(&gprmc_line, ","); // Extract UTC time
            utc_time = strsep(&gprmc_line, ",");
            strsep(&gprmc_line, ","); // Skip status field
            latitude = strsep(&gprmc_line, ","); // Extract latitude
            strsep(&gprmc_line, ","); // Skip N/S indicator
            longitude = strsep(&gprmc_line, ","); // Extract longitude
            strsep(&gprmc_line, ","); // Skip E/W indicator
            strsep(&gprmc_line, ","); // Skip speed
            strsep(&gprmc_line, ","); // Skip course
            date = strsep(&gprmc_line, ","); // Extract date

            printk(KERN_INFO "GPS_Driver: UTC-TIME: %s, Latitude: %s, Longitude: %s, Date: %s\n", utc_time, latitude, longitude, date);
            printk(KERN_INFO "GPS_Driver: UTC-TIME - HH: %c%c MM: %c%c SS: %c%c\n", utc_time[0], utc_time[1], utc_time[2], utc_time[3], utc_time[4], utc_time[5]);

            snprintf(gps_data, sizeof(gps_data), "UTC: %c%c:%c%c:%c%c, Date: %c%c/%c%c/%c%c, Latitude: %s, Longitude: %s", 
                     utc_time[0], utc_time[1], utc_time[2], utc_time[3], utc_time[4], utc_time[5], 
                     date[0], date[1], date[2], date[3], date[4], date[5], latitude, longitude);
        } else {
            printk(KERN_INFO "GPS_Driver: No GPRMC Data found\n");
            snprintf(gps_data, sizeof(gps_data), "No GPS Data\n");
        }
    } else {
        printk(KERN_INFO "GPS_Driver: Failed to read Data\n");
        snprintf(gps_data, sizeof(gps_data), "Failed to read Data from GPS\n");
    }

    filp_close(file, NULL); // Close the serial port

    printk(KERN_INFO "GPS_Driver: GPS-Data: %s\n", gps_data);
    return snprintf(buf, PAGE_SIZE, "%s\n", gps_data);
}

/* Create sysfs attribute for reading GPS time */
static struct kobj_attribute gps_time_attribute = __ATTR(gps_time, 0444, gps_time_show, NULL);

/* Initialize GPS driver */
static int __init gps_driver_init(void) {
    printk(KERN_INFO "GPS Driver: Initializing\n");

    configure_serial_port(); // Configure the serial port

    gps_kobj = kobject_create_and_add("gps", kernel_kobj); // Create sysfs entry
    if (!gps_kobj)
        return -ENOMEM;
    if (sysfs_create_file(gps_kobj, &gps_time_attribute.attr)) {
        kobject_put(gps_kobj);
        return -ENOMEM;
    }

    return 0;
}

/* Cleanup GPS driver */
static void __exit gps_driver_exit(void) {
    sysfs_remove_file(gps_kobj, &gps_time_attribute.attr); // Remove sysfs entry
    kobject_put(gps_kobj); // Release kobject
    printk(KERN_INFO "GPS Driver: Exiting\n");
}

module_init(gps_driver_init);
module_exit(gps_driver_exit);

MODULE_LICENSE("GPL"); // Define module license
MODULE_AUTHOR("Bhuvan"); // Define module author
MODULE_DESCRIPTION("GPS Driver"); // Define module description
