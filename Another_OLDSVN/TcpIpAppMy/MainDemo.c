/*********************************************************************
 *
 *  Main Application Entry Point and TCP/IP Stack Demo
 *  Module for Microchip TCP/IP Stack
 *   -Demonstrates how to call and use the Microchip TCP/IP stack
 *	 -Reference: AN833
 *
 *********************************************************************
 * FileName:        MainDemo.c
 * Dependencies:    TCPIP.h
 * Processor:       PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F, PIC32
 * Compiler:        Microchip C32 v1.05 or higher
 *					Microchip C30 v3.12 or higher
 *					Microchip C18 v3.30 or higher
 *					HI-TECH PICC-18 PRO 9.63PL2 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright (C) 2002-2010 Microchip Technology Inc.  All rights
 * reserved.
 *
 * Microchip licenses to you the right to use, modify, copy, and
 * distribute:
 * (i)  the Software when embedded on a Microchip microcontroller or
 *      digital signal controller product ("Device") which is
 *      integrated into Licensee's product; or
 * (ii) ONLY the Software driver source files ENC28J60.c, ENC28J60.h,
 *		ENCX24J600.c and ENCX24J600.h ported to a non-Microchip device
 *		used in conjunction with a Microchip ethernet controller for
 *		the sole purpose of interfacing with the ethernet controller.
 *
 * You should refer to the license agreement accompanying this
 * Software for additional information regarding your rights and
 * obligations.
 *
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 *
 * Author              Date         Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Nilesh Rajbharti		4/19/01		Original (Rev. 1.0)
 * Nilesh Rajbharti		2/09/02		Cleanup
 * Nilesh Rajbharti		5/22/02		Rev 2.0 (See version.log for detail)
 * Nilesh Rajbharti		7/9/02		Rev 2.1 (See version.log for detail)
 * Nilesh Rajbharti		4/7/03		Rev 2.11.01 (See version log for detail)
 * Howard Schlunder		10/1/04		Beta Rev 0.9 (See version log for detail)
 * Howard Schlunder		10/8/04		Beta Rev 0.9.1 Announce support added
 * Howard Schlunder		11/29/04	Beta Rev 0.9.2 (See version log for detail)
 * Howard Schlunder		2/10/05		Rev 2.5.0
 * Howard Schlunder		1/5/06		Rev 3.00
 * Howard Schlunder		1/18/06		Rev 3.01 ENC28J60 fixes to TCP, 
 *									UDP and ENC28J60 files
 * Howard Schlunder		3/01/06		Rev. 3.16 including 16-bit micro support
 * Howard Schlunder		4/12/06		Rev. 3.50 added LCD for Explorer 16
 * Howard Schlunder		6/19/06		Rev. 3.60 finished dsPIC30F support, added PICDEM.net 2 support
 * Howard Schlunder		8/02/06		Rev. 3.75 added beta DNS, NBNS, and HTTP client (GenericTCPClient.c) services
 * Howard Schlunder		12/28/06	Rev. 4.00RC added SMTP, Telnet, substantially modified TCP layer
 * Howard Schlunder		04/09/07	Rev. 4.02 added TCPPerformanceTest, UDPPerformanceTest, Reboot and fixed some bugs
 * Howard Schlunder		xx/xx/07	Rev. 4.03
 * HSchlunder & EWood	08/27/07	Rev. 4.11
 * HSchlunder & EWood	10/08/07	Rev. 4.13
 * HSchlunder & EWood	11/06/07	Rev. 4.16
 * HSchlunder & EWood	11/08/07	Rev. 4.17
 * HSchlunder & EWood	11/12/07	Rev. 4.18
 * HSchlunder & EWood	02/11/08	Rev. 4.19
 * HSchlunder & EWood   04/26/08    Rev. 4.50 Moved most code to other files for clarity
 * KHesky               07/07/08    Added ZG2100-specific support
 * HSchlunder & EWood   07/24/08    Rev. 4.51
 * Howard Schlunder		11/10/08    Rev. 4.55
 * Howard Schlunder		04/14/09    Rev. 5.00
 * Howard Schlunder		07/10/09    Rev. 5.10
 * Howard Schlunder		11/18/09    Rev. 5.20
 * Howard Schlunder		04/28/10    Rev. 5.25
 ********************************************************************/
/*
 * This macro uniquely defines this file as the main entry point.
 * There should only be one such definition in the entire project,
 * and this file must define the AppConfig variable as described below.
 */
#define THIS_IS_STACK_APPLICATION

// Include all headers for any enabled TCPIP Stack functions
#include "TCPIP Stack/TCPIP.h"

#if defined(STACK_USE_ZEROCONF_LINK_LOCAL)
#include "TCPIP Stack/ZeroconfLinkLocal.h"
#endif
#if defined(STACK_USE_ZEROCONF_MDNS_SD)
#include "TCPIP Stack/ZeroconfMulticastDNS.h"
#endif

// Include functions specific to this stack application
#include "MainDemo.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#pragma udata temp_buffer
BYTE temp_buffer[64];
#pragma udata
// Declare AppConfig structure and some other supporting stack variables
APP_CONFIG AppConfig;
BYTE AN0String[ANALOG_INPUTS][ANALOG_LEN];

DWORD dwInternalTicks1 = 0;
WORD vTick1Reading[4] = {0,0,0,0};
DWORD g_LedBlinkCount = 0;


static DWORD cur_time = 0;

BYTE m_cur_button_states[4];
BYTE m_button_change[4];


DWORD timeGetTime(void)
{
	return cur_time;
}


// Use UART2 instead of UART1 for stdout (printf functions).  Explorer 16 
// serial port hardware is on PIC UART2 module.
#if defined(EXPLORER_16) || defined(PIC24FJ256DA210_DEV_BOARD)
	int __C30_UART = 2;
#endif


// Private helper functions.
// These may or may not be present in all applications.
static void InitAppConfig(void);
static void InitAppSettings(void);
static void InitializeBoard(void);

//
// PIC18 Interrupt Service Routines
// 
// NOTE: Several PICs, including the PIC18F4620 revision A3 have a RETFIE FAST/MOVFF bug
// The interruptlow keyword is used to work around the bug when using C18
#if defined(__18CXX)
	#if defined(HI_TECH_C)
	void interrupt low_priority LowISR(void)
	#else
	#pragma interruptlow LowISR
	void LowISR(void)
	#endif
	{
		ProcessReader();
	    TickUpdate();
		Tick1Update();
	}
	
	#if defined(HI_TECH_C)
	void interrupt HighISR(void)
	#else
	#pragma interruptlow HighISR
	void HighISR(void)
	#endif
	{
	    #if defined(STACK_USE_UART2TCP_BRIDGE)
			UART2TCPBridgeISR();
		#endif

		#if defined(WF_CS_TRIS)
			WFEintISR();
		#endif // WF_CS_TRIS
	}

	#if !defined(HI_TECH_C)
	#pragma code lowVector=0x18
	void LowVector(void){_asm goto LowISR _endasm}
	#pragma code highVector=0x8
	void HighVector(void){_asm goto HighISR _endasm}
	#pragma code // Return to default code section
	#endif

// C30 and C32 Exception Handlers
// If your code gets here, you either tried to read or write
// a NULL pointer, or your application overflowed the stack
// by having too many local variables or parameters declared.
#elif defined(__C30__)
	void _ISR __attribute__((__no_auto_psv__)) _AddressError(void)
	{
	    Nop();
		Nop();
	}
	void _ISR __attribute__((__no_auto_psv__)) _StackError(void)
	{
	    Nop();
		Nop();
	}
	
#elif defined(__C32__)
	void _general_exception_handler(unsigned cause, unsigned status)
	{
		Nop();
		Nop();
	}
#endif

