/*********************************************************************
This is an example for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

This example is for a 128x32|64 size display using SPI or I2C to communicate
4 or 5 pins are required to interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution

02/18/2013 	Charles-Henri Hallard (http://hallard.me)
						Modified for compiling and use on Raspberry ArduiPi Board
						LCD size and connection are now passed as arguments on 
						the command line (no more #define on compilation needed)
						ArduiPi project documentation http://hallard.me/arduipi

15/June/2016 johnty wang (johnty.ca)
                  Modified example to show simple gauge
                  from 1st ADC channel of a mcp3008 chip on SPI ch 0
                  built against wiringPi 2.32
						
*********************************************************************/
//note: include wiringPi stuff first, otherwise 
// get compile error due to conflicting delay names
#include <wiringPi.h>
#include <mcp3004.h>
#include "ArduiPi_OLED_lib.h"
#include "Adafruit_GFX.h"
#include "ArduiPi_OLED.h"

#include <stdio.h>
#include <signal.h>


#define BASE 100 //mcp3004/8
#define SPI_CH 0
#include <getopt.h>

#define PRG_NAME        "oled_demo"
#define PRG_VERSION     "1.1"
// Instantiate the display
ArduiPi_OLED display;

static volatile int keepRunning = 1; //for ctrl+c interrupt

void intHandler(int sig)
{
   keepRunning = 0;
   printf("\nctrl+c detected! exiting...\n");
}
// Config Option
struct s_opts
{
	int oled;
	int verbose;
} ;

int sleep_divisor = 1 ;
	
// default options values
s_opts opts = {
	OLED_ADAFRUIT_SPI_128x32,	// Default oled
  false										// Not verbose
};

void testscrolltext(void) {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10,0);
  display.clearDisplay();
  
  display.print("Analog\n Gauge \n Example!!");
  if (opts.oled != OLED_SH1106_I2C_128x64)
    display.startscrolldiagright(0x00, 0x0F);
  display.display();
}

/* ======================================================================
Function: usage
Purpose : display usage
Input 	: program name
Output	: -
Comments: 
====================================================================== */
void usage( char * name)
{
	printf("%s\n", name );
	printf("Usage is: %s --oled type [options]\n", name);
	printf("  --<o>led type\nOLED type are:\n");
	for (int i=0; i<OLED_LAST_OLED;i++)
		printf("  %1d %s\n", i, oled_type_str[i]);
	
	printf("Options are:\n");
	printf("  --<v>erbose  : speak more to user\n");
	printf("  --<h>elp\n");
	printf("<?> indicates the equivalent short option.\n");
	printf("Short options are prefixed by \"-\" instead of by \"--\".\n");
	printf("Example :\n");
	printf( "%s -o 1 use a %s OLED\n\n", name, oled_type_str[1]);
	printf( "%s -o 4 -v use a %s OLED being verbose\n", name, oled_type_str[4]);
}


/* ======================================================================
Function: parse_args
Purpose : parse argument passed to the program
Input 	: -
Output	: -
Comments: 
====================================================================== */
void parse_args(int argc, char *argv[])
{
	static struct option longOptions[] =
	{
		{"oled"	  , required_argument,0, 'o'},
		{"verbose", no_argument,	  	0, 'v'},
		{"help"		, no_argument, 			0, 'h'},
		{0, 0, 0, 0}
	};

	int optionIndex = 0;
	int c;

	while (1) 
	{
		/* no default error messages printed. */
		opterr = 0;

    c = getopt_long(argc, argv, "vho:", longOptions, &optionIndex);

		if (c < 0)
			break;

		switch (c) 
		{
			case 'v': opts.verbose = true	;	break;

			case 'o':
				opts.oled = (int) atoi(optarg);
				
				if (opts.oled < 0 || opts.oled >= OLED_LAST_OLED )
				{
						fprintf(stderr, "--oled %d ignored must be 0 to %d.\n", opts.oled, OLED_LAST_OLED-1);
						fprintf(stderr, "--oled set to 0 now\n");
						opts.oled = 0;
				}
			break;

			case 'h':
				usage(argv[0]);
				exit(EXIT_SUCCESS);
			break;
			
			case '?':
			default:
				fprintf(stderr, "Unrecognized option.\n");
				fprintf(stderr, "Run with '--help'.\n");
				exit(EXIT_FAILURE);
		}
	} /* while */

	if (opts.verbose)
	{
		printf("%s v%s\n", PRG_NAME, PRG_VERSION);
		printf("-- OLED params -- \n");
		printf("Oled is    : %s\n", oled_type_str[opts.oled]);
		printf("-- Other Stuff -- \n");
		printf("verbose is : %s\n", opts.verbose? "yes" : "no");
		printf("\n");
	}	
}

int drawGauge(int i) {
      display.clearDisplay();
      display.setCursor(0,0);
      display.print("Gauge \n\nGraph!\n");
      display.printf("  %03d %%", i);
      display.drawHorizontalBargraph(0,16, (int16_t) display.width(),16,1, i);
      display.display();
      return 0;
}

/* ======================================================================
Function: main
Purpose : Main entry Point
Input 	: -
Output	: -
Comments: 
====================================================================== */
int main(int argc, char **argv)
{
   //install sig handler
   signal(SIGINT, intHandler);
   if (wiringPiSetup() == -1) {
      printf("wiringPi init failed\n");
      return -1;
   }

   mcp3004Setup(BASE,SPI_CH);
	
	// Oled supported display in ArduiPi_SSD1306.h
	// Get OLED type
	parse_args(argc, argv);

	// SPI
	if (display.oled_is_spi_proto(opts.oled))
	{
		// SPI change parameters to fit to your LCD
		if ( !display.init(OLED_SPI_DC,OLED_SPI_RESET,OLED_SPI_CS, opts.oled) )
			exit(EXIT_FAILURE);
	}
	else
	{
		// I2C change parameters to fit to your LCD
		if ( !display.init(OLED_I2C_RESET,opts.oled) )
			exit(EXIT_FAILURE);
	}

	display.begin();
   display.stopscroll();
	
  // init done
  display.clearDisplay();   // clears the screen  buffer
  display.display();   		// display it (clear display)

  testscrolltext();
  sleep(5);
  
  display.stopscroll();

  display.setTextSize(2);
  while (keepRunning == 1)
  {
    int val = analogRead(BASE+0);
    //printf("read %i\n", val);
    drawGauge(val*100/1023);
  }
	
	// Free PI GPIO ports
	display.close();

}


