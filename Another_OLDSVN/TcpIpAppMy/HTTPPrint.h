/**************************************************************
 * HTTPPrint.h
 * Provides callback headers and resolution for user's custom
 * HTTP Application.
 * 
 * This file is automatically generated by the MPFS Utility
 * ALL MODIFICATIONS WILL BE OVERWRITTEN BY THE MPFS GENERATOR
 **************************************************************/

#ifndef __HTTPPRINT_H
#define __HTTPPRINT_H

#include "TCPIP Stack/TCPIP.h"

#if defined(STACK_USE_HTTP2_SERVER)

extern HTTP_CONN curHTTP;
extern HTTP_STUB httpStubs[MAX_HTTP_CONNECTIONS];
extern BYTE curHTTPID;

void HTTPPrint(DWORD callbackID);
void HTTPPrint_hellomsg(void);
void HTTPPrint_builddate(void);
void HTTPPrint_led(WORD);
void HTTPPrint_version(void);
void HTTPPrint_btn(WORD);
void HTTPPrint_status_fail(void);
void HTTPPrint_config_mac(void);
void HTTPPrint_config_hostname(void);
void HTTPPrint_config_dhcpchecked(void);
void HTTPPrint_config_ip(void);
void HTTPPrint_config_gw(void);
void HTTPPrint_config_subnet(void);
void HTTPPrint_config_dns1(void);
void HTTPPrint_config_dns2(void);
void HTTPPrint_reboot(void);
void HTTPPrint_rebootaddr(void);
void HTTPPrint_pot(WORD);
void HTTPPrint_time(WORD);
void HTTPPrint_name(void);
void HTTPPrint_pass(void);
void HTTPPrint_reader(WORD);
void HTTPPrint_card_position(void);
void HTTPPrint_facility_code(void);
void HTTPPrint_card_code(void);
void HTTPPrint_card_list(void);
void HTTPPrint_var_name(void);
void HTTPPrint_var_val(void);
void HTTPPrint_btn_change(WORD);
void HTTPPrint_host_cl(void);
void HTTPPrint_port_cl(void);

void HTTPPrint(DWORD callbackID)
{
	switch(callbackID)
	{
        case 0x00000001:
			HTTPPrint_hellomsg();
			break;
        case 0x00000005:
			HTTPPrint_builddate();
			break;
        case 0x00000006:
			HTTPPrint_led(7);
			break;
        case 0x00000007:
			HTTPPrint_led(6);
			break;
        case 0x00000008:
			HTTPPrint_led(5);
			break;
        case 0x00000009:
			HTTPPrint_led(4);
			break;
        case 0x0000000a:
			HTTPPrint_led(3);
			break;
        case 0x0000000b:
			HTTPPrint_led(2);
			break;
        case 0x0000000c:
			HTTPPrint_led(1);
			break;
        case 0x00000016:
			HTTPPrint_version();
			break;
        case 0x00000017:
			HTTPPrint_led(0);
			break;
        case 0x00000018:
			HTTPPrint_btn(0);
			break;
        case 0x00000019:
			HTTPPrint_btn(1);
			break;
        case 0x0000001a:
			HTTPPrint_btn(2);
			break;
        case 0x0000001b:
			HTTPPrint_btn(3);
			break;
        case 0x00000024:
			HTTPPrint_status_fail();
			break;
        case 0x00000025:
			HTTPPrint_config_mac();
			break;
        case 0x00000026:
			HTTPPrint_config_hostname();
			break;
        case 0x00000027:
			HTTPPrint_config_dhcpchecked();
			break;
        case 0x00000028:
			HTTPPrint_config_ip();
			break;
        case 0x00000029:
			HTTPPrint_config_gw();
			break;
        case 0x0000002a:
			HTTPPrint_config_subnet();
			break;
        case 0x0000002b:
			HTTPPrint_config_dns1();
			break;
        case 0x0000002c:
			HTTPPrint_config_dns2();
			break;
        case 0x0000002d:
			HTTPPrint_reboot();
			break;
        case 0x0000002e:
			HTTPPrint_rebootaddr();
			break;
        case 0x0000004a:
			HTTPPrint_pot(0);
			break;
        case 0x0000004b:
			HTTPPrint_pot(1);
			break;
        case 0x0000004c:
			HTTPPrint_pot(2);
			break;
        case 0x0000004f:
			HTTPPrint_time(0);
			break;
        case 0x0000005d:
			HTTPPrint_name();
			break;
        case 0x0000005e:
			HTTPPrint_pass();
			break;
        case 0x0000005f:
			HTTPPrint_reader(0);
			break;
        case 0x00000060:
			HTTPPrint_card_position();
			break;
        case 0x00000061:
			HTTPPrint_facility_code();
			break;
        case 0x00000062:
			HTTPPrint_card_code();
			break;
        case 0x00000063:
			HTTPPrint_card_list();
			break;
        case 0x00000064:
			HTTPPrint_var_name();
			break;
        case 0x00000065:
			HTTPPrint_var_val();
			break;
        case 0x00000066:
			HTTPPrint_btn_change(0);
			break;
        case 0x00000067:
			HTTPPrint_btn_change(1);
			break;
        case 0x00000068:
			HTTPPrint_btn_change(2);
			break;
        case 0x00000069:
			HTTPPrint_btn_change(3);
			break;
        case 0x0000006a:
			HTTPPrint_host_cl();
			break;
        case 0x0000006b:
			HTTPPrint_port_cl();
			break;
		default:
			// Output notification for undefined values
			TCPPutROMArray(sktHTTP, (ROM BYTE*)"!DEF", 4);
	}

	return;
}

void HTTPPrint_(void)
{
	TCPPut(sktHTTP, '~');
	return;
}

#endif

#endif