void ProcessReader(void)
{
	if(INTCON3bits.INT1IF)
	{
		ProcessReadData(0);
		INTCON3bits.INT1IF = 0;
	}
	if(INTCON3bits.INT2IF)
	{
		ProcessReadData(1);
		INTCON3bits.INT2IF = 0;
	}
	if(INTCON3bits.INT3IF)
	{
		ProcessSensor(1);
		INTCON3bits.INT3IF = 0;
	}
}
void ProcessSensor(BYTE bOpen)
{
	if(m_skd_data.sensor != bOpen)
	{
		m_skd_data.sensor = bOpen;
		if(bOpen)
		{
			if(m_skd_data.reader_state & READER_OPEN)
			{
				return;
			}
			m_skd_data_to_send.sensor_open_timestamp = timeGetTime();
			m_skd_data.reader_state|=READER_OPEN;
		}
		else
		{
			m_skd_data_to_send.sensor_close_timestamp = timeGetTime();
			m_skd_data.reader_state|=READER_CLOSE;
			if(m_autonom)
			{
				//���� ���������� �����, �� ���� �������, ���� ���������������� ���������
				m_skd_data.reader_state|=READER_READY;
				m_skd_data.reader_state&=~READER_OPEN;
				m_skd_data.reader_state&=~READER_CLOSE;
				m_skd_data.reader_state&=~READER_KEY;
			}
			ProcessRELAY(0);
		}
	}
}

void ProcessRELAY(BYTE bOpen)
{
	if(bOpen)//relay command came
	{
		m_skd_data.relay_open_counter = timeGetTime();
		RELAY = 1;
		m_skd_data.reader_state|=READER_RELAY;
	}
	else
	{
		m_skd_data.relay_open_counter = 0;
		m_skd_data.reader_state&=~READER_RELAY;
	}
}

void ProcessRTE(BYTE bButtonPushed)
{
	if(bButtonPushed)//Pushed
	{
		m_skd_data.rte_open_counter = timeGetTime();
		m_skd_data_to_send.rte_push_timestamp = m_skd_data.rte_open_counter;
		RELAY = 1;
		m_skd_data.reader_state|=READER_RTE;
	}
	else//Pulled
	{
		m_skd_data.rte_open_counter = 0;
		m_skd_data.reader_state&=~READER_RTE;
	}
}

void ProcessTimeOuts(void)
{
	if(m_skd_data.relay_open_counter > 0)
	{
		if( (timeGetTime() - m_skd_data.relay_open_counter) > m_skd_settings.relay_open_time*TICK_SECOND)
		{
			ProcessRELAY(0);
		}
	}

	if(m_skd_data.rte_open_counter > 0)
	{
		if( (timeGetTime() - m_skd_data.rte_open_counter) > m_skd_settings.rte_open_time*TICK_SECOND)
		{
			ProcessRTE(0);//����� ��������� ������, ������� ��������� "��������� ������"
		}
	}

	if( !(m_skd_data.reader_state&READER_RTE) && !(m_skd_data.reader_state&READER_RELAY))
	{
		RELAY = 0;
	}

	if(m_skd_data.key_read_counter > 0)
	{
		if( (timeGetTime() - m_skd_data.key_read_counter) > m_skd_settings.key_read_time*TICK_SECOND)
		{
			m_skd_data.key_read_counter = 0;

			m_skd_data.reader_state|=READER_READY;
			m_skd_data.reader_state&=~READER_OPEN;
			m_skd_data.reader_state&=~READER_CLOSE;
			m_skd_data.reader_state&=~READER_KEY;
		}
	}
}

void ProcessReadData(int now_bit)
{
	static DWORD t1 = 0;

    if((timeGetTime() - t1) > TICK_SECOND*2ul)
    {
      	m_skd_data.bit_counter = 0;
    }
	t1 = timeGetTime();

	if(!(m_skd_data.reader_state & READER_READY))
	{
		return;
	}
	if(m_skd_data.bit_counter == 0)
	{
		m_skd_data.wiegand26.Val = 0;
	}
     if(now_bit == 0)
     {
       	  m_skd_data.wiegand26.Val &= ~(edenica.Val << (31 - m_skd_data.bit_counter-6)); 
     }
     else
	{
          m_skd_data.wiegand26.Val |= (edenica.Val << (31 - m_skd_data.bit_counter-6)); 
	}
     m_skd_data.bit_counter++;
     if(m_skd_data.bit_counter == 26)
	{
		m_skd_data_to_send.key_new_timestamp = timeGetTime();
        m_skd_data.bit_counter = 0;
		m_skd_data.reader_state|=READER_KEY;
		m_skd_data.reader_state&=~READER_READY;
		m_skd_data.reader_state&=~READER_OPEN;
		m_skd_data.reader_state&=~READER_CLOSE;
		
		m_skd_data.key_read_counter = m_skd_data_to_send.key_new_timestamp;
	}
}

void Tick1Update(void)
{
    if(PIR1bits.TMR1IF)
    {
		// Increment internal high tick counter
		dwInternalTicks1++;
		vTick1Reading[0]+=2;
		if(vTick1Reading[0]>=60)
		{
			vTick1Reading[0] = 0;
			vTick1Reading[1]++;
		}
		if(vTick1Reading[1]>=60)
		{
			vTick1Reading[1] = 0;
			vTick1Reading[2]++;
		}
		if(vTick1Reading[2]>=24)
		{
			vTick1Reading[2] = 0;
			vTick1Reading[3]++;
		}
		// Reset interrupt flag
        PIR1bits.TMR1IF = 0;
    }
}

void TickInit1(void)
{
    // Timer0 on, 16-bit, internal timer, 1:256 prescalar
    T1CONbits.TMR1ON = 1;
    T1CONbits.TMR1CS = 1;
    T1CONbits.T1SYNC = 1;
    T1CONbits.T1OSCEN = 1;
    T1CONbits.T1CKPS0 = 0;
    T1CONbits.T1CKPS1 = 0;
    T1CONbits.T1RUN = 0;
    T1CONbits.RD16 = 0;

	// Set up the timer interrupt
	IPR1bits.TMR1IP = 0;		// Low priority
    PIR1bits.TMR1IF = 0;
    PIE1bits.TMR1IE = 1;		// Enable interrupt

    TMR1H = 0;
    TMR1L = 0;
}

