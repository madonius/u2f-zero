#include <SI_EFM8UB1_Register_Enums.h>
#include <stdarg.h>

#include "efm8_usb.h"
#include "usb_0.h"
#include "atecc508a.h"
#include "InitDevice.h"
#include "descriptors.h"
#include "idle.h"
#include "bsp.h"
#include "app.h"
#include "i2c.h"
#include "u2f_hid.h"
#include "tests.h"


data struct APP_DATA appdata;


#ifdef U2F_PRINT

	FIFO_CREATE(debug,struct debug_msg, 5)

	static xdata struct debug_msg dbg;

	void flush_messages()
	{
		while(debug_fifo_get(&dbg) == 0)
		{
			u2f_write_s(dbg.buf);
		}
	}

#else

	#define flush_messages(x)

#endif

static void init(struct APP_DATA* ap)
{
	memset(ap,0, sizeof(struct APP_DATA));
	debug_fifo_init();
	u2f_hid_init();
	smb_init();
}

static void listen_for_pkt(struct APP_DATA* ap)
{
	USBD_Read(EP1OUT, ap->hidmsgbuf, sizeof(ap->hidmsgbuf), true);
}



void test_ecc508a()
{
	uint8_t buf[40];
	uint8_t len;
//	do{
//		atecc_send(ATECC_CMD_COUNTER,
//				ATECC_COUNTER_READ,
//				ATECC_COUNTER1,NULL,0);
//	}while((len = atecc_recv(buf,sizeof(buf))) < 0);

//	do{
//		atecc_send(ATECC_CMD_RNG,
//				ATECC_RNG_P1,
//				ATECC_RNG_P2,NULL,0);
//	}while((len = atecc_recv(buf,sizeof(buf))) < 0);



}

#define ms_since(ms,num) (((uint16_t)get_ms() - (ms)) >= num ? (1|(ms=(uint16_t)get_ms())):0)

int16_t main(void) {

	data uint8_t i = 0;
	data uint16_t last_ms = get_ms();
	data uint16_t ms_heart;
	data uint16_t ms_wink;
	data uint8_t winks = 0;
	data uint8_t test = 0;


	init(&appdata);

	enter_DefaultMode_from_RESET();

	// STDIO library requires TI to print
	SCON0_TI = 1;

	// Enable interrupts
	IE_EA = 1;

	u2f_print("U2F ZERO\r\n");


	while (1) {

		if (!test)
		{
			run_tests();
			test = 1;
		}

		if (ms_since(ms_heart,500))
		{
			u2f_print("ms %lu\r\n", get_ms());
			LED_G = !LED_G;

		}


		if ( USBD_GetUsbState() == USBD_STATE_CONFIGURED)
		{
			if (!USBD_EpIsBusy(EP1OUT))
			{
				listen_for_pkt(&appdata);
				u2f_print("read added\r\n");
			}

		}

		switch(appdata.state)
		{
			case APP_NOTHING:
				break;
			case APP_WINK:
				LED_B = 0;
				ms_wink = get_ms();
				appdata.state = _APP_WINK;
				break;
			case _APP_WINK:
				if (ms_since(ms_wink,150))
				{
					LED_B = !LED_B;
					winks++;
				}
				if (winks == 5)
				{
					winks = 0;
					appdata.state = APP_NOTHING;
				}

				break;
		}

		flush_messages();

	}
}


