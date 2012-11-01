#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include "serial.h"

#include "ftdiserial.h"

#include "aducloader.h"
#include "hex.h"

/* Used by main to communicate with parse_opt. */
struct arguments {
    int ftdilist;
    char * ftdicfgname;
    int bbreset;
    int bbisp;
    char * firmwarename;
};

/*
#include <argp.h>

const char *argp_program_version =
       "aducftdi 0.0";
const char *argp_program_bug_address =
       "<eugene.stahov@gmail.com>";

// Program documentation. 
static char doc[] = 
     "aducftdi -- ADUC70xx FTDI232R flasher";
     
// A description of the arguments we accept.
//static char args_doc[]="-l ";
       
// The options we understand.
static struct argp_option options[] = {
    {"list",    'l', 0,      0,  "List all connected FT232 devices" },
    {"ftdicfg",   'c', "ftdi.hex", 0, "Write FTDI config eeprom"},
    {"reset",  'r', 0,      0,  "Reset target (FT232R bitbang B2; see manual)" },
    {"isp",  'i', 0,      0,  "Enter ISP mode (FT232R bitbang B1, B2; see manual)" },
    {"flash",   'f', "firmware.hex", 0, "Erase flash memory and write it with firmware.hex file" },
    { 0 }
};

// Parse a single option. 
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  // Get the input argument from argp_parse, which we
  //  know is a pointer to our arguments structure.
  struct arguments *arguments = state->input;

  switch (key)
    {
    case 'l':       arguments->ftdilist = 1; break;
    case 'c':       arguments->ftdicfgname = arg; break;
    case 'r':       arguments->bbreset = 1;
    case 'i':       arguments->bbisp = 1; break;
    case 'f':       arguments->firmwarename = arg; break;
      break;

    case ARGP_KEY_ARG:
      //if (state->arg_num >= 2)
	// Too many arguments.
	//argp_usage (state);

      break;

    case ARGP_KEY_END:
      //if (state->arg_num < 2)
	// Not enough arguments.
	//argp_usage (state);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

// Our argp parser.
//static struct argp argp = { options, parse_opt, args_doc, doc };
static struct argp argp = { options, parse_opt, NULL, doc };
*/

void advFlash() {
    char tmp[40];
    
    memset(tmp, ' ', 32);
    getFtdiReadWrite()->writeBlock(tmp, 32, 1000);
    getFtdiReadWrite()->readBlock(tmp, 40, 200);

    flushFtdi();
  
}

int main(int argc, char **argv) {
    struct arguments arguments;
     
    /* Default values. */
    arguments.ftdilist = 0;
    arguments.bbreset = 0;
    arguments.bbisp = 0;
    arguments.firmwarename = "inp9.hex";
    arguments.ftdicfgname = NULL;
     
  /* Parse our arguments; every option seen by parse_opt will
    be reflected in arguments. */
  //argp_parse (&argp, argc, argv, 0, 0, &arguments);

  //return (ftdiListAll()>=0) ? 0:-1;
  if (argc==2) arguments.firmwarename = argv[1];
  
  struct HexRecord * root;
  
  if (process_ihexfile(arguments.firmwarename, &root)<0)  return -1;
  
  if (bindFtdi("i:0x0403:0x6001") < 0 ) return -1;
  
  printFtdiInfo();
  
  setFtdiBaudRate(115200);
  
  //flushFtdi();
  advFlash();

  //enter programming mode
  aducFtdiReset(1);
  
  flushFtdi();
  
  int flashresult = writeAducFlash(getFtdiReadWrite(), root);
  //leave programming mode
  aducFtdiReset(0);
  
  closeFtdi();
  
  //resetFtdi();
  
  return flashresult;
}