void InitReaderCycle(void)
{
	memset(&m_skd_data,0,sizeof(m_skd_data));
	m_skd_data.reader_state|=READER_READY;
}
//
// Main application entry point.
//
#if defined(__18CXX)
void main(void)
#else
int main(void)
#endif
{
	static DWORD dwLastIP = 0;
	char p1[16];
	char p2[16];
//	DWORD dwtest = 0;
//	strcpypgm2ram(p2, "24799");
//	strcpypgm2ram(p1, "95");
//�������� ���������� 95 24799==000000BEC1BE==12501438
    //dwtest =Wiegand2TouchMemoryDword(p2,p1);
	// Initialize application specific hardware
	InitializeBoard();
	edenica.Val = 1;
	
	m_skd_settings.rte_open_time = 3;
	m_skd_settings.key_read_time = 5;
	InitReaderCycle();

	#if defined(USE_LCD)
	// Initialize and display the stack version on the LCD
	LCDInit();
	DelayMs(100);
	strcpypgm2ram((char*)LCDText, "TCPStack " TCPIP_STACK_VERSION "  "
		"                ");
	LCDUpdate();
	#endif
	// Initialize stack-related hardware components that may be 
	// required by the UART configuration routines
    TickInit();
	TickInit1();
	#if defined(STACK_USE_MPFS) || defined(STACK_USE_MPFS2)
	MPFSInit();
	#endif

	// Initialize Stack and application related NV variables into AppConfig.
	InitAppConfig();
	InitAppSettings();
	
	//SaveAppConfig();
    // Initiates board setup process if button is depressed 
	// on startup

    if(BUTTON0_IO == 0u)
    {
		SaveAppConfig(NULL);
		#if defined(STACK_USE_UART)
        DoUARTConfig();
		#endif
    	while(BUTTON0_IO == 0u)
		{
			LED_PUT(1);
		}
    }
	LoadAppConfig();
	// Initialize core stack layers (MAC, ARP, TCP, UDP) and
	// application modules (HTTP, SNMP, etc.)
    StackInit();

	// Initialize any application-specific modules or functions/
	// For this demo application, this only includes the
	// UART 2 TCP Bridge
	#if defined(STACK_USE_UART2TCP_BRIDGE)
	UART2TCPBridgeInit();
	#endif

	#if defined(STACK_USE_ZEROCONF_LINK_LOCAL)
    ZeroconfLLInitialize();
	#endif

	#if defined(STACK_USE_ZEROCONF_MDNS_SD)
	mDNSInitialize(MY_DEFAULT_HOST_NAME);
	mDNSServiceRegister(
		(const char *) "DemoWebServer",	// base name of the service
		"_http._tcp.local",			    // type of the service
		80,				                // TCP or UDP port, at which this service is available
		((const BYTE *)"path=/index.htm"),	// TXT info
		1,								    // auto rename the service when if needed
		NULL,							    // no callback function
		NULL							    // no application context
		);

    mDNSMulticastFilterRegister();			
	#endif

	// Now that all items are initialized, begin the co-operative
	// multitasking loop.  This infinite loop will continuously 
	// execute all stack-related tasks, as well as your own
	// application's functions.  Custom functions should be added
	// at the end of this loop.
    // Note that this is a "co-operative mult-tasking" mechanism
    // where every task performs its tasks (whether all in one shot
    // or part of it) and returns so that other tasks can do their
    // job.
    // If a task needs very long time to do its job, it must be broken
    // down into smaller pieces so that other tasks can have CPU time.
    while(1)
    {
        // Blink LED0 (right most one) every second.
        if(TickGet() - timeGetTime() >= TICK_SECOND/2ul)
        {
			g_LedBlinkCount++;
            cur_time = TickGet();
			LED_PUT(LED_GET()^ 1);
        }

        // This task performs normal stack task including checking
        // for incoming packet, type of packet and calling
        // appropriate stack entity to process it.
        StackTask();

        // This tasks invokes each of the core stack application tasks
        StackApplications();

        #if defined(STACK_USE_ZEROCONF_LINK_LOCAL)
		ZeroconfLLProcess();
        #endif

        #if defined(STACK_USE_ZEROCONF_MDNS_SD)
        mDNSProcess();
		// Use this function to exercise service update function
		// HTTPUpdateRecord();
        #endif

		// Process application specific tasks here.
		// For this demo app, this will include the Generic TCP 
		// client and servers, and the SNMP, Ping, and SNMP Trap
		// demos.  Following that, we will process any IO from
		// the inputs on the board itself.
		// Any custom modules or processing you need to do should
		// go here.
		#if defined(STACK_USE_GENERIC_TCP_CLIENT_EXAMPLE)
		GenericTCPClient();
		#endif
		
		#if defined(STACK_USE_GENERIC_TCP_SERVER_EXAMPLE)
		GenericTCPServer();
		#endif
		
		#if defined(STACK_USE_SMTP_CLIENT)
		SMTPDemo();
		#endif
		
		#if defined(STACK_USE_ICMP_CLIENT)
		PingDemo();
		#endif
		
		#if defined(STACK_USE_SNMP_SERVER) && !defined(SNMP_TRAP_DISABLED)
		//User should use one of the following SNMP demo
		// This routine demonstrates V1 or V2 trap formats with one variable binding.
		SNMPTrapDemo();
		#if defined(SNMP_STACK_USE_V2_TRAP)
		//This routine provides V2 format notifications with multiple (3) variable bindings
		//User should modify this routine to send v2 trap format notifications with the required varbinds.
		//SNMPV2TrapDemo();
		#endif 
		if(gSendTrapFlag)
			SNMPSendTrap();
		#endif
		
		#if defined(STACK_USE_BERKELEY_API)
		BerkeleyTCPClientDemo();
		BerkeleyTCPServerDemo();
		BerkeleyUDPClientDemo();
		#endif

		ProcessIO();

        // If the local IP address has changed (ex: due to DHCP lease change)
        // write the new IP address to the LCD display, UART, and Announce 
        // service
		if(dwLastIP != AppConfig.MyIPAddr.Val)
		{
			dwLastIP = AppConfig.MyIPAddr.Val;
			
			#if defined(STACK_USE_UART)
				putrsUART((ROM char*)"\r\nNew IP Address: ");
			#endif

			DisplayIPValue(AppConfig.MyIPAddr);

			#if defined(STACK_USE_UART)
				putrsUART((ROM char*)"\r\n");
			#endif


			#if defined(STACK_USE_ANNOUNCE)
				AnnounceIP();
			#endif

            #if defined(STACK_USE_ZEROCONF_MDNS_SD)
				mDNSFillHostRecord();
			#endif
		}
	}
}

// Writes an IP address to the LCD display and the UART as available
void DisplayIPValue(IP_ADDR IPVal)
{
//	printf("%u.%u.%u.%u", IPVal.v[0], IPVal.v[1], IPVal.v[2], IPVal.v[3]);
    BYTE IPDigit[4];
	BYTE i;
#ifdef USE_LCD
	BYTE j;
	BYTE LCDPos=16;
#endif

	for(i = 0; i < sizeof(IP_ADDR); i++)
	{
	    uitoa((WORD)IPVal.v[i], IPDigit);

		#if defined(STACK_USE_UART)
			putsUART((char *) IPDigit);
		#endif

		#ifdef USE_LCD
			for(j = 0; j < strlen((char*)IPDigit); j++)
			{
				LCDText[LCDPos++] = IPDigit[j];
			}
			if(i == sizeof(IP_ADDR)-1)
				break;
			LCDText[LCDPos++] = '.';
		#else
			if(i == sizeof(IP_ADDR)-1)
				break;
		#endif

		#if defined(STACK_USE_UART)
			while(BusyUART());
			WriteUART('.');
		#endif
	}

	#ifdef USE_LCD
		if(LCDPos < 32u)
			LCDText[LCDPos] = 0;
		LCDUpdate();
	#endif
}

// Processes A/D data from the potentiometer
void ProcessIO(void)
{
	if(SENSOR1 == 0)
	{
		ProcessSensor(0);
	}

	if(!RTE1)
	{//������ �������� �� ����� (������)
		ProcessRTE(1);
	}
	ProcessTimeOuts();

	

    // AN0 should already be set up as an analog input
    ADCON0bits.GO = 1;

    // Wait until A/D conversion is done
    while(ADCON0bits.GO);

	if(ADCON0 == 0x09)		// ADON, Channel 2
	{
	    // Convert 10-bit value into ASCII string
	    uitoa(*((WORD*)(&ADRESL)), AN0String[0]);
		ADCON0 = 0x0D;
	}
	else if(ADCON0 == 0x0D)// ADON, Channel 3
	{
		uitoa(*((WORD*)(&ADRESL)), AN0String[1]);
		ADCON0 = 0x11;
	}
	else if(ADCON0 == 0x11)// ADON, Channel 4
	{
		uitoa(*((WORD*)(&ADRESL)), AN0String[2]);
		ADCON0 = 0x09;
	}
	else
	{
		ADCON0 = 0x09;
	}

	if(m_cur_button_states[0] != BUTTON0_IO)
	{
		m_button_change[0] = 1;
	}
	if(m_cur_button_states[1] != BUTTON1_IO)
	{
		m_button_change[1] = 1;
	}
	if(m_cur_button_states[2] != BUTTON2_IO)
	{
		m_button_change[2] = 1;
	}
	if(m_cur_button_states[3] != BUTTON3_IO)
	{
		m_button_change[3] = 1;
	}
}


