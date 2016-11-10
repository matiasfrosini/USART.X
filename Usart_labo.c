/* 
Plantilla inicial para
Código C 
*/

// Includes
#include <p24fj128ga010.h>
#define FCY 16000000
#include <libpic30.h>

#define CTS   _RF12
#define RTS   _RF13

#define RS PORTBbits.RB15
#define RW PORTDbits.RD5
#define E PORTDbits.RD4


#define TRTS TRISFbits.TRISF13



//Configuración de bits para Explorer-16
_CONFIG1( JTAGEN_OFF        // deshabilito interface JTAG
        & GCP_OFF           // deshabilito proteción de código general
        & GWRP_OFF          // deshabilito protección escritura flash
        & ICS_PGx2         // ICSP interface (2=default)
        & FWDTEN_OFF)       // deshabilito watchdog timer
_CONFIG2( IESO_OFF          // deshabilito 2 velocidades de start up
        & FCKSM_CSDCMD      // deshabilito clock-swithcing/monitor
        & FNOSC_PRIPLL      // primary oscillator: habilito PLL
        & POSCMOD_XT)       // primary oscillator: modo XT



//Definiciones

//Declaracion de variables
        int valor;
        char rxReg;
        int ADCValue;
        float Temp;
        ACDValue = 0;
        int *p_ADCValue;
        float valoradc;
        int UniALTA;
        int DecALTA;
        int DecBAJA;
        
        //Todo lo correspondiente a RTCC
        unsigned int q,w,r,t;
        unsigned int HORA[7]={0};       /* este vector retendra los valores de la 
                                 * configuracion de la hora*/
        unsigned int FECHA[7]={0};      // se almacenara la fecha y el 

//Prototipos de funciones

        void INITUsart(void);
        void txStart(int valor);
        void INITPort(void);
        char rxStart(void);
        int rxAdc(void);
        void INITAdc(void);
        void INITLcd(void);
        int Dato(char valor);
        void Comando (char valor);
        float Conversion(float valoradc);
        void INITRtcc(void);


int main(void)
{
    p_ADCValue = &ADCValue;
    
    // Inicio todos los modulos 
    
    INITAdc();
    INITPort();
    INITLcd();
    INITUsart();
    //INITRtcc();
    
    // Inicializo LCD
    
    Comando(0x38);
    Comando(0x0c);
    Comando(0x06);
    Comando(0x00);
    
    __delay_ms(10);
    

    while(1)
    {
        
        rxAdc();                    // Adquiero valor de la temperatura
        valoradc = *p_ADCValue;     
        Conversion(valoradc);       // Convierto valor de tension en temp
        
        Comando(0xC0);
        DecALTA=(int)(Temp/10);
        UniALTA=(int)(Temp-(DecALTA*10));
        DecBAJA=(int)((Temp-(DecALTA*10)-UniALTA)*10);
        
        Dato((DecALTA)+0x30);
        Dato((UniALTA)+0x30);
        Dato('.');
        Dato((DecBAJA)+0x30);
        txStart((DecALTA)+0x30);
        txStart((UniALTA)+0x30);
        txStart('.');
        txStart((DecBAJA)+0x30);
        txStart('       ');
        __delay_ms(100);            
        
        
    }
    

}



/*********************************************************
 *
 * Description: Config inicial modulo USART
 * 
 * Notes: 
 * 
 * Returns: Void.
 * 
 **********************************************************/
void INITUsart(void)
{
    
    U2MODE  = 0x0000;
    U2BRG   = 103; // SINO PROBAR CON 0XCF o 0X1A0
    U2STA   = 0;
    IEC1bits.U2RXIE=1;      // Habilito la interrupcion 
    U2MODEbits.UARTEN = 1;
    U2STA   = 0x400;        //Habilito transmision
    
}

/*********************************************************
 *
 * Description: Transmisión modulo USART
 * 
 * Notes:
 * 
 * Return: Void.
 * 
 **********************************************************/

void txStart(int valor)
{
    while(U2STAbits.UTXBF);
    U2TXREG     = valor;
    
    while (U2STAbits.TRMT==0);
    
}

/*********************************************************
 *
 * Description: Recepción modulo USART por Interrupcion
 * 
 * Notes:
 * 
 * Return: Char rxReg.
 * 
 **********************************************************/

void __attribute__ ((interrupt,no_auto_psv)) _U2RXInterrupt(void)
{
    IFS1bits.U2RXIF=0;
    PORTAbits.RA0=0;
    rxReg=U2RXREG;
}



void INITPort(void)
{
        TRISA= 0x00;
        PORTA=0x00;
        PORTAbits.RA0 = 1;
}


/*********************************************************
 *
 * Description: Configuración ADC
 * 
 * Notes:
 * 
 * Return: Void.
 * 
 **********************************************************/

