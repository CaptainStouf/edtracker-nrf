#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "reports.h"
#include "keycode.h"
#include "rf_protocol.h"
#include "rf_dngl.h"

#include "nrfdbg.h"

hid_joystick_report_t	usb_joystick_report;

void reset_joystick_report(void)
{
	usb_joystick_report.x = 0;
	usb_joystick_report.y = 0;
	usb_joystick_report.z = 0;
}