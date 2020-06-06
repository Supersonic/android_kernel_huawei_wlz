/*
 * power_event_ne.h
 *
 * notifier event for power event module
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _POWER_EVENT_NE_H_
#define _POWER_EVENT_NE_H_

/*
 * define notifier event for power event driver
 * NE is simplified identifier with notifier event
 */
enum power_event_ne_list {
	POWER_EVENT_NE_BEGIN = 0,
	POWER_EVENT_NE_USB_DISCONNECT = POWER_EVENT_NE_BEGIN,
	POWER_EVENT_NE_USB_CONNECT,
	POWER_EVENT_NE_WIRELESS_DISCONNECT,
	POWER_EVENT_NE_WIRELESS_CONNECT,
	POWER_EVENT_NE_VBUS_CHECK,
	POWER_EVENT_NE_END,
};

void power_event_notify(enum power_event_ne_list event);

#endif /* _POWER_EVENT_NE_H_ */