/****************************************************************************
  Function:
    static void InitializeBoard(void)

  Description:
    This routine initializes the hardware.  It is a generic initialization
    routine for many of the Microchip development boards, using definitions
    in HardwareProfile.h to determine specific initialization.

  Precondition:
    None

  Parameters:
    None - None

  Returns:
    None

  Remarks:
    None
  ***************************************************************************/
static void InitializeBoard(void)
{	
	// LEDs
	LED_TRIS  = 0;
	LED0_TRIS = 0;
	LED1_TRIS = 0;
	LED2_TRIS = 0;
	LED3_TRIS = 0;
	LED4_TRIS = 0;
	LED5_TRIS = 0;
	LED6_TRIS = 0;
	LED7_TRIS = 0;

	LED0_IO = 0;
	LED1_IO = 0;
	LED2_IO = 0;
	LED3_IO = 0;
	LED4_IO = 0;
	LED5_IO = 0;
	LED6_IO = 0;
	LED7_IO = 0;

	DATA0_TRIS = 1;
	DATA1_TRIS = 1;
	SENSOR1_TRIS = 1;
	RTE1_TRIS = 1;

	BUTTON0_TRIS = 1;
	BUTTON1_TRIS = 1;
	BUTTON2_TRIS = 1;
	BUTTON3_TRIS = 1;

	memset(m_button_change,0,sizeof(m_button_change));
	m_cur_button_states[0] = BUTTON0_IO;
	m_cur_button_states[1] = BUTTON1_IO;
	m_cur_button_states[2] = BUTTON2_IO;
	m_cur_button_states[3] = BUTTON3_IO;

	memset(&AN0String[0][0],0,sizeof(AN0String));

#if !defined(EXPLORER_16)	// Pin multiplexed with a button on EXPLORER_16 

#endif
	LED_PUT(0x00);

#if defined(__18CXX)
	// Enable 4x/5x/96MHz PLL on PIC18F87J10, PIC18F97J60, PIC18F87J50, etc.
    OSCTUNE = 0x40;

	// Set up analog features of PORTA

	// PICDEM.net 2 board has POT on AN2, Temp Sensor on AN3
	#if defined(PICDEMNET2)
		ADCON0 = 0x09;		// ADON, Channel 2
		ADCON1 = 0x0B;		// Vdd/Vss is +/-REF, AN0, AN1, AN2, AN3 are analog
	#elif defined(PICDEMZ)
		ADCON0 = 0x81;		// ADON, Channel 0, Fosc/32
		ADCON1 = 0x0F;		// Vdd/Vss is +/-REF, AN0, AN1, AN2, AN3 are all digital
	#elif defined(__18F87J11) || defined(_18F87J11) || defined(__18F87J50) || defined(_18F87J50)
		ADCON0 = 0x01;		// ADON, Channel 0, Vdd/Vss is +/-REF
		WDTCONbits.ADSHR = 1;
		ANCON0 = 0xFC;		// AN0 (POT) and AN1 (temp sensor) are anlog
		ANCON1 = 0xFF;
		WDTCONbits.ADSHR = 0;		
	#else
		//ADCON0 = 0x01;		// ADON, Channel 0
		//ADCON0 = 0x11;		// ADON, Channel 4
		//ADCON1 = 0x0A;		// Vdd/Vss is +/-REF, AN4 is analog
		ADCON0 = 0x0D;		// ADON, Channel 3
		ADCON1 = 0x0A;		// Vdd/Vss is +/-REF, AN0, AN1, AN2, AN3,4 are analog
	#endif
	ADCON2 = 0xBE;		// Right justify, 20TAD ACQ time, Fosc/64 (~21.0kHz)


	// Configure USART
    TXSTA = 0x20;
    RCSTA = 0x90;

	// See if we can use the high baud rate setting
	#if ((GetPeripheralClock()+2*BAUD_RATE)/BAUD_RATE/4 - 1) <= 255
		SPBRG = (GetPeripheralClock()+2*BAUD_RATE)/BAUD_RATE/4 - 1;
		TXSTAbits.BRGH = 1;
	#else	// Use the low baud rate setting
		SPBRG = (GetPeripheralClock()+8*BAUD_RATE)/BAUD_RATE/16 - 1;
	#endif


	// Enable Interrupts
	RCONbits.IPEN = 1;		// Enable interrupt priorities
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;

		
	
	//INTCON2bits.RBIP = 0;
	//INTCONbits.RBIF = 0;
	//INTCONbits.RBIE = 1;

	//INTCON2bits.INTEDG0 = 1;//Interrupt 1 - rising edge 0 - falling edge
	INTCON2bits.INTEDG1 = 0;
	INTCON2bits.INTEDG2 = 0;
	INTCON2bits.INTEDG3 = 1;

	INTCON3bits.INT1IP = 0;//0-LowPrior,1-high
	INTCON3bits.INT2IP = 0;
	INTCON2bits.INT3IP = 0;

	//INTCONbits.INT0IF = 0;
	INTCON3bits.INT1IF = 0;//clear interrupt bit
	INTCON3bits.INT2IF = 0;//clear interrupt bit
	INTCON3bits.INT3IF = 0;//clear interrupt bit
	
	//INTCONbits.INT0IE = 1;//Enable interrupt
	INTCON3bits.INT1IE = 1;//Enable interrupt
	INTCON3bits.INT2IE = 1;//Enable interrupt
	INTCON3bits.INT3IE = 1;//Enable interrupt

    // Enable internal PORTB pull-ups
    INTCON2bits.RBPU = 0;
    // Do a calibration A/D conversion
	#if defined(__18F87J10) || defined(__18F86J15) || defined(__18F86J10) || defined(__18F85J15) || defined(__18F85J10) || defined(__18F67J10) || defined(__18F66J15) || defined(__18F66J10) || defined(__18F65J15) || defined(__18F65J10) || defined(__18F97J60) || defined(__18F96J65) || defined(__18F96J60) || defined(__18F87J60) || defined(__18F86J65) || defined(__18F86J60) || defined(__18F67J60) || defined(__18F66J65) || defined(__18F66J60) || \
	     defined(_18F87J10) ||  defined(_18F86J15) || defined(_18F86J10)  ||  defined(_18F85J15) ||  defined(_18F85J10) ||  defined(_18F67J10) ||  defined(_18F66J15) ||  defined(_18F66J10) ||  defined(_18F65J15) ||  defined(_18F65J10) ||  defined(_18F97J60) ||  defined(_18F96J65) ||  defined(_18F96J60) ||  defined(_18F87J60) ||  defined(_18F86J65) ||  defined(_18F86J60) ||  defined(_18F67J60) ||  defined(_18F66J65) ||  defined(_18F66J60)
		ADCON0bits.ADCAL = 1;
	    ADCON0bits.GO = 1;
		while(ADCON0bits.GO);
		ADCON0bits.ADCAL = 0;
	#elif defined(__18F87J11) || defined(__18F86J16) || defined(__18F86J11) || defined(__18F67J11) || defined(__18F66J16) || defined(__18F66J11) || \
		   defined(_18F87J11) ||  defined(_18F86J16) ||  defined(_18F86J11) ||  defined(_18F67J11) ||  defined(_18F66J16) ||  defined(_18F66J11) || \
		  defined(__18F87J50) || defined(__18F86J55) || defined(__18F86J50) || defined(__18F67J50) || defined(__18F66J55) || defined(__18F66J50) || \
		   defined(_18F87J50) ||  defined(_18F86J55) ||  defined(_18F86J50) ||  defined(_18F67J50) ||  defined(_18F66J55) ||  defined(_18F66J50)
		ADCON1bits.ADCAL = 1;
	    ADCON0bits.GO = 1;
		while(ADCON0bits.GO);
		ADCON1bits.ADCAL = 0;
	#endif

#else	// 16-bit C30 and and 32-bit C32
	#if defined(__PIC32MX__)
	{
		// Enable multi-vectored interrupts
		INTEnableSystemMultiVectoredInt();
		
		// Enable optimal performance
		SYSTEMConfigPerformance(GetSystemClock());
		mOSCSetPBDIV(OSC_PB_DIV_1);				// Use 1:1 CPU Core:Peripheral clocks
		
		// Disable JTAG port so we get our I/O pins back, but first
		// wait 50ms so if you want to reprogram the part with 
		// JTAG, you'll still have a tiny window before JTAG goes away.
		// The PIC32 Starter Kit debuggers use JTAG and therefore must not 
		// disable JTAG.
		DelayMs(50);
		#if !defined(__MPLAB_DEBUGGER_PIC32MXSK) && !defined(__MPLAB_DEBUGGER_FS2)
			DDPCONbits.JTAGEN = 0;
		#endif
		LED_PUT(0x00);				// Turn the LEDs off
		
		CNPUESET = 0x00098000;		// Turn on weak pull ups on CN15, CN16, CN19 (RD5, RD7, RD13), which is connected to buttons on PIC32 Starter Kit boards
	}
	#endif

	#if defined(__dsPIC33F__) || defined(__PIC24H__)
		// Crank up the core frequency
		PLLFBD = 38;				// Multiply by 40 for 160MHz VCO output (8MHz XT oscillator)
		CLKDIV = 0x0000;			// FRC: divide by 2, PLLPOST: divide by 2, PLLPRE: divide by 2
	
		// Port I/O
		AD1PCFGHbits.PCFG23 = 1;	// Make RA7 (BUTTON1) a digital input
		AD1PCFGHbits.PCFG20 = 1;	// Make RA12 (INT1) a digital input for MRF24WB0M PICtail Plus interrupt

		// ADC
	    AD1CHS0 = 0;				// Input to AN0 (potentiometer)
		AD1PCFGLbits.PCFG5 = 0;		// Disable digital input on AN5 (potentiometer)
		AD1PCFGLbits.PCFG4 = 0;		// Disable digital input on AN4 (TC1047A temp sensor)
	#else	//defined(__PIC24F__) || defined(__PIC32MX__)
		#if defined(__PIC24F__)
			CLKDIVbits.RCDIV = 0;		// Set 1:1 8MHz FRC postscalar
		#endif
		
		// ADC
	    #if defined(__PIC24FJ256DA210__) || defined(__PIC24FJ256GB210__)
	    	// Disable analog on all pins
	    	ANSA = 0x0000;
	    	ANSB = 0x0000;
	    	ANSC = 0x0000;
	    	ANSD = 0x0000;
	    	ANSE = 0x0000;
	    	ANSF = 0x0000;
	    	ANSG = 0x0000;
		#else
		    AD1CHS = 0;					// Input to AN0 (potentiometer)
			AD1PCFGbits.PCFG4 = 0;		// Disable digital input on AN4 (TC1047A temp sensor)
			#if defined(__32MX460F512L__) || defined(__32MX795F512L__)	// PIC32MX460F512L and PIC32MX795F512L PIMs has different pinout to accomodate USB module
				AD1PCFGbits.PCFG2 = 0;		// Disable digital input on AN2 (potentiometer)
			#else
				AD1PCFGbits.PCFG5 = 0;		// Disable digital input on AN5 (potentiometer)
			#endif
		#endif
	#endif

	// ADC
	AD1CON1 = 0x84E4;			// Turn on, auto sample start, auto-convert, 12 bit mode (on parts with a 12bit A/D)
	AD1CON2 = 0x0404;			// AVdd, AVss, int every 2 conversions, MUXA only, scan
	AD1CON3 = 0x1003;			// 16 Tad auto-sample, Tad = 3*Tcy
	#if defined(__32MX460F512L__) || defined(__32MX795F512L__)	// PIC32MX460F512L and PIC32MX795F512L PIMs has different pinout to accomodate USB module
		AD1CSSL = 1<<2;				// Scan pot
	#else
		AD1CSSL = 1<<5;				// Scan pot
	#endif

	// UART
	#if defined(STACK_USE_UART)
		UARTTX_TRIS = 0;
		UARTRX_TRIS = 1;
		UMODE = 0x8000;			// Set UARTEN.  Note: this must be done before setting UTXEN

		#if defined(__C30__)
			USTA = 0x0400;		// UTXEN set
			#define CLOSEST_UBRG_VALUE ((GetPeripheralClock()+8ul*BAUD_RATE)/16/BAUD_RATE-1)
			#define BAUD_ACTUAL (GetPeripheralClock()/16/(CLOSEST_UBRG_VALUE+1))
		#else	//defined(__C32__)
			USTA = 0x00001400;		// RXEN set, TXEN set
			#define CLOSEST_UBRG_VALUE ((GetPeripheralClock()+8ul*BAUD_RATE)/16/BAUD_RATE-1)
			#define BAUD_ACTUAL (GetPeripheralClock()/16/(CLOSEST_UBRG_VALUE+1))
		#endif
	
		#define BAUD_ERROR ((BAUD_ACTUAL > BAUD_RATE) ? BAUD_ACTUAL-BAUD_RATE : BAUD_RATE-BAUD_ACTUAL)
		#define BAUD_ERROR_PRECENT	((BAUD_ERROR*100+BAUD_RATE/2)/BAUD_RATE)
		#if (BAUD_ERROR_PRECENT > 3)
			#warning UART frequency error is worse than 3%
		#elif (BAUD_ERROR_PRECENT > 2)
			#warning UART frequency error is worse than 2%
		#endif
	
		UBRG = CLOSEST_UBRG_VALUE;
	#endif

#endif

// Deassert all chip select lines so there isn't any problem with 
// initialization order.  Ex: When ENC28J60 is on SPI2 with Explorer 16, 
// MAX3232 ROUT2 pin will drive RF12/U2CTS ENC28J60 CS line asserted, 
// preventing proper 25LC256 EEPROM operation.
#if defined(ENC_CS_TRIS)
	ENC_CS_IO = 1;
	ENC_CS_TRIS = 0;
#endif
#if defined(ENC100_CS_TRIS)
	ENC100_CS_IO = (ENC100_INTERFACE_MODE == 0);
	ENC100_CS_TRIS = 0;
#endif
#if defined(EEPROM_CS_TRIS)
	EEPROM_CS_IO = 1;
	EEPROM_CS_TRIS = 0;
#endif
#if defined(SPIRAM_CS_TRIS)
	SPIRAM_CS_IO = 1;
	SPIRAM_CS_TRIS = 0;
#endif
#if defined(SPIFLASH_CS_TRIS)
	SPIFLASH_CS_IO = 1;
	SPIFLASH_CS_TRIS = 0;
#endif
#if defined(WF_CS_TRIS)
	WF_CS_IO = 1;
	WF_CS_TRIS = 0;
#endif

#if defined(PIC24FJ64GA004_PIM)
	__builtin_write_OSCCONL(OSCCON & 0xBF);  // Unlock PPS

	// Remove some LED outputs to regain other functions
	LED1_TRIS = 1;		// Multiplexed with BUTTON0
	LED5_TRIS = 1;		// Multiplexed with EEPROM CS
	LED7_TRIS = 1;		// Multiplexed with BUTTON1
	
	// Inputs
	RPINR19bits.U2RXR = 19;			//U2RX = RP19
	RPINR22bits.SDI2R = 20;			//SDI2 = RP20
	RPINR20bits.SDI1R = 17;			//SDI1 = RP17
	
	// Outputs
	RPOR12bits.RP25R = U2TX_IO;		//RP25 = U2TX  
	RPOR12bits.RP24R = SCK2OUT_IO; 	//RP24 = SCK2
	RPOR10bits.RP21R = SDO2_IO;		//RP21 = SDO2
	RPOR7bits.RP15R = SCK1OUT_IO; 	//RP15 = SCK1
	RPOR8bits.RP16R = SDO1_IO;		//RP16 = SDO1
	
	AD1PCFG = 0xFFFF;				//All digital inputs - POT and Temp are on same pin as SDO1/SDI1, which is needed for ENC28J60 commnications

	__builtin_write_OSCCONL(OSCCON | 0x40); // Lock PPS
#endif

#if defined(__PIC24FJ256DA210__)
	__builtin_write_OSCCONL(OSCCON & 0xBF);  // Unlock PPS

	// Inputs
	RPINR19bits.U2RXR = 11;	// U2RX = RP11
	RPINR20bits.SDI1R = 0;	// SDI1 = RP0
	RPINR0bits.INT1R = 34;	// Assign RE9/RPI34 to INT1 (input) for MRF24WB0M Wi-Fi PICtail Plus interrupt
	
	// Outputs
	RPOR8bits.RP16R = 5;	// RP16 = U2TX
	RPOR1bits.RP2R = 8; 	// RP2 = SCK1
	RPOR0bits.RP1R = 7;		// RP1 = SDO1

	__builtin_write_OSCCONL(OSCCON | 0x40); // Lock PPS
#endif

#if defined(__PIC24FJ256GB110__) || defined(__PIC24FJ256GB210__)
	__builtin_write_OSCCONL(OSCCON & 0xBF);  // Unlock PPS
	
	// Configure SPI1 PPS pins (ENC28J60/ENCX24J600/MRF24WB0M or other PICtail Plus cards)
	RPOR0bits.RP0R = 8;		// Assign RP0 to SCK1 (output)
	RPOR7bits.RP15R = 7;	// Assign RP15 to SDO1 (output)
	RPINR20bits.SDI1R = 23;	// Assign RP23 to SDI1 (input)

	// Configure SPI2 PPS pins (25LC256 EEPROM on Explorer 16)
	RPOR10bits.RP21R = 11;	// Assign RG6/RP21 to SCK2 (output)
	RPOR9bits.RP19R = 10;	// Assign RG8/RP19 to SDO2 (output)
	RPINR22bits.SDI2R = 26;	// Assign RG7/RP26 to SDI2 (input)
	
	// Configure UART2 PPS pins (MAX3232 on Explorer 16)
	#if !defined(ENC100_INTERFACE_MODE) || (ENC100_INTERFACE_MODE == 0) || defined(ENC100_PSP_USE_INDIRECT_RAM_ADDRESSING)
	RPINR19bits.U2RXR = 10;	// Assign RF4/RP10 to U2RX (input)
	RPOR8bits.RP17R = 5;	// Assign RF5/RP17 to U2TX (output)
	#endif
	
	// Configure INT1 PPS pin (MRF24WB0M Wi-Fi PICtail Plus interrupt signal when in SPI slot 1)
	RPINR0bits.INT1R = 33;	// Assign RE8/RPI33 to INT1 (input)

	// Configure INT3 PPS pin (MRF24WB0M Wi-Fi PICtail Plus interrupt signal when in SPI slot 2)
	RPINR1bits.INT3R = 40;	// Assign RC3/RPI40 to INT3 (input)

	__builtin_write_OSCCONL(OSCCON | 0x40); // Lock PPS
#endif

#if defined(__PIC24FJ256GA110__)
	__builtin_write_OSCCONL(OSCCON & 0xBF);  // Unlock PPS
	
	// Configure SPI2 PPS pins (25LC256 EEPROM on Explorer 16 and ENC28J60/ENCX24J600/MRF24WB0M or other PICtail Plus cards)
	// Note that the ENC28J60/ENCX24J600/MRF24WB0M PICtails SPI PICtails must be inserted into the middle SPI2 socket, not the topmost SPI1 slot as normal.  This is because PIC24FJ256GA110 A3 silicon has an input-only RPI PPS pin in the ordinary SCK1 location.  Silicon rev A5 has this fixed, but for simplicity all demos will assume we are using SPI2.
	RPOR10bits.RP21R = 11;	// Assign RG6/RP21 to SCK2 (output)
	RPOR9bits.RP19R = 10;	// Assign RG8/RP19 to SDO2 (output)
	RPINR22bits.SDI2R = 26;	// Assign RG7/RP26 to SDI2 (input)
	
	// Configure UART2 PPS pins (MAX3232 on Explorer 16)
	RPINR19bits.U2RXR = 10;	// Assign RF4/RP10 to U2RX (input)
	RPOR8bits.RP17R = 5;	// Assign RF5/RP17 to U2TX (output)
	
	// Configure INT3 PPS pin (MRF24WB0M PICtail Plus interrupt signal)
	RPINR1bits.INT3R = 36;	// Assign RA14/RPI36 to INT3 (input)

	__builtin_write_OSCCONL(OSCCON | 0x40); // Lock PPS
#endif


#if defined(DSPICDEM11)
	// Deselect the LCD controller (PIC18F252 onboard) to ensure there is no SPI2 contention
	LCDCTRL_CS_TRIS = 0;
	LCDCTRL_CS_IO = 1;

	// Hold the codec in reset to ensure there is no SPI2 contention
	CODEC_RST_TRIS = 0;
	CODEC_RST_IO = 0;
#endif

#if defined(SPIRAM_CS_TRIS)
	SPIRAMInit();
#endif
#if defined(EEPROM_CS_TRIS)
	XEEInit();
#endif
#if defined(SPIFLASH_CS_TRIS)
	SPIFlashInit();
#endif
}

