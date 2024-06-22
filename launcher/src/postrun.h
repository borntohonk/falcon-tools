/*
* Copyright (c) 2022 EliseZeroTwo <mail@elise.moe>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms and conditions of the GNU General Public License,
* version 2, as published by the Free Software Foundation.
*
* This program is distributed in the hope it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _POSTRUN_H_
#define _POSTRUN_H_

#include "hwinit.h"
#include <stdarg.h>
#include "fs.h"

const unsigned int TSEC_ENTRYPOINT = 0x00;


char **sout_buf;

static void _s_putc(char c)
{
	**sout_buf = c;
	*sout_buf += 1;
}

static void _s_puts(char *s)
{
	for (; *s; s++)
		_s_putc(*s);
}

static void _s_putn(u32 v, int base, char fill, int fcnt)
{
	static const char digits[] = "0123456789ABCDEF";

	char *p;
	char buf[65];
	int c = fcnt;
	bool negative = false;

	if (base != 10 && base != 16)
		return;

	// Account for negative numbers.
	if (base == 10 && v & 0x80000000)
	{
		negative = true;
		v = (int)v * -1;
		c--;
	}

	p = buf + 64;
	*p = 0;
	do
	{
		c--;
		*--p = digits[v % base];
		v /= base;
	} while (v);

	if (negative)
		*--p = '-';

	if (fill != 0)
	{
		while (c > 0 && p > buf)
		{
			*--p = fill;
			c--;
		}
	}

	_s_puts(p);
}

void s_printf(char *out_buf, const char *fmt, ...)
{
	va_list ap;
	int fill, fcnt;

	sout_buf = &out_buf;

	va_start(ap, fmt);
	while (*fmt)
	{
		if (*fmt == '%')
		{
			fmt++;
			fill = 0;
			fcnt = 0;
			if ((*fmt >= '0' && *fmt <= '9') || *fmt == ' ')
			{
				fcnt = *fmt;
				fmt++;
				if (*fmt >= '0' && *fmt <= '9')
				{
					fill = fcnt;
					fcnt = *fmt - '0';
					fmt++;
				}
				else
				{
					fill = ' ';
					fcnt -= '0';
				}
			}
			switch (*fmt)
			{
			case 'c':
				_s_putc(va_arg(ap, u32));
				break;
			case 's':
				_s_puts(va_arg(ap, char *));
				break;
			case 'd':
				_s_putn(va_arg(ap, u32), 10, fill, fcnt);
				break;
			case 'p':
			case 'P':
			case 'x':
			case 'X':
				_s_putn(va_arg(ap, u32), 16, fill, fcnt);
				break;
			case '%':
				_s_putc('%');
				break;
			case '\0':
				goto out;
			default:
				_s_putc('%');
				_s_putc(*fmt);
				break;
			}
		}
		else
			_s_putc(*fmt);
		fmt++;
	}

out:
	**sout_buf = '\0';
	va_end(ap);
}

void s_vprintf(char *out_buf, const char *fmt, va_list ap)
{
	int fill, fcnt;

	sout_buf = &out_buf;

	while (*fmt)
	{
		if (*fmt == '%')
		{
			fmt++;
			fill = 0;
			fcnt = 0;
			if ((*fmt >= '0' && *fmt <= '9') || *fmt == ' ')
			{
				fcnt = *fmt;
				fmt++;
				if (*fmt >= '0' && *fmt <= '9')
				{
					fill = fcnt;
					fcnt = *fmt - '0';
					fmt++;
				}
				else
				{
					fill = ' ';
					fcnt -= '0';
				}
			}
			switch(*fmt)
			{
			case 'c':
				_s_putc(va_arg(ap, u32));
				break;
			case 's':
				_s_puts(va_arg(ap, char *));
				break;
			case 'd':
				_s_putn(va_arg(ap, u32), 10, fill, fcnt);
				break;
			case 'p':
			case 'P':
			case 'x':
			case 'X':
				_s_putn(va_arg(ap, u32), 16, fill, fcnt);
				break;
			case '%':
				_s_putc('%');
				break;
			case '\0':
				goto out;
			default:
				_s_putc('%');
				_s_putc(*fmt);
				break;
			}
		}
		else
			_s_putc(*fmt);
		fmt++;
	}

out:
	**sout_buf = '\0';
}

void uart_printf(const char *fmt, ...)
{
	va_list ap;

	//! NOTE: Anything more and it will hang. Heap usage is out of the question.
	char text[256];

	va_start(ap, fmt);
	s_vprintf(text, fmt, ap);
	va_end(ap);

	uart_send(DEBUG_UART_PORT, (u8 *)text, strlen(text));
}

void postrun(tsec_res_t* run) {
    print("sor1 hdcp output: \n");
    unsigned char* sor1 = (unsigned char *)(run->sor1);
    for (int x = 0; x < 0x10; x++) {
        print("%02x", sor1[x]);
    }
    uart_printf("test");
    print("\n");

    if (!sdMount()) {
        print("No SD card, can't write key-file\n");
    } else {
        if (!fopen("/sor1key.bin", "wb")) {
            print("Failed to open sor1key.bin for writing\n");
        } else {
            if (!fwrite((unsigned char *)(run->sor1), 0x10, 1)) {
                print("Failed to write sor1key.bin\n");
            }
            fclose();
        }
    }
}

#endif
