//***************************************/
#include <p18f2550.h>

#include "./includes/usb_hal_pic18.h"
#include "./includes/usb_device.h"

#include "HardwareProfile.h"

/** C O N F I G U R A T I O N   B I T S **************************************/
      // Configuration bits for PIC18F2550, 24 MHz crystal
#pragma config PLLDIV = 5, CPUDIV = OSC1_PLL2, USBDIV = 2                   // CONFIG1L
#pragma config FOSC =HSPLL_HS, FCMEN = OFF, IESO = OFF 	                // CONFIG1H
#pragma config PWRT = ON, BOR = ON, BORV = 3, VREGEN = ON                   // CONFIG2L
#pragma config WDT = OFF, WDTPS = 32768                                     // CONFIG2H
#pragma config CCP2MX = ON, PBADEN = OFF, LPT1OSC = OFF, MCLRE = OFF        // CONFIG3H
#pragma config STVREN = OFF, LVP = OFF, XINST = OFF, DEBUG = OFF            // CONFIG4L
#pragma config CP0 = OFF, CP1 = OFF, CP2 = OFF, CP3 = OFF                   // CONFIG5L
#pragma config CPB = OFF, CPD = OFF                                         // CONFIG5H
#pragma config WRT0 = OFF, WRT1 = OFF, WRT2 = OFF, WRT3 = OFF               // CONFIG6L
#pragma config WRTB = OFF, WRTC = OFF, WRTD = OFF                           // CONFIG6H
#pragma config EBTR0 = OFF, EBTR1 = OFF, EBTR2 = OFF, EBTR3 = OFF           // CONFIG7L
#pragma config EBTRB = OFF                                                  // CONFIG7H

/** I N C L U D E S **********************************************************/

#include "GenericTypeDefs.h"
#include "Compiler.h"
#include "usb_config.h"
#include "USB/usb_device.h"
#include "USB/usb.h"

#include "HardwareProfile.h"

/** V A R I A B L E S ********************************************************/
#if defined(__18CXX)
    #pragma udata
#endif

char USB_In_Buffer[64];
char USB_Out_Buffer[64];

BOOL stringPrinted;
volatile BOOL buttonPressed;
volatile BYTE buttonCount;

/** P R I V A T E  P R O T O T Y P E S ***************************************/
static void InitializeSystem(void);
void ProcessIO(void);
void USBDeviceTasks(void);
void YourHighPriorityISRCode();
void YourLowPriorityISRCode();
void USBCBSendResume(void);
void BlinkUSBStatus(void);
void UserInit(void);
void delay450(void);
void delay60(void);
void delay50(void);
void delay10(void);
void delay1(void);
#if defined(__18CXX)

	
	#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)
		#define REMAPPED_RESET_VECTOR_ADDRESS			0x1000
		#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x1008
		#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x1018
	#elif defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)	
		#define REMAPPED_RESET_VECTOR_ADDRESS			0x800
		#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x808
		#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x818
	#else	
		#define REMAPPED_RESET_VECTOR_ADDRESS			0x00
		#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x08
		#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x18
	#endif
	
	#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)||defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)
	extern void _startup (void);        
	#pragma code REMAPPED_RESET_VECTOR = REMAPPED_RESET_VECTOR_ADDRESS
	void _reset (void)
	{
	    _asm goto _startup _endasm
	}
	#endif
	#pragma code REMAPPED_HIGH_INTERRUPT_VECTOR = REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS
	void Remapped_High_ISR (void)
	{
	     _asm goto YourHighPriorityISRCode _endasm
	}
	#pragma code REMAPPED_LOW_INTERRUPT_VECTOR = REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS
	void Remapped_Low_ISR (void)
	{
	     _asm goto YourLowPriorityISRCode _endasm
	}
	
	#if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)||defined(PROGRAMMABLE_WITH_USB_MCHPUSB_BOOTLOADER)

	#pragma code HIGH_INTERRUPT_VECTOR = 0x08
	void High_ISR (void)
	{
	     _asm goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS _endasm
	}
	#pragma code LOW_INTERRUPT_VECTOR = 0x18
	void Low_ISR (void)
	{
	     _asm goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS _endasm
	}
	#endif	

	#pragma code
	
	
	
	#pragma interrupt YourHighPriorityISRCode
	void YourHighPriorityISRCode()
	{

        #if defined(USB_INTERRUPT)
	        USBDeviceTasks();
        #endif
	
	}	
	#pragma interruptlow YourLowPriorityISRCode
	void YourLowPriorityISRCode()
	{
		
	
	}	