void INITAdc(void)
{
    AD1PCFG = 0;
    AD1CON1 = 0;
    AD1CHS  = 0x04; // AN5 entrada analogica
    AD1CSSL = 0; 
    AD1CON3 = 0x02;
    AD1CON2 = 0;
    
    AD1CON1bits.ADON = 1;       // Enciendo ADC
    
    return;        
}

/*********************************************************
 *
 * Description: Lectura ADC Termometro
 * 
 * Notes:
 * 
 * Return:  Int ADCValue.
 * 
 **********************************************************/

int rxAdc(void)
{
    AD1CON1bits.SAMP =1;
    __delay_us(0.1);
    
    AD1CON1bits.SAMP    =   0;
    while   (!AD1CON1bits.DONE);
    ADCValue = ADC1BUF0;
    
    return ADCValue ;
} 


void Comando (char valor)
{
    PORTE   = valor;
    RS      = 0;
    RW      = 0 ;
    E       = 1;
    Nop();
    E       = 0;
    __delay_ms(100);

}

int Dato(char valor)
{
    PORTE   = valor;
    RS      =1;
    RW      =0;
    E       =1;
    __delay_us(1);
    E       =0;
    __delay_ms(1);
}

void INITLcd(void)
{
    unsigned char i;
    TRISBbits.TRISB15=0;
    TRISDbits.TRISD5=0;
    TRISDbits.TRISD4=0;
    TRISE=0;
    for (i=0; i<3 ; i++)
    {
        PORTE   =0x30;
        RS      =0;
        RW      =0;
        E       =1;
        __delay_us(1);
        E       =0;
        __delay_ms(100);
        
    }
}

/*********************************************************
 *
 * Description: Obtener el valor de temperatura
 * 
 * Notes:
 * 
 * Return:  Float Temp.
 * 
 **********************************************************/

 float Conversion( float valoradc)
{
    Temp = valoradc;
    Temp = (((((Temp)/1024)*3.3)-0.5)/0.01);        //Conversion de tension a temperatura.
   
    return Temp;
}
 
 /*********************************************************
 *
 * Description: Inicializacion del RTCC
 * 
 * Notes:
 * 
 * Return:  Float Temp.
 * 
 **********************************************************/
 
 void INITRtcc( void){    // inicializacion del RTCC
    // Habilito el  OSCON escritura y seteo
    //_SOSCEN =1;
 	asm volatile ("mov #OSCCON,W1");
	asm volatile ("mov.b	#0x46, W2");    //secuencia para desbloquear
	asm volatile ("mov.b	#0x57, W3");
	asm volatile ("mov.b	#0x02, W0");    // SOSCEN =1
	asm volatile ("mov.b	W2, [W1]");
	asm volatile ("mov.b	W3, [W1]");
	asm volatile ("mov.b	W0, [W1]");


    //_RTCWREN = 1;     // unlock setting
	asm volatile("disi	#5");
	asm volatile("mov	#0x55, w7");
	asm volatile("mov	w7, _NVMKEY");
	asm volatile("mov	#0xAA, w8");
	asm volatile("mov	w8, _NVMKEY");
    asm volatile("bset	_RCFGCAL, #13");    // RTCWREN =1;
	asm volatile("nop");
	asm volatile("nop");
    /* Con estas transformaciones cargo el reloj */    
    q = ((FECHA[4]<<4))+ (FECHA[5]);
    w = ((FECHA[2]<<12)+ (FECHA[3]<<8) + (FECHA[0]<<4) + (FECHA[1]));
    r = ((FECHA[6]<<8) + (HORA[0]<<4)  + (HORA[1]));
    t = ((HORA[2]<<12) + (HORA[3]<<8)  + (HORA[4]<<4)  + (HORA[5]));
    
    _RTCEN = 0;         // deshabilito el clock
    
    // seteo al cargar las variables q, w, r y t// formato de 24 horas
    _RTCPTR = 3;        // comienza la secuencia
    RTCVAL = q;         // año
    RTCVAL = w;         // mes-1/dia-1
    RTCVAL = r;         // dia de semana/hora
    RTCVAL = t;         // minutos/segundos

    // En el caso de querer calibrar el RTCC
    //_CAL = 0x00;     // comando para calibrar por desajustes con el cristal
    // Quiero habilitar la alarma
    _AMASK = HORA[7];        /* habilito para cada minuto // acá determino el tiempo
                        *  para que se active la alarma */
    _ARPT   = 0xFF;    // la alarma se repetira unas 255 veces más
    _CHIME  = 1;       // bit para que la alarma repique, no se a que se refiere
    _ALRMEN = 1;       // habilito alarma
    // lock y habilito
    _RTCEN  = 1;       // habilito el clock
    _RTCWREN= 0;       // lock el seteo
    _RTCIE  = 1;       // habilito interrupcion
} 
 
 