/*********************************************************************
 * Function:        void InitAppConfig(void)
 *
 * PreCondition:    MPFSInit() is already called.
 *
 * Input:           None
 *
 * Output:          Write/Read non-volatile config variables.
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
// MAC Address Serialization using a MPLAB PM3 Programmer and 
// Serialized Quick Turn Programming (SQTP). 
// The advantage of using SQTP for programming the MAC Address is it
// allows you to auto-increment the MAC address without recompiling 
// the code for each unit.  To use SQTP, the MAC address must be fixed
// at a specific location in program memory.  Uncomment these two pragmas
// that locate the MAC address at 0x1FFF0.  Syntax below is for MPLAB C 
// Compiler for PIC18 MCUs. Syntax will vary for other compilers.
//#pragma romdata MACROM=0x1FFF0
static ROM BYTE SerializedMACAddress[6] = {MY_DEFAULT_MAC_BYTE1, MY_DEFAULT_MAC_BYTE2, MY_DEFAULT_MAC_BYTE3, MY_DEFAULT_MAC_BYTE4, MY_DEFAULT_MAC_BYTE5, MY_DEFAULT_MAC_BYTE6};
//#pragma romdata

static void InitAppConfig(void)
{
	AppConfig.Flags.bIsDHCPEnabled = TRUE;
	AppConfig.Flags.bInConfigMode = TRUE;
	memcpypgm2ram((void*)&AppConfig.MyMACAddr, (ROM void*)SerializedMACAddress, sizeof(AppConfig.MyMACAddr));
//	{
//		_prog_addressT MACAddressAddress;
//		MACAddressAddress.next = 0x157F8;
//		_memcpy_p2d24((char*)&AppConfig.MyMACAddr, MACAddressAddress, sizeof(AppConfig.MyMACAddr));
//	}
	AppConfig.MyIPAddr.Val = MY_DEFAULT_IP_ADDR_BYTE1 | MY_DEFAULT_IP_ADDR_BYTE2<<8ul | MY_DEFAULT_IP_ADDR_BYTE3<<16ul | MY_DEFAULT_IP_ADDR_BYTE4<<24ul;
	AppConfig.DefaultIPAddr.Val = AppConfig.MyIPAddr.Val;
	AppConfig.MyMask.Val = MY_DEFAULT_MASK_BYTE1 | MY_DEFAULT_MASK_BYTE2<<8ul | MY_DEFAULT_MASK_BYTE3<<16ul | MY_DEFAULT_MASK_BYTE4<<24ul;
	AppConfig.DefaultMask.Val = AppConfig.MyMask.Val;
	AppConfig.MyGateway.Val = MY_DEFAULT_GATE_BYTE1 | MY_DEFAULT_GATE_BYTE2<<8ul | MY_DEFAULT_GATE_BYTE3<<16ul | MY_DEFAULT_GATE_BYTE4<<24ul;
	AppConfig.PrimaryDNSServer.Val = MY_DEFAULT_PRIMARY_DNS_BYTE1 | MY_DEFAULT_PRIMARY_DNS_BYTE2<<8ul  | MY_DEFAULT_PRIMARY_DNS_BYTE3<<16ul  | MY_DEFAULT_PRIMARY_DNS_BYTE4<<24ul;
	AppConfig.SecondaryDNSServer.Val = MY_DEFAULT_SECONDARY_DNS_BYTE1 | MY_DEFAULT_SECONDARY_DNS_BYTE2<<8ul  | MY_DEFAULT_SECONDARY_DNS_BYTE3<<16ul  | MY_DEFAULT_SECONDARY_DNS_BYTE4<<24ul;


	// SNMP Community String configuration
	#if defined(STACK_USE_SNMP_SERVER)
	{
		BYTE i;
		static ROM char * ROM cReadCommunities[] = SNMP_READ_COMMUNITIES;
		static ROM char * ROM cWriteCommunities[] = SNMP_WRITE_COMMUNITIES;
		ROM char * strCommunity;
		
		for(i = 0; i < SNMP_MAX_COMMUNITY_SUPPORT; i++)
		{
			// Get a pointer to the next community string
			strCommunity = cReadCommunities[i];
			if(i >= sizeof(cReadCommunities)/sizeof(cReadCommunities[0]))
				strCommunity = "";

			// Ensure we don't buffer overflow.  If your code gets stuck here, 
			// it means your SNMP_COMMUNITY_MAX_LEN definition in TCPIPConfig.h 
			// is either too small or one of your community string lengths 
			// (SNMP_READ_COMMUNITIES) are too large.  Fix either.
			if(strlenpgm(strCommunity) >= sizeof(AppConfig.readCommunity[0]))
				while(1);
			
			// Copy string into AppConfig
			strcpypgm2ram((char*)AppConfig.readCommunity[i], strCommunity);

			// Get a pointer to the next community string
			strCommunity = cWriteCommunities[i];
			if(i >= sizeof(cWriteCommunities)/sizeof(cWriteCommunities[0]))
				strCommunity = "";

			// Ensure we don't buffer overflow.  If your code gets stuck here, 
			// it means your SNMP_COMMUNITY_MAX_LEN definition in TCPIPConfig.h 
			// is either too small or one of your community string lengths 
			// (SNMP_WRITE_COMMUNITIES) are too large.  Fix either.
			if(strlenpgm(strCommunity) >= sizeof(AppConfig.writeCommunity[0]))
				while(1);

			// Copy string into AppConfig
			strcpypgm2ram((char*)AppConfig.writeCommunity[i], strCommunity);
		}
	}
	#endif

	// Load the default NetBIOS Host Name
	memcpypgm2ram(AppConfig.NetBIOSName, (ROM void*)MY_DEFAULT_HOST_NAME, 16);
	FormatNetBIOSName(AppConfig.NetBIOSName);

	//if(!LoadAppConfig())
	{
		//SaveAppConfig();
	}

	#if defined(EEPROM_CS_TRIS)
	{
		BYTE c;
		
	    // When a record is saved, first byte is written as 0x60 to indicate
	    // that a valid record was saved.  Note that older stack versions
		// used 0x57.  This change has been made to so old EEPROM contents
		// will get overwritten.  The AppConfig() structure has been changed,
		// resulting in parameter misalignment if still using old EEPROM
		// contents.
		XEEReadArray(0x0000, &c, 1);
	    if(c == 0x60u)
		    XEEReadArray(0x0001, (BYTE*)&AppConfig, sizeof(AppConfig));
	    else
	        SaveAppConfig();
	}
	#elif defined(SPIFLASH_CS_TRIS)
	{
		BYTE c;
		
		SPIFlashReadArray(0x0000, &c, 1);
		if(c == 0x60u)
			SPIFlashReadArray(0x0001, (BYTE*)&AppConfig, sizeof(AppConfig));
		else
			SaveAppConfig();
	}
	#endif
}

//if defined(EEPROM_CS_TRIS) || defined(SPIFLASH_CS_TRIS)
void SaveAppConfig(BYTE* pAppConfig)
{
	int i = 0;
	int  filled = 0;
	if(pAppConfig == NULL)
	{
		pAppConfig = (BYTE*)&AppConfig;
	}

	temp_buffer_big[filled] = 0x60u;
	filled++;

	memcpy(temp_buffer_big + filled,(void*)pAppConfig,sizeof(AppConfig));
	filled+=sizeof(AppConfig);

	memcpy(temp_buffer_big + filled,(void*)&AutonomSettings,sizeof(AutonomSettings));
	filled+=sizeof(AutonomSettings);

	RawEraseFlashBlock((DWORD)&RomConfBuff[0]);
	for( i = 0; i < 256; i+=FLASH_WRITE_SIZE)
	{
		RawWriteFlashBlock((DWORD)&RomConfBuff[i], temp_buffer_big + i, NULL);
	}
}


BOOL LoadAppConfig(void)
{
    // When a record is saved, first byte is written as 0x60 to indicate
    // that a valid record was saved.  Note that older stack versions
	// used 0x57.  This change has been made to so old EEPROM contents
	// will get overwritten.  The AppConfig() structure has been changed,
	// resulting in parameter misalignment if still using old EEPROM
	// contents.
    if(RomConfBuff[0] == 0x60)
	{
		memcpypgm2ram (&AppConfig, (const rom void *)&RomConfBuff[1],sizeof(AppConfig));
		memcpypgm2ram (&AutonomSettings, (const rom void *)&RomConfBuff[1+sizeof(AppConfig)],sizeof(AutonomSettings));
		return TRUE;
	}
	return FALSE;
	 
}
//#endif


/******************************************************************************
* Function:        static void RawEraseFlashBlock(DWORD dwAddress)
*
* PreCondition:    None
*
* Input:           dwAddress: Begining byte address to start erasing at.  Note 
*							   that this address must be page aligned (ie: evenly 
*							   divisible by FLASH_ERASE_SIZE). 
*
* Output:          Erases the specified page, limited by the Flash 
*					technology's FLASH_ERASE_SIZE.
*
* Side Effects:    None
*
* Overview:        None
*
* Note:            This function is bootloader safe.  If an attempt to erase 
*					the addresses containing the bootloader is made, the 
*					function will not perform the erase.
*****************************************************************************/
void RawEraseFlashBlock(DWORD dwAddress)
{
	WORD w;
	// Guarantee that we make calculations using the real erase address 
	// supported by the hardware
	dwAddress &= ~(FLASH_ERASE_SIZE-1ul);


	// Blank checking and erasing can take considerable time
	ClrWdt();
	TBLPTR = dwAddress;
	for(w = 0; w < FLASH_ERASE_SIZE; w++)
	{
		_asm TBLRDPOSTINC _endasm

			// Check to see if any of the 8 bytes has data in it
			if(TABLAT != 0xFF)
			{
				// Erasing can take considerable time
				ClrWdt();

				// Data found, erase this page
				TBLPTR = dwAddress;
				EECON1bits.WREN = 1;
				EECON1bits.FREE = 1;
				INTCONbits.GIE = 0; 
				_asm
					movlw	0x55
					movwf	EECON2, ACCESS
					movlw	0xAA
					movwf	EECON2, ACCESS
					bsf		EECON1, 1, ACCESS	//WR
				_endasm
					EECON1bits.WREN = 0;
					INTCONbits.GIE = 1; 

				break;
			}
	}
	// Blank checking and erasing can take considerable time
	ClrWdt();
}