#elif defined(__C30__)
    #if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)
       
    #endif
#endif





#if defined(__18CXX)
    #pragma code
#endif


#if defined(__18CXX)
void main(void)
#else
int main(void)
#endif
{   
    InitializeSystem();

    while(1)
    {
        #if defined(USB_INTERRUPT)
            if(USB_BUS_SENSE && (USBGetDeviceState() == DETACHED_STATE))
            {
                USBDeviceAttach();
            }
        #endif

        #if defined(USB_POLLING)
	
        USBDeviceTasks(); 
        #endif
    				  

		
        ProcessIO();        
    }
}



static void InitializeSystem(void)
{
    #if (defined(__18CXX) & !defined(PIC18F87J50_PIM))
        ADCON1 |= 0x0F;                
    #elif defined(__C30__)
    	#if defined(__PIC24FJ256DA210__) || defined(__PIC24FJ256GB210__)
    		ANSA = 0x0000;
    		ANSB = 0x0000;
    		ANSC = 0x0000;
    		ANSD = 0x0000;
    		ANSE = 0x0000;
    		ANSF = 0x0000;
    		ANSG = 0x0000;
        #elif defined(__dsPIC33EP512MU810__) || defined (__PIC24EP512GU810__)
        	ANSELA = 0x0000;
    		ANSELB = 0x0000;
    		ANSELC = 0x0000;
    		ANSELD = 0x0000;
    		ANSELE = 0x0000;
    		ANSELG = 0x0000;
            
           

             RPINR19 = 0;
             RPINR19 = 0x64;
             RPOR9bits.RP101R = 0x3;

        #else
            AD1PCFGL = 0xFFFF;
        #endif
    #elif defined(__C32__)
        AD1PCFG = 0xFFFF;
    #endif

    #if defined(PIC18F87J50_PIM) || defined(PIC18F46J50_PIM) || defined(PIC18F_STARTER_KIT_1) || defined(PIC18F47J53_PIM)
	
    {
        unsigned int pll_startup_counter = 600;
        OSCTUNEbits.PLLEN = 1; 
        while(pll_startup_counter--);
    }
   
    #endif
    
    #if defined(__32MX460F512L__)|| defined(__32MX795F512L__)
   
    SYSTEMConfigPerformance(60000000);
	#endif

    #if defined(__dsPIC33EP512MU810__) || defined (__PIC24EP512GU810__)

 

	PLLFBD = 38;				/* M  = 60	*/
	CLKDIVbits.PLLPOST = 0;		/* N1 = 2	*/
	CLKDIVbits.PLLPRE = 0;		/* N2 = 2	*/
	OSCTUN = 0;			

  
	
    __builtin_write_OSCCONH(0x03);		
	__builtin_write_OSCCONL(0x01);
	
	
	while (OSCCONbits.COSC != 0x3);       

  

    ACLKCON3 = 0x24C1;   
    ACLKDIV3 = 0x7;
    
    
    ACLKCON3bits.ENAPLL = 1;
    while(ACLKCON3bits.APLLCK != 1); 

    #endif

    #if defined(PIC18F87J50_PIM)
	
    WDTCONbits.ADSHR = 1;			
    ANCON0 = 0xFF;                 
    ANCON1 = 0xFF;                  
    WDTCONbits.ADSHR = 0;			
    #endif

    #if defined(PIC18F46J50_PIM) || defined(PIC18F_STARTER_KIT_1) || defined(PIC18F47J53_PIM)
	
    ANCON0 = 0xFF;                  // Default all pins to digital
    ANCON1 = 0xFF;                  // Default all pins to digital
    #endif
    
   #if defined(PIC24FJ64GB004_PIM) || defined(PIC24FJ256DA210_DEV_BOARD)
	
    {
        unsigned int pll_startup_counter = 600;
        CLKDIVbits.PLLEN = 1;
        while(pll_startup_counter--);
    }

   
    #endif



    #if defined(USE_USB_BUS_SENSE_IO)
    tris_usb_bus_sense = INPUT_PIN; 
    #endif
    

    #if defined(USE_SELF_POWER_SENSE_IO)
    tris_self_power = INPUT_PIN;	
    #endif
    
    UserInit();

    USBDeviceInit();	//usb_device.c. 




void UserInit(void)
{
    TRISB=0x10;
    PORTB=0;
    LATB=0;
    PORTA=0;
    LATA=0;
    ADCON1=0X07;
    TRISA=0;
   



}
void delay450(void)
{
   
  _asm 
         movlw 0x8
         movwf 0x40,0
 met2:   movlw 0xfa
         movwf 0x41,0
 met1:   decfsz 0x41,1,0
         goto met1
         decfsz 0x40,1,0
         goto met2
  _endasm  

}
void delay60(void)
{
   
  _asm 
         movlw 0xf0
         movwf 0x41,0
 met1:   decfsz 0x41,1,0
         goto met1
  
  _endasm     

}
void delay50(void)
{
   
  _asm 
         movlw 0xc8
         movwf 0x41,0
 met1:   decfsz 0x41,1,0
         goto met1
  
  _endasm     

}
void delay10(void)
{
   
  _asm 
         movlw 0x28
         movwf 0x41,0
 met1:   decfsz 0x41,1,0
         goto met1
  
  _endasm     

}
void delay1(void)
{
   
  _asm 
         movlw 0x8
         movwf 0x41,0
 met1:   decfsz 0x41,1,0
         goto met1
  
  _endasm     

}

void ProcessIO(void)
{   
    BYTE numBytesRead;

 
    BlinkUSBStatus();

    if((USBDeviceState < CONFIGURED_STATE)||(USBSuspendControl==1)) return;

    if(buttonPressed)
    {
        if(stringPrinted == FALSE)
        {
            if(mUSBUSARTIsTxTrfReady())
            {
                putrsUSBUSART("Button Pressed -- \r\n");
                stringPrinted = TRUE;
            }
        }
    }
    else
    {
        stringPrinted = FALSE;
    }

    if(USBUSARTIsTxTrfReady())
    {
		numBytesRead = getsUSBUSART(USB_Out_Buffer,2);
		if(numBytesRead != 0)
		{

			BYTE i;
            int a,b,c;
            
            c=0;
			for(i=0;i<numBytesRead;i++)
			{
				switch(USB_Out_Buffer[i])
				{
				   
					   
                    case '1':
                        PR2=0X20;
                        CCPR1L=1;
                        TRISCbits.TRISC2 = 0;
                        T2CON=0X0F;
                        CCP1CON=0X0F;
                        CCPR2L=23;
                        TRISCbits.TRISC1 = 0;
                        T2CON=0X0F;
                        CCP2CON=0X0F;
						break;   
                    
						    
                    case '2':
                        PR2=0X20;
                        CCPR1L=2;
                        TRISCbits.TRISC2 = 0;
                        T2CON=0X0F;
                        CCP1CON=0X0F;
                        CCPR2L=22;
                        TRISCbits.TRISC1 = 0;
                        T2CON=0X0F;
                        CCP2CON=0X0F;
						break; 
                    case '3':
                        PR2=0X20;
                        CCPR1L=3;
                        TRISCbits.TRISC2 = 0;
                        T2CON=0X0F;
                        CCP1CON=0X0F;
                        CCPR2L=21;
                        TRISCbits.TRISC1 = 0;
                        T2CON=0X0F;
                        CCP2CON=0X0F;
						break;     
                     case '4':
                        PR2=0X20;
                        CCPR1L=4;
                        TRISCbits.TRISC2 = 0;
                        T2CON=0X0F;
                        CCP1CON=0X0F;
                        CCPR2L=20;
                        TRISCbits.TRISC1 = 0;
                        T2CON=0X0F;
                        CCP2CON=0X0F;
						break;    
                      case '5':
                        PR2=0X20;
                        CCPR1L=5;
                        TRISCbits.TRISC2 = 0;
                        T2CON=0X0F;
                        CCP1CON=0X0F;
                        CCPR2L=19;
                        TRISCbits.TRISC1 = 0;
                        T2CON=0X0F;
                        CCP2CON=0X0F;
						break;    
                      case '6':
                        PR2=0X20;
                        CCPR1L=6;
                        TRISCbits.TRISC2 = 0;
                        T2CON=0X0F;
                        CCP1CON=0X0F;
                        CCPR2L=18;
                        TRISCbits.TRISC1 = 0;
                        T2CON=0X0F;
                        CCP2CON=0X0F;
						break;    
                      case '7':
                        PR2=0X20;
                        CCPR1L=7;
                        TRISCbits.TRISC2 = 0;
                        T2CON=0X0F;
                        CCP1CON=0X0F;
                        CCPR2L=17;
                        TRISCbits.TRISC1 = 0;
                        T2CON=0X0F;
                        CCP2CON=0X0F;
						break;    
                        case '8':
                        PR2=0X20;
                        CCPR1L=8;
                        TRISCbits.TRISC2 = 0;
                        T2CON=0X0F;
                        CCP1CON=0X0F;
                        CCPR2L=16;
                        TRISCbits.TRISC1 = 0;
                        T2CON=0X0F;
                        CCP2CON=0X0F;
						break;       
                    case '9':
                        PR2=0X20;
                        CCPR1L=9;
                        TRISCbits.TRISC2 = 0;
                        T2CON=0X0F;
                        CCP1CON=0X0F;
                        CCPR2L=15;
                        TRISCbits.TRISC1 = 0;
                        T2CON=0X0F;
                        CCP2CON=0X0F;
						break; 
                    case ':':
                        PR2=0X20;
                        CCPR1L=10;
                        TRISCbits.TRISC2 = 0;
                        T2CON=0X0F;
                        CCP1CON=0X0F;
                        CCPR2L=14;
                        TRISCbits.TRISC1 = 0;
                        T2CON=0X0F;
                        CCP2CON=0X0F;
						break;
                        
                        case ';':
                        PR2=0X20;
                        CCPR1L=11;
                        TRISCbits.TRISC2 = 0;
                        T2CON=0X0F;
                        CCP1CON=0X0F;
                        CCPR2L=13;
                        TRISCbits.TRISC1 = 0;
                        T2CON=0X0F;
                        CCP2CON=0X0F;
						break;   
                    
						    
                    case '<':
                        PR2=0X20;
                        CCPR1L=12;
                        TRISCbits.TRISC2 = 0;
                        T2CON=0X0F;
                        CCP1CON=0X0F;
                        CCPR2L=12;
                        TRISCbits.TRISC1 = 0;
                        T2CON=0X0F;
                        CCP2CON=0X0F;
						break; 
                    case '=':
                        PR2=0X20;
                        CCPR1L=13;
                        TRISCbits.TRISC2 = 0;
                        T2CON=0X0F;
                        CCP1CON=0X0F;
                        CCPR2L=11;
                        TRISCbits.TRISC1 = 0;
                        T2CON=0X0F;
                        CCP2CON=0X0F;
						break;     
                     case '>':
                        PR2=0X20;
                        CCPR1L=14;
                        TRISCbits.TRISC2 = 0;
                        T2CON=0X0F;
                        CCP1CON=0X0F;
                        CCPR2L=10;
                        TRISCbits.TRISC1 = 0;
                        T2CON=0X0F;
                        CCP2CON=0X0F;
						break;    
                      case '?':
                        PR2=0X20;
                        CCPR1L=15;
                        TRISCbits.TRISC2 = 0;
                        T2CON=0X0F;
                        CCP1CON=0X0F;
                        CCPR2L=9;
                        TRISCbits.TRISC1 = 0;
                        T2CON=0X0F;
                        CCP2CON=0X0F;
						break;    
                      case '@':
                        PR2=0X20;
                        CCPR1L=16;
                        TRISCbits.TRISC2 = 0;
                        T2CON=0X0F;
                        CCP1CON=0X0F;
                        CCPR2L=8;
                        TRISCbits.TRISC1 = 0;
                        T2CON=0X0F;
                        CCP2CON=0X0F;
						break;    
                      case 'A':
                        PR2=0X20;
                        CCPR1L=17;
                        TRISCbits.TRISC2 = 0;
                        T2CON=0X0F;
                        CCP1CON=0X0F;
                        CCPR2L=7;
                        TRISCbits.TRISC1 = 0;
                        T2CON=0X0F;
                        CCP2CON=0X0F;
						break;    
                        case 'B':
                        PR2=0X20;
                        CCPR1L=18;
                        TRISCbits.TRISC2 = 0;
                        T2CON=0X0F;
                        CCP1CON=0X0F;
                        CCPR2L=6;
                        TRISCbits.TRISC1 = 0;
                        T2CON=0X0F;
                        CCP2CON=0X0F;
						break;       
                    case 'C':
                        PR2=0X20;
                        CCPR1L=19;
                        TRISCbits.TRISC2 = 0;
                        T2CON=0X0F;
                        CCP1CON=0X0F;
                        CCPR2L=5;
                        TRISCbits.TRISC1 = 0;
                        T2CON=0X0F;
                        CCP2CON=0X0F;
						break; 
                     case 'D':
                        PR2=0X20;
                        CCPR1L=20;
                        TRISCbits.TRISC2 = 0;
                        T2CON=0X0F;
                        CCP1CON=0X0F;
                        CCPR2L=4;
                        TRISCbits.TRISC1 = 0;
                        T2CON=0X0F;
                        CCP2CON=0X0F;
						break; 
                       case 'E':
                        PR2=0X20;
                        CCPR1L=21;
                        TRISCbits.TRISC2 = 0;
                        T2CON=0X0F;
                        CCP1CON=0X0F;
                        CCPR2L=3;
                        TRISCbits.TRISC1 = 0;
                        T2CON=0X0F;
                        CCP2CON=0X0F;
						break;
                        case 'F':
                        PR2=0X20;
                        CCPR1L=22;
                        TRISCbits.TRISC2 = 0;
                        T2CON=0X0F;
                        CCP1CON=0X0F;
                        CCPR2L=2;
                        TRISCbits.TRISC1 = 0;
                        T2CON=0X0F;
                        CCP2CON=0X0F;
						break;
                        case 'G':
                        PR2=0X20;
                        CCPR1L=23;
                        TRISCbits.TRISC2 = 0;
                        T2CON=0X0F;
                        CCP1CON=0X0F;
                        CCPR2L=1;
                        TRISCbits.TRISC1 = 0;
                        T2CON=0X0F;
                        CCP2CON=0X0F;
						break;
                        case 'a':
                        PORTB=1;
                       
                          
                        break;
                        case 'b':
                        PORTB=2;
                        break;
                        case 'c':
                        PORTB=4;
                        break;
                        case 'd':
                        PORTB=8;
                        break;
                        case 'g':
                        PORTB=0;
                        break;
                        
                        case 'r':
                        {
                            INTCONbits.GIE_GIEH=0;
                            b=0;       
                           PORTBbits.RB5=1; //
                           delay450();      //   reset
                           PORTBbits.RB5=0; // 
                           delay60();
                           INTCONbits.GIE_GIEH=1;
                        
                             }
                             break; 
                     case 'k':
                        {
                           INTCONbits.GIE_GIEH=0; 
                           PORTBbits.RB5=1; //
                           delay60();       //
                           PORTBbits.RB5=0; //   '0'
                           delay1();        //
                           
                           PORTBbits.RB5=1; //
                           delay60();       // 
                           PORTBbits.RB5=0; //   '0'
                           delay1();        //
                           
                           PORTBbits.RB5=1; //
                           delay10();       // 
                           PORTBbits.RB5=0; //   '1'
                           delay50();       //
                           
                           PORTBbits.RB5=1; //
                           delay10();       //
                           PORTBbits.RB5=0; //   '1'
                           delay50();       //
                           
                           PORTBbits.RB5=1; //
                           delay60();       //
                           PORTBbits.RB5=0; //   '0'
                           delay1();        //
                           
                           PORTBbits.RB5=1; //
                           delay60();       // 
                           PORTBbits.RB5=0; //   '0'
                           delay1();        //
                           
                           PORTBbits.RB5=1; //
                           delay10();       // 
                           PORTBbits.RB5=0; //   '1'
                           delay50();       //
                           
                           PORTBbits.RB5=1; //
                           delay10();       //
                           PORTBbits.RB5=0; //   '1'
                           delay50();       //
                           
                           
                           PORTBbits.RB5=1; //
                           delay60();       //
                           PORTBbits.RB5=0; //   '0'
                           delay1();        //
                          
                           PORTBbits.RB5=1; //
                           delay60();       //
                           PORTBbits.RB5=0; //   '0'
                           delay1();        //
                           
                           PORTBbits.RB5=1; //
                           delay10();       //
                           PORTBbits.RB5=0; //   '1'
                           delay50();       //
                           
                          
                           PORTBbits.RB5=1; //
                           delay60();       //
                           PORTBbits.RB5=0; //   '0'
                           delay1();        //
                           
                           PORTBbits.RB5=1; //
                           delay60();       //
                           PORTBbits.RB5=0; //   '0'
                           delay1();        //
                           
                           PORTBbits.RB5=1; //
                           delay60();       //
                           PORTBbits.RB5=0; //   '0'
                           delay1();        //
                           
                           PORTBbits.RB5=1; //
                           delay10();       //
                           PORTBbits.RB5=0; //   '1'
                           delay50();       //
                           
                           PORTBbits.RB5=1; //
                           delay60();       //
                           PORTBbits.RB5=0; //   '0'
                           delay1();        //
                           INTCONbits.GIE_GIEH=1;
                           }
                             break; 
                    
                     case 'e':
                        {  
                           INTCONbits.GIE_GIEH=0;
                           PORTBbits.RB5=1; //
                           delay60();       //
                           PORTBbits.RB5=0; //   '0'
                           delay1();        //
                           
                           PORTBbits.RB5=1; //
                           delay60();       // 
                           PORTBbits.RB5=0; //   '0'
                           delay1();        //
                           
                           PORTBbits.RB5=1; //
                           delay10();       // 
                           PORTBbits.RB5=0; //   '1'
                           delay50();       //
                           
                           PORTBbits.RB5=1; //
                           delay10();       //
                           PORTBbits.RB5=0; //   '1'
                           delay50();       //
                           
                           PORTBbits.RB5=1; //
                           delay60();       //
                           PORTBbits.RB5=0; //   '0'
                           delay1();        //
                           
                           PORTBbits.RB5=1; //
                           delay60();       // 
                           PORTBbits.RB5=0; //   '0'
                           delay1();        //
                           
                           PORTBbits.RB5=1; //
                           delay10();       // 
                           PORTBbits.RB5=0; //   '1'
                           delay50();       //
                           
                           PORTBbits.RB5=1; //
                           delay10();       //
                           PORTBbits.RB5=0; //   '1'
                           delay50();       //
                           delay60();  
                           
                            
                           PORTBbits.RB5=1; //
                           delay60();       // 
                           PORTBbits.RB5=0; //   '0'
                           delay1();        //
                            
                           PORTBbits.RB5=1; //
                           delay10();       // 
                           PORTBbits.RB5=0; //   '1'
                           delay50();       //
                           
                           PORTBbits.RB5=1; //
                           delay10();       //
                           PORTBbits.RB5=0; //   '1'
                           delay50();       //
                            
                           PORTBbits.RB5=1; //
                           delay10();       // 
                           PORTBbits.RB5=0; //   '1'
                           delay50();       //
                           
                           PORTBbits.RB5=1; //
                           delay10();       //
                           PORTBbits.RB5=0; //   '1'
                           delay50();       //
                           
                           PORTBbits.RB5=1; //
                           delay10();       //
                           PORTBbits.RB5=0; //   '1'
                           delay50();       //
                           
                           PORTBbits.RB5=1; //
                           delay60();       // 
                           PORTBbits.RB5=0; //   '0'
                           delay1();        //
                           
                           PORTBbits.RB5=1; //
                           delay10();       //
                           PORTBbits.RB5=0; //   '1'
                           delay50();       //
                           INTCONbits.GIE_GIEH=1;
                             }
                             break; 
                     case 'j':
                        {
                            INTCONbits.GIE_GIEH=0;
                                
                                  PORTBbits.RB5=1; //
                                  delay1();       //
                                  PORTBbits.RB5=0; //   '1'
                                  delay60();       //
                                  PORTBbits.RB5=1; //
                                  delay1();       //
                                  PORTBbits.RB5=0; //   '1'
                                  delay60();       //
                                  PORTBbits.RB5=1; //
                                  delay1();       //
                                  PORTBbits.RB5=0; //   '1'
                                  delay60();       //
                                  PORTBbits.RB5=1; //
                                  delay1();       //
                                  PORTBbits.RB5=0; //   '1'
                                  delay60();       //
                           c=0;
                           
                            PORTBbits.RB5=1; //    
                           delay1();       //
                           PORTBbits.RB5=0; //   '1'
                           delay10();       //
                           if(PORTBbits.RB4==1) c=c+1;
                           delay50();          
                            
                            PORTBbits.RB5=1; //
                           delay1();       //
                           PORTBbits.RB5=0; //   '1'
                           delay10();       //
                           if(PORTBbits.RB4==1) c=c+2;
                           delay50();          
                               
                            PORTBbits.RB5=1; //
                           delay1();       //
                           PORTBbits.RB5=0; //   '1'
                           delay10();       //
                           if(PORTBbits.RB4==1) c=c+4;
                           delay50();          
                                    
                            PORTBbits.RB5=1; //
                           delay1();       //
                           PORTBbits.RB5=0; //   '1'
                           delay10();       //
                           if(PORTBbits.RB4==1) c=c+8;
                           delay450();  
                           
                             PORTBbits.RB5=1; //
                           delay1();       //
                           PORTBbits.RB5=0; //   '1'
                           delay10();       //
                           if(PORTBbits.RB4==1) c=c+16;
                           delay50();          
                            
                      PORTBbits.RB5=1; //
                           delay1();       //
                           PORTBbits.RB5=0; //   '1'
                           delay10();       //
                           if(PORTBbits.RB4==1) c=c+32;
                           delay50(); 
                           
                         PORTBbits.RB5=1; //
                           delay1();       //
                           PORTBbits.RB5=0; //   '1'
                           delay10();       //
                           if(PORTBbits.RB4==1) c=c+64;
                           delay50();          
                               
                         PORTBbits.RB5=1; //
                           delay1();       //
                           PORTBbits.RB5=0; //   '1'
                           delay10();       //
                           if(PORTBbits.RB4==1) c=c+128;
                           delay50();
                           
                        PORTA=c;
                      
                         
                        USB_In_Buffer[i] = c; 
                           
                          
                        putUSBUSART(USB_In_Buffer,numBytesRead);
                      
                        
                       INTCONbits.GIE_GIEH=1;
                        }
                        break;
					default:
						
						break;
				}

			}
            

			
		}
           
	}

    CDCTxService();
}		

void USBCBInitEP(void)
{
    CDCInitEP();
}


void USBCBSendResume(void)
{
    static WORD delay_count;
    
   
    if(USBGetRemoteWakeupStatus() == TRUE) 
    {
        
        if(USBIsBusSuspended() == TRUE)
        {
            USBMaskInterrupts();
            
           
            USBCBWakeFromSuspend();
            USBSuspendControl = 0; 
            USBBusIsSuspended = FALSE;  
            delay_count = 3600U;        
            do
            {
                delay_count--;
            }while(delay_count);
            
           
            USBResumeControl = 1;      
            delay_count = 1800U;       
            do
            {
                delay_count--;
            }while(delay_count);
            USBResumeControl = 0;       

            USBUnmaskInterrupts();
        }
    }
}



#if defined(ENABLE_EP0_DATA_RECEIVED_CALLBACK)
void USBCBEP0DataReceived(void)
{
}
#endif


BOOL USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, WORD size)
{
    switch( (INT)event )
    {
        case EVENT_TRANSFER:
           
            break;
        case EVENT_SOF:
            USBCB_SOF_Handler();
            break;
        case EVENT_SUSPEND:
            USBCBSuspend();
            break;
        case EVENT_RESUME:
            USBCBWakeFromSuspend();
            break;
        case EVENT_CONFIGURED: 
            USBCBInitEP();
            break;
        case EVENT_SET_DESCRIPTOR:
            USBCBStdSetDscHandler();
            break;
        case EVENT_EP0_REQUEST:
            USBCBCheckOtherReq();
            break;
        case EVENT_BUS_ERROR:
            USBCBErrorHandler();
            break;
        case EVENT_TRANSFER_TERMINATED:
           
            break;
        default:
            break;
    }      
    return TRUE; 
}
