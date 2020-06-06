#ifndef _HISI_USB_CORE_H_
#define _HISI_USB_CORE_H_

struct usb_device;
struct usb_hub;

void notify_hub_too_deep(void);
void notify_power_insufficient(void);

int usb_device_read_mutex_trylock(void);
int usb_device_read_usb_trylock_device(struct usb_device *udev);

bool check_huawei_dock_quirk(struct usb_device *hdev,
		struct usb_hub *hub, int port1);

#endif /* _HISI_USB_CORE_H_ */