/******************************************************************************
* Function:        static BOOL RawWriteFlashBlock(DWORD Address, BYTE *BlockData, BYTE *StatusData)
*
* PreCondition:    None
*
* Input:           Address: Location to write to.  This address MUST be an 
*							 integer multiple of the FLASH_WRITE_SIZE constant.
*					*BlockData: Pointer to an array of bytes to write.  The 
*								array is assumed to be exactly FLASH_WRITE_SIZE 
*								in length (or longer).  If longer, only the 
*								first FLASH_WRITE_SIZE bytes will be written.  
*								You must call WriteFlashBlock() again with an 
*								updated Address to write more bytes.
*					*StatusData: Pointer to a byte array of size FLASH_WRITE_SIZE/8 
*								 which will be written with the results of the 
*								 write.  Each bit in the array represents one 
*								 data byte successfully or unsuccessfully 
*								 written.  If the bit is set, the write was 
*								 successful.  Pointer can be NULL if individual 
*								 byte status data is not needed.
*
* Output:          TRUE: If the write was successful or no write was needed 
*						  because the data was already matching.
*					FALSE: If the write failed either because some other data 
*						   was already in the memory (need to perform Erase 
*						   first), or if, following the write, the data did 
*						   not verify correctly (may have run out of Flash 
*						   Erase-Write cycles).
*
* Side Effects:    None
*
* Overview:        Writes one FLASH_WRITE_SIZE block of data to the Flash 
*					memory.  The memory must be erased prior to writing.  
*
* Note:            None
*****************************************************************************/
BOOL RawWriteFlashBlock(DWORD Address, BYTE *BlockData, BYTE *StatusData)
{
	BYTE i;
	WORD w;
	BOOL WriteNeeded;
	BOOL WriteAllowed;
	BOOL VerifyPassed;


	WriteNeeded = FALSE;
	WriteAllowed = TRUE;
	VerifyPassed = TRUE;

	// Load up the internal Flash holding registers in preperation for the write
	TBLPTR = Address;
	for(w = 0; w < FLASH_WRITE_SIZE; w++)
	{
		_asm TBLRD _endasm
			if(TABLAT != 0xFF)
				WriteAllowed = FALSE;
		i = BlockData[w];
		if(TABLAT != i)
		{
			TABLAT = i;
			WriteNeeded = TRUE;
		}
		_asm TBLWTPOSTINC _endasm
	}
	TBLPTR = Address;

	// Start out assuming that all bytes were written successfully
	if(StatusData != NULL)
		memset(StatusData, 0xFF, FLASH_WRITE_SIZE>>3);

	if(WriteNeeded)
	{
		// Perform the write
		if(WriteAllowed)
		{
			// Writing can take some real time
			ClrWdt();

			// Copy the holding registers into FLASH.  This takes approximately 2.8ms.
			EECON1bits.FREE = 0;
			EECON1bits.WREN = 1;
			INTCONbits.GIE = 0;
			_asm
				movlw	0x55
				movwf	EECON2, ACCESS
				movlw	0xAA
				movwf	EECON2, ACCESS
				bsf		EECON1, 1, ACCESS	//WR
			_endasm
				EECON1bits.WREN = 0;
				INTCONbits.GIE = 1;
			// Writing can take some real time
			ClrWdt();
		}

		// Verify that the write was performed successfully
		for(w = 0; w < FLASH_WRITE_SIZE; w++)
		{
			_asm TBLRDPOSTINC _endasm
				if(TABLAT != BlockData[w])
				{
					// Uhh oh.  Data is invalid.  
					// You need to perform an erase first 
					// if WriteAllowed is FALSE.
					// If WriteAllowed is TRUE maybe FLASH 
					// ran out of Erase/Write cycles 
					// (endurance problem).
					if(StatusData != NULL)
						StatusData[w>>3] &= ~(1<<(w&0x07));
					VerifyPassed = FALSE;
				}
		}

		return VerifyPassed;
	}

	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**************************************************
�*
�*   �ftoa - converts float to string
�*
�**************************************************/
�void ftoa(float f, char* str, int buf_len)
�{
	sprintf(str,"%d.%03u", (int)f , ((int) (((f)-(int)f) * 1000)) );
�}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

static void InitAppSettings(void)
{
	strcpypgm2ram((char*)AutonomSettings.data[E_NAME], "root");
	strcpypgm2ram((char*)AutonomSettings.data[E_PASS], "root");
	strcpypgm2ram((char*)AutonomSettings.data[E_HOST_CL], "192.168.0.103");
	strcpypgm2ram((char*)AutonomSettings.data[E_PORT_CL], "80");
}


BYTE* GetAsWiegand(DWORD card)
{

	long temp_int = 0;
	int len = 0;
	if(card == -1)
	{
		card = m_skd_data.wiegand26.Val;
	}
	memset(temp_buffer,0, 20);

	temp_int = (card >> 17) & 0xFF;
	sprintf(temp_buffer,"%li",temp_int);

	len = strlen(temp_buffer);

	temp_int = (card >> 1) & 0xFFFF;
	sprintf(temp_buffer+len," %li",temp_int);

	return temp_buffer;
}

BYTE* GetAsWiegandFacilityCode(void)
{
	long temp_int = 0;
	memset(temp_buffer,0, 20);
	temp_int = (m_skd_data.wiegand26.Val >> 17) & 0xFF;
	sprintf(temp_buffer,"%li",temp_int);
	return temp_buffer;
}
BYTE* GetAsWiegandCardCode(void)
{
	long temp_int = 0;
	temp_int = (m_skd_data.wiegand26.Val >> 1) & 0xFFFF;
	sprintf(temp_buffer,"%li",temp_int);
	return temp_buffer;
}
BYTE* GetAsTouchMem(void)
{
	int len = 0;
	memset(temp_buffer,0, 32);
	sprintf(temp_buffer,"%.8X",m_skd_data.wiegand26.word.HW);
	
	len = strlen(temp_buffer);
	
	sprintf(temp_buffer + len,"%.4X",m_skd_data.wiegand26.word.LW);
	
	return temp_buffer;
}


void SetLed(int channel,int val)
{
	if(channel == 0)
	{
		LED0_IO = (val);
	}
	if(channel == 1)
	{
		LED1_IO = (val);
	}
	if(channel == 2)
	{
		LED2_IO = (val);
	}
	if(channel == 3)
	{
		LED3_IO = (val);
	}
	if(channel == 4)
	{
		LED4_IO = (val);
	}
	if(channel == 5)
	{
		LED5_IO = (val);
	}
	if(channel == 6)
	{
		LED6_IO = (val);
	}
	if(channel == 7)
	{
		LED7_IO = (val);
	}
}