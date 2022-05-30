/*-------------------------------------------------------------------------------
 * dsd_ncurses.c
 * A dsd ncurses terminal printer
 *
 * ASCII art generated by:
 * https://fsymbols.com/generators/carty/
 *
 * LWVMOBILE
 * 2022-02 DSD-FME Florida Man Edition
 *-----------------------------------------------------------------------------*/
 /*
  * Copyright (C) 2010 DSD Author
  * GPG Key ID: 0x3F1D7FD0 (74EF 430D F7F2 0A48 FCE6  F630 FAA2 635D 3F1D 7FD0)
  *
  * Permission to use, copy, modify, and/or distribute this software for any
  * purpose with or without fee is hereby granted, provided that the above
  * copyright notice and this permission notice appear in all copies.
  *
  * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
  * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
  * AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
  * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
  * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
  * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
  * PERFORMANCE OF THIS SOFTWARE.
  */

#include "dsd.h"
#include "git_ver.h"

int reset = 0;
char c; //getch key
int tg;
int tgR;
int tgn;
int rd;
int rdR;
int rn;
int nc;
int src;
int lls = -1;
int dcc = -1;
int i = 0;
char versionstr[25];
unsigned long long int call_matrix[33][6];

char * FM_bannerN[9] = {
  " 2022 Dove Award Winner           CTRL + C twice to exit",
  " ██████╗  ██████╗██████╗     ███████╗███╗   ███╗███████╗",
  " ██╔══██╗██╔════╝██╔══██╗    ██╔════╝████╗ ████║██╔════╝",
  " ██║  ██║╚█████╗ ██║  ██║    █████╗  ██╔████╔██║█████╗  ",
  " ██║  ██║ ╚═══██╗██║  ██║    ██╔══╝  ██║╚██╔╝██║██╔══╝  ",
  " ██████╔╝██████╔╝██████╔╝    ██║     ██║ ╚═╝ ██║███████╗",
  " ╚═════╝ ╚═════╝ ╚═════╝     ╚═╝     ╚═╝     ╚═╝╚══════╝",
  "https://github.com/lwvmobile/dsd-fme/tree/pulseaudio    "
};

char * SyncTypes[36] = {
  "+P25P1",
  "-P25P1",
  "+X2TDMA DATA",
  "-X2TDMA DATA",
  "+X2TDMA VOICE",
  "-X2TDMA VOICE",
  "+DSTAR",
  "-DSTAR",
  "+NXDN VOICE",
  "-NXDN VOICE",
  "+DMR DATA",
  "-DMR DATA",
  "+DMR VOICE",
  "-DMR VOICE",
  "+PROVOICE",
  "-PROVOICE",
  "+NXDN DATA",
  "-NXDN DATA",
  "+DSTAR HD",
  "-DSTAR HD",
  "+dPMR",
  "+dPMR",
  "+dPMR",
  "+dPMR",
  "-dPMR",
  "-dPMR",
  "-dPMR",
  "-dPMR",
  "+NXDN (sync only)",
  "-NXDN (sync only)",
  "+YSF",
  "-YSF",
  "DMR MS VOICE",
  "DMR MS DATA",
  "DMR RC DATA"
};

char * DMRBusrtTypes[18] = {
  "PI Header     ",
  "VOICE LC HDR  ",
  "TLC           ",
  "CSBK          ",
  "MBC Header    ",
  "MBC Cont      ",
  "DATA Header   ",
  "RATE 1/2 DATA ",
  "RATE 3/4 DATA ",
  "Slot Idle     ",
  "Rate 1 DATA   ",
  "ERR           ", //These values for ERR may be Reserved for use in future?
  "ERR           ",
  "ERR           ",
  "ERR           ",
  "ERR           ",
  "Voice         ", //Using 16 for Voice since its higher than possible value?
  "INITIAL       "  //17 is assigned on start up

};

time_t nowN;
char * getTimeN(void) //get pretty hh:mm:ss timestamp
{
  time_t t = time(NULL);

  char * curr;
  char * stamp = asctime(localtime( & t));

  curr = strtok(stamp, " ");
  curr = strtok(NULL, " ");
  curr = strtok(NULL, " ");
  curr = strtok(NULL, " ");

  return curr;
}

void ncursesOpen ()
{
  mbe_printVersion (versionstr);
  setlocale(LC_ALL, "");
  initscr(); //Initialize NCURSES screen window
  start_color();
  init_pair(1, COLOR_YELLOW, COLOR_BLACK);      //Yellow/Amber for frame sync/control channel, NV style
  init_pair(2, COLOR_RED, COLOR_BLACK);        //Red for Terminated Calls
  init_pair(3, COLOR_GREEN, COLOR_BLACK);     //Green for Active Calls
  init_pair(4, COLOR_CYAN, COLOR_BLACK);     //Cyan for Site Extra and Patches
  init_pair(5, COLOR_MAGENTA, COLOR_BLACK); //Magenta for no frame sync/signal
  noecho();
  cbreak();

}

void
ncursesPrinter (dsd_opts * opts, dsd_state * state)
{
  int level;
  level = 0; //start each cycle with 0

  //Variable reset/set section

  //carrier reset
  if (state->carrier == 0) //reset these to 0 when no carrier
  {
    state->payload_algid    = 0;
    state->payload_keyid    = 0;
    state->payload_keyidR   = 0;
    state->payload_algidR   = 0;
    //state->payload_mfid  = 0;
    //state->payload_mi       = 0;
    //state->payload_miR      = 0;

    state->nxdn_key         = 0;
    state->nxdn_cipher_type = 0;
    //memset(state->dstarradioheader, 0, 41);
    sprintf(state->dmr_branding, " ");
  }

  //set lls sync types
  if (state->synctype >= 0 && state->synctype < 35) //not sure if this will be okay
  {
    lls = state->synctype;
  }

  //reset DMR alias block and embedded gps if burst type is not 16 or carrier drop
  if (state->dmrburstL != 16 || state->carrier == 0)
  {
    for (short i = 0; i < 6; i++)
    {
      sprintf (state->dmr_callsign[0][i], "");
    }
  }
  if (state->dmrburstR != 16 || state->carrier == 0)
  {
    for (short i = 0; i < 6; i++)
    {
      sprintf (state->dmr_callsign[1][i], "");
    }
  }

  //reset DMR LRRP when call is active on current slot if burst type is not data or carrier drop
  if (state->dmrburstL == 16 || state->carrier == 0)
  {
    for (short i = 0; i < 6; i++)
    {
      sprintf (state->dmr_lrrp[0][i], "");
    }
  }
  if (state->dmrburstR == 16 || state->carrier == 0)
  {
    for (short i = 0; i < 6; i++)
    {
      sprintf (state->dmr_lrrp[1][i], "");
    }
  }

  //NXDN
  if (state->nxdn_last_rid > 0 && state->nxdn_last_rid != src);
  {
    src = state->nxdn_last_rid;
  }
  if (state->nxdn_last_ran > -1 && state->nxdn_last_ran != rn);
  {
    rn = state->nxdn_last_ran;
  }
  if (state->nxdn_last_tg > 0 && state->nxdn_last_tg != tgn);
  {
    tgn = state->nxdn_last_tg;
  }

  //DMR CC
  if (state->color_code_ok && state->dmr_color_code != -1 && (lls == 12 || lls == 13 || lls == 10 || lls == 11 || lls == 32 || lls == 33) )
  {
    dcc = state->dmr_color_code;
  }

  //DMR SRC
  if ( (lls == 12 || lls == 13 || lls == 10 || lls == 11 || lls == 32) )
  {
    if (state->dmrburstL == 16 && state->lastsrc > 0) //state->currentslot == 0 &&
    {
      rd = state->lastsrc;
    }

    if (state->dmrburstR == 16 && state->lastsrcR > 0) //state->currentslot == 1 &&
    {
      rdR = state->lastsrcR;
    }
    //move to seperate P25 version plz
    opts->p25enc = 0;
  }

  //DMR TG
  if ( (lls == 12 || lls == 13 || lls == 10 || lls == 11 || lls == 32) )
  {
    if (state->dmrburstL == 16 && state->lasttg > 0) //state->currentslot == 0 &&
    {
      tg = state->lasttg;
    }

    if (state->dmrburstR == 16 && state->lastsrcR > 0) //state->currentslot == 1 &&
    {
      tgR = state->lasttgR;
    }
    opts->p25enc = 0;
  }

  //P25
  if (state->nac > 0)
  {
    nc = state->nac;
  }
  if ( state->lasttg > 0 && (lls == 0 || lls == 1) )
  {
    tg = state->lasttg;
  }
  if ( state->lastsrc > 0 && (lls == 0 || lls == 1) )
  {
    rd = state->lastsrc;
  }

  //Call History Matrix Shuffling
  //ProVoice
  if ( (lls == 14 || lls == 15) && (time(NULL) - call_matrix[9][5] > 5) && state->carrier == 1)
  {
    for (short int k = 0; k < 9; k++)
    {
      call_matrix[k][0] = call_matrix[k+1][0];
      call_matrix[k][1] = call_matrix[k+1][1];
      call_matrix[k][2] = call_matrix[k+1][2];
      call_matrix[k][3] = call_matrix[k+1][3];
      call_matrix[k][4] = call_matrix[k+1][4];
      call_matrix[k][5] = call_matrix[k+1][5];
    }

    call_matrix[9][0] = lls;
    call_matrix[9][1] = 1;
    call_matrix[9][2] = 1;
    call_matrix[9][3] = 1;
    call_matrix[9][4] = 1;
    call_matrix[9][5] = time(NULL);

  }

  //D-STAR, work on adding the headers later
  if ( (lls == 6 || lls == 7 || lls == 18 || lls == 19) && (time(NULL) - call_matrix[9][5] > 5) && state->carrier == 1)
  {
    for (short int k = 0; k < 9; k++)
    {
      call_matrix[k][0] = call_matrix[k+1][0];
      call_matrix[k][1] = call_matrix[k+1][1];
      call_matrix[k][2] = call_matrix[k+1][2];
      call_matrix[k][3] = call_matrix[k+1][3];
      call_matrix[k][4] = call_matrix[k+1][4];
      call_matrix[k][5] = call_matrix[k+1][5];
    }

    call_matrix[9][0] = lls;
    call_matrix[9][1] = 1;
    call_matrix[9][2] = 1;
    call_matrix[9][3] = 1;
    call_matrix[9][4] = 1;
    call_matrix[9][5] = time(NULL);

  }

  //NXDN
  if ( call_matrix[9][2] != src && src > 0 && rn > -1 )
  {
    for (short int k = 0; k < 9; k++)
    {
      call_matrix[k][0] = call_matrix[k+1][0];
      call_matrix[k][1] = call_matrix[k+1][1];
      call_matrix[k][2] = call_matrix[k+1][2];
      call_matrix[k][3] = call_matrix[k+1][3];
      call_matrix[k][4] = call_matrix[k+1][4];
      call_matrix[k][5] = call_matrix[k+1][5];
    }
    call_matrix[9][0] = lls;
    call_matrix[9][1] = rn;
    call_matrix[9][2] = src;
    call_matrix[9][3] = 0;
    call_matrix[9][4] = tgn;
    call_matrix[9][5] = time(NULL);

  }

  //DMR MS
  if ( call_matrix[9][2] != rd && (lls == 32 || lls == 33 || lls == 34) )
  {

    for (short int k = 0; k < 10; k++)
    {
      call_matrix[k][0] = call_matrix[k+1][0];
      call_matrix[k][1] = call_matrix[k+1][1];
      call_matrix[k][2] = call_matrix[k+1][2];
      call_matrix[k][3] = call_matrix[k+1][3];
      call_matrix[k][4] = call_matrix[k+1][4];
      call_matrix[k][5] = call_matrix[k+1][5];
    }

    call_matrix[9][0] = lls;
    call_matrix[9][1] = tg;
    call_matrix[9][2] = rd;
    call_matrix[9][3] = 1; //hard set slot number
    call_matrix[9][4] = dcc;
    call_matrix[9][5] = time(NULL);

  }

  //DMR BS Slot 1 - matrix 0-4
  if ( call_matrix[4][2] != rd && (lls == 12 || lls == 13 || lls == 10 || lls == 11) )
  {

    for (short int k = 0; k < 4; k++)
    {
      call_matrix[k][0] = call_matrix[k+1][0];
      call_matrix[k][1] = call_matrix[k+1][1];
      call_matrix[k][2] = call_matrix[k+1][2];
      call_matrix[k][3] = call_matrix[k+1][3];
      call_matrix[k][4] = call_matrix[k+1][4];
      call_matrix[k][5] = call_matrix[k+1][5];
    }

    call_matrix[4][0] = lls;
    call_matrix[4][1] = tg;
    call_matrix[4][2] = rd;
    call_matrix[4][3] = 1; //hard set slot number
    call_matrix[4][4] = dcc;
    call_matrix[4][5] = time(NULL);

  }

  //DMR BS Slot 2 - matrix 5-9
  if ( call_matrix[9][2] != rdR && (lls == 12 || lls == 13 || lls == 10 || lls == 11) )
  {

    for (short int k = 5; k < 9; k++)
    {
      call_matrix[k][0] = call_matrix[k+1][0];
      call_matrix[k][1] = call_matrix[k+1][1];
      call_matrix[k][2] = call_matrix[k+1][2];
      call_matrix[k][3] = call_matrix[k+1][3];
      call_matrix[k][4] = call_matrix[k+1][4];
      call_matrix[k][5] = call_matrix[k+1][5];
    }

    call_matrix[9][0] = lls;
    call_matrix[9][1] = tgR;
    call_matrix[9][2] = rdR;
    call_matrix[9][3] = 2; //hard set slot number
    call_matrix[9][4] = dcc;
    call_matrix[9][5] = time(NULL);

  }

  //P25
  if ( (lls == 0 || lls == 1) && call_matrix[9][2] != rd && nc > 0 && tg > 0)
  {
    for (short int k = 0; k < 9; k++)
    {
      call_matrix[k][0] = call_matrix[k+1][0];
      call_matrix[k][1] = call_matrix[k+1][1];
      call_matrix[k][2] = call_matrix[k+1][2];
      call_matrix[k][3] = call_matrix[k+1][3];
      call_matrix[k][4] = call_matrix[k+1][4];
      call_matrix[k][5] = call_matrix[k+1][5];
    }

    call_matrix[9][0] = lls;
    call_matrix[9][1] = tg;
    call_matrix[9][2] = rd;
    call_matrix[9][3] = 0;
    call_matrix[9][4] = nc;
    call_matrix[9][5] = time(NULL);

  }

  //Start Printing Section
  erase();
  printw ("%s \n", FM_bannerN[0]); //top line in white
  attron(COLOR_PAIR(4));
  for (short int i = 1; i < 7; i++) //following lines in cyan
  {
    printw("%s \n", FM_bannerN[i]);
  }
  attroff(COLOR_PAIR(4));
  printw ("--Build Info------------------------------------------------------------------\n");
  printw ("| %s \n", FM_bannerN[7]); //http link
  printw ("| Digital Speech Decoder: Florida Man Edition\n");
  printw ("| Github Build Version: %s \n", GIT_TAG);
  printw ("| mbelib version %s\n", versionstr);
  //printw ("| Press CTRL+C twice to exit\n");
  printw ("------------------------------------------------------------------------------\n");
  attron(COLOR_PAIR(4));
  printw ("--Input Output----------------------------------------------------------------\n");
  if (opts->audio_in_type == 0)
  {
    printw ("| Pulse Audio  Input: [%i] kHz [%i] Channel\n", opts->pulse_digi_rate_in/1000, opts->pulse_digi_in_channels);
  }
  if (opts->audio_in_type == 1)
  {
    printw ("| STDIN Standard Input: - \n");
  }
  if (opts->audio_in_type == 3)
  {
    printw ("| RTL2838UHIDIR Device #[%d]", opts->rtl_dev_index);
    printw (" Gain [%i] dB -", opts->rtl_gain_value);
    printw (" Squelch [%i]", opts->rtl_squelch_level);
    printw (" VFO [%i] kHz\n", opts->rtl_bandwidth);
    printw ("| Freq: [%lld] Hz", opts->rtlsdr_center_freq);
    printw (" - Tuning available on UDP Port [%i]\n", opts->rtl_udp_port);
  }
  if (opts->audio_out_type == 0)
  {
    printw ("| Pulse Audio Output: [%i] kHz [%i] Channel\n", opts->pulse_digi_rate_out/1000, opts->pulse_digi_out_channels);
  }
  if (opts->monitor_input_audio == 1)
  {
    printw ("| Monitoring Source Audio when Carrier Present and No Sync Detected\n");
  }
  if (opts->mbe_out_dir[0] != 0 && opts->dmr_stereo == 0)
  {
    printw ("| Writing MBE data files to directory %s\n", opts->mbe_out_dir);
  }
  if (opts->wav_out_file[0] != 0 && opts->dmr_stereo == 0)
  {
    printw ("| Appending Audio WAV to file %s\n", opts->wav_out_file);
  }

  printw ("------------------------------------------------------------------------------\n");
  attroff(COLOR_PAIR(4));

  if (state->carrier == 1){ //figure out method that will tell me when is active and when not active, maybe carrier but this doesn't print anyways unless activity
    attron(COLOR_PAIR(3));
    level = (int) state->max / 164; //only update on carrier present
    reset = 1;
  }
  if (state->carrier == 0 && opts->reset_state == 1 && reset == 1)
  {
    resetState (state);
    reset = 0;
  }

  printw ("--Audio Decode----------------------------------------------------------------\n");
  printw ("| Decoding:    [%s] \n", opts->output_name);
  printw ("| In Level:    [%3i%%] \n", level);
  if (opts->dmr_stereo == 0)
  {
    printw ("| Voice Error: [%i][%i] \n", state->errs, state->errs2);
  }

  if (opts->dmr_stereo == 1)
  {
    printw ("| Voice ErrS1: [%i][%i] \n", state->errs, state->errs2);
    printw ("| Voice ErrS2: [%i][%i] \n", state->errsR, state->errs2R);
  }
  printw ("------------------------------------------------------------------------------\n");

  printw ("--Call Info-------------------------------------------------------------------\n");
  //DSTAR...what a pain...
  if (lls == 6 || lls == 7 || lls == 18 || lls == 19)
  {
    if (state->dstarradioheader[3] != 0) //
    {
      printw ("| RPT 2: [%c%c%c%c%c%c%c%c] ", state->dstarradioheader[3], state->dstarradioheader[4],
  			state->dstarradioheader[5], state->dstarradioheader[6], state->dstarradioheader[7], state->dstarradioheader[8],
  			state->dstarradioheader[9], state->dstarradioheader[10]);
      printw ("RPT 1: [%c%c%c%c%c%c%c%c] \n| ", state->dstarradioheader[11], state->dstarradioheader[12],
  			state->dstarradioheader[13], state->dstarradioheader[14], state->dstarradioheader[15], state->dstarradioheader[16],
  			state->dstarradioheader[17], state->dstarradioheader[18]);
      printw ("YOUR:  [%c%c%c%c%c%c%c%c] ", state->dstarradioheader[19], state->dstarradioheader[20],
  			state->dstarradioheader[21], state->dstarradioheader[22], state->dstarradioheader[23], state->dstarradioheader[24],
  			state->dstarradioheader[25], state->dstarradioheader[26]);
      printw ("MY: [%c%c%c%c%c%c%c%c] [%c%c%c%c]\n", state->dstarradioheader[27],
  			state->dstarradioheader[28], state->dstarradioheader[29], state->dstarradioheader[30], state->dstarradioheader[31],
  			state->dstarradioheader[32], state->dstarradioheader[33], state->dstarradioheader[34], state->dstarradioheader[35],
  			state->dstarradioheader[36], state->dstarradioheader[37], state->dstarradioheader[38]);
    }
  }

  //NXDN
  if (lls == 8 || lls == 9 || lls == 16 || lls == 17)
  {
    printw ("| RAN: [%2d] ", rn);
    printw ("TID: [%4d] ", tgn);
    printw ("RID: [%4d] \n| ALG: [0x%02X] KEY [0x%02X] ", src, state->nxdn_cipher_type, state->nxdn_key);
    if (state->carrier == 1)
    {
      printw("%s ", state->nxdn_call_type);
    }
    if (state->nxdn_cipher_type == 0x1 && state->carrier == 1)
    {
      attron(COLOR_PAIR(2));
      printw ("Scrambler Encryption ");
      attroff(COLOR_PAIR(2));
      attron(COLOR_PAIR(3));
    }
    if (state->nxdn_cipher_type == 0x2 && state->carrier == 1)
    {
      attron(COLOR_PAIR(2));
      printw ("DES Encryption ");
      attroff(COLOR_PAIR(2));
      attron(COLOR_PAIR(3));
    }
    if (state->nxdn_cipher_type == 0x3 && state->carrier == 1)
    {
      attron(COLOR_PAIR(2));
      printw ("AES Encryption ");
      attroff(COLOR_PAIR(2));
      attron(COLOR_PAIR(3));
    }
    if (state->nxdn_cipher_type > 0x3 && state->carrier == 1)
    {
      attron(COLOR_PAIR(2));
      printw ("Unknown Encryption ");
      attroff(COLOR_PAIR(2));
      attron(COLOR_PAIR(3));
    }
    //printw("%s ", state->nxdn_call_type);
    printw("\n");
  }

  //P25
  if (lls == 0 || lls == 1)
  {
    printw("| TID:[%8i] RID:[%8i] ", tg, rd);
    printw("NAC: [0x%3X] \n", nc);
    printw("| ALG:[0x%02X] ", state->payload_algid);
    printw("    KEY:[0x%04X] ", state->payload_keyid);
    printw("  MFID: [0x%02X] ", state->payload_mfid); //no way of knowing if this is accurate info yet
    if (state->payload_algid != 0x80 && state->payload_algid != 0x0 && state->carrier == 1)
    {
      attron(COLOR_PAIR(2));
      printw("**ENC**");
      attroff(COLOR_PAIR(2));
      attron(COLOR_PAIR(3));
    }
    printw("\n");
  }

  //DMR BS/MS Voice and Data Types
  if ( lls == 12 || lls == 13 || lls == 10 || lls == 11 || lls == 32 || lls == 33 || lls == 34)
  {
    printw ("| ");
    if (lls < 30)
    {
      printw ("DMR BS - DCC: [%02i] ", dcc);
    }
    else
    {
      printw ("DMR MS - DCC: [%02i] ", dcc);
    }

    printw ("\n");
    //Slot 1 [0]
    printw ("| SLOT 1 - ");
    if (state->dmrburstL != 16 && state->carrier == 1 && state->lasttg > 0 && state->lastsrc > 0)
    {
      attron(COLOR_PAIR(2));
    }
    printw ("TGT: [%8i] SRC: [%8i] ", state->lasttg, state->lastsrc);
    if (state->dmrburstL != 16 && state->carrier == 1 && state->lasttg > 0 && state->lastsrc > 0)
    {
      attroff(COLOR_PAIR(2));
      attron(COLOR_PAIR(3));
    }
    printw ("FID: [%02X] SVC: [%02X] ", state->dmr_fid, state->dmr_so);
    printw ("%s ", DMRBusrtTypes[state->dmrburstL]);
    printw ("\n");
    //printw ("|        | "); //10 spaces
    printw ("| V XTRA | "); //10 spaces
    //Burger King
    if(state->dmrburstL == 16 && state->payload_mi == 0 && (state->dmr_so & 0xCF) == 0x40) //4F or CF mask?
    {
      attron(COLOR_PAIR(5));
      printw (" **BP** ");
      attroff(COLOR_PAIR(5));
      attron(COLOR_PAIR(3));
    }
    //Point
    if(state->dmrburstL == 16 && state->payload_mi == 0 && state->K > 0 && (state->dmr_so & 0xCF) == 0x40)
    {
      attron(COLOR_PAIR(1));
      printw ("BPK [%3lld] ", state->K);
      attroff(COLOR_PAIR(1));
      attron(COLOR_PAIR(3));
    }
    //ALG, KeyID, MI
    if(state->dmrburstL == 16 && state->payload_keyid > 0 && (state->dmr_so & 0xCF) == 0x40)
    {
      attron(COLOR_PAIR(1));
      printw ("ALG: [0x%02X] KEY: [0x%02X] MI: [0x%08X]", state->payload_algid, state->payload_keyid, state->payload_mi);
      attroff(COLOR_PAIR(1));
      attron(COLOR_PAIR(3));
    }

    if(state->dmrburstL == 16 && state->dmr_so == 0x40) //0100 0000
    {
      attron(COLOR_PAIR(2));
      printw (" **ENC** ");
      attroff(COLOR_PAIR(2));
      attron(COLOR_PAIR(3));
    }
    if(state->dmrburstL == 16 && state->dmr_so == 0x80)
    {
      attron(COLOR_PAIR(2));
      printw (" **Emergency** ");
      attroff(COLOR_PAIR(2));
      attron(COLOR_PAIR(3));
    }
    if(state->dmrburstL == 16 && state->dmr_so == 0x30) //0010 0000
    {
      attron(COLOR_PAIR(2));
      printw (" **Private Call** ");
      attroff(COLOR_PAIR(2));
      attron(COLOR_PAIR(3));
    }
    printw ("\n");
    //Alias Blocks and Embedded GPS
    //printw ("|        | "); //10 spaces
    printw ("| D XTRA | ");
    if(state->dmrburstL == 16) //only during call
    {
      attron(COLOR_PAIR(5));
      for (short i = 0; i < 5; i++)
      {
        printw ("%s", state->dmr_callsign[0][i]);
      }
      //Embedded GPS (not LRRP)
      printw ("%s", state->dmr_callsign[0][5] );
      attroff(COLOR_PAIR(5));
      if (state->carrier == 1)
      {
        attron(COLOR_PAIR(3));
      }

      //LRRP
      if(state->dmrburstL != 16) //only during data
      {
        attron(COLOR_PAIR(5));
        for (short i = 0; i < 5; i++)
        {
          printw ("%s", state->dmr_lrrp[0][i]);
        }
        attroff(COLOR_PAIR(5));
        if (state->carrier == 1)
        {
          attron(COLOR_PAIR(3));
        }
      }
    }
    printw ("\n");

    //Slot 2 [1]
    if (lls < 30){ //Don't print on MS mode
    printw ("| SLOT 2 - ");
    if (state->dmrburstR != 16 && state->carrier == 1 && state->lasttgR > 0 && state->lastsrcR > 0)
    {
      attron(COLOR_PAIR(2));
    }
    printw ("TGT: [%8i] SRC: [%8i] ", state->lasttgR, state->lastsrcR);
    if (state->dmrburstR != 16 && state->carrier == 1 && state->lasttgR > 0 && state->lastsrcR > 0)
    {
      attroff(COLOR_PAIR(2));
      attron(COLOR_PAIR(3));
    }
    printw ("FID: [%02X] SVC: [%02X] ", state->dmr_fidR, state->dmr_soR);
    printw ("%s ", DMRBusrtTypes[state->dmrburstR]);
    printw ("\n");
    //printw ("|        | "); //12 spaces
    printw ("| V XTRA | "); //10 spaces

    //Burger King 2
    if(state->dmrburstR == 16 && state->payload_miR == 0 && (state->dmr_soR & 0xCF) == 0x40) //4F or CF mask?
    {
      attron(COLOR_PAIR(5));
      printw (" **BP** ");
      attroff(COLOR_PAIR(5));
      attron(COLOR_PAIR(3));
    }
    //Point 2
    if(state->dmrburstR == 16 && state->payload_miR == 0 && state->K > 0 && (state->dmr_soR & 0xCF) == 0x40)
    {
      attron(COLOR_PAIR(1));
      printw ("BPK [%3lld] ", state->K);
      attroff(COLOR_PAIR(1));
      attron(COLOR_PAIR(3));
    }
    //ALG, KeyID, MI 2
    if(state->dmrburstR == 16 && state->payload_keyidR > 0 && (state->dmr_soR & 0xCF) == 0x40)
    {
      attron(COLOR_PAIR(1));
      printw ("ALG: [0x%02X] KEY: [0x%02X] MI: [0x%08X]", state->payload_algidR, state->payload_keyidR, state->payload_miR);
      attroff(COLOR_PAIR(1));
      attron(COLOR_PAIR(3));
    }
    //Call Types, may switch to the more robust version later?
    if(state->dmrburstR == 16 && state->dmr_soR == 0x40) //0100 0000
    {
      attron(COLOR_PAIR(2));
      printw (" **ENC** ");
      attroff(COLOR_PAIR(2));
      attron(COLOR_PAIR(3));
    }
    if(state->dmrburstR == 16 && state->dmr_soR == 0x80)
    {
      attron(COLOR_PAIR(2));
      printw (" **Emergency** ");
      attroff(COLOR_PAIR(2));
      attron(COLOR_PAIR(3));
    }
    if(state->dmrburstR == 16 && state->dmr_soR == 0x30) //0010 0000
    {
      attron(COLOR_PAIR(2));
      printw (" **Private Call** ");
      attroff(COLOR_PAIR(2));
      attron(COLOR_PAIR(3));
    }
    printw ("\n");
    //Alias Blocks and Embedded GPS
    //printw ("|        | ");
    printw ("| D XTRA | ");
    if(state->dmrburstR == 16) //only during call
    {
      attron(COLOR_PAIR(5));
      for (short i = 0; i < 5; i++)
      {
        printw ("%s", state->dmr_callsign[1][i]);
      }
      //Embedded GPS (not LRRP)
      printw ("%s", state->dmr_callsign[1][5] );
      attroff(COLOR_PAIR(5));
      if (state->carrier == 1)
      {
        attron(COLOR_PAIR(3));
      }
    }
    //LRRP
    if(state->dmrburstR != 16) //only during data
    {
      attron(COLOR_PAIR(5));
      for (short i = 0; i < 5; i++)
      {
        printw ("%s", state->dmr_lrrp[1][i]);
      }
      attroff(COLOR_PAIR(5));
      if (state->carrier == 1)
      {
        attron(COLOR_PAIR(3));
      }
    }
    printw ("\n");
  }  // end if not MS
  } //end DMR BS Types

  //dPMR
  if (lls == 20 || lls == 21 || lls == 22 || lls == 23 ||lls == 24 || lls == 25 || lls == 26 || lls == 27)
  {
    printw ("| DCC: [%i] ", state->dpmr_color_code);
    printw ("TID: [%s] RID: [%s] \n", state->dpmr_target_id, state->dpmr_caller_id);
  }

  if (lls == 6 || lls == 7 || lls == 18 || lls == 19 || lls == 14 || lls == 15)
  {
    printw ("| %s ", SyncTypes[lls]);
    //printw ("%s", state->dmr_branding);
    printw ("\n");
  }
  //fence bottom
  printw ("------------------------------------------------------------------------------\n");
  //colors off
  if (state->carrier == 1){ //same as above
    attroff(COLOR_PAIR(3));
  }

  attron(COLOR_PAIR(4)); //cyan for history
  printw ("--Call History----------------------------------------------------------------\n");
  for (short int j = 0; j < 10; j++)
  {
    //only print if a valid time was assinged to the matrix
    if ( ((time(NULL) - call_matrix[9-j][5]) < 9999)  )
    {
      printw ("| %s ", SyncTypes[call_matrix[9-j][0]]);
      if (lls == 8 || lls == 9 || lls == 16 || lls == 17)
      {
        printw ("RAN [%2lld] ", call_matrix[9-j][1]);
        printw ("TG [%4lld] ", call_matrix[9-j][4]);
        printw ("RID [%4lld] ", call_matrix[9-j][2]);
      }
      //dPMR
      if (lls == 20 || lls == 21 || lls == 22 || lls == 23 ||lls == 24 || lls == 25 || lls == 26 || lls == 27)
      {
        printw ("TGT [%8lld] ", call_matrix[9-j][1]);
        printw ("SRC [%8lld] ", call_matrix[9-j][2]);
        printw ("DCC [%2lld] ", call_matrix[9-j][4]);
      }
      //P25P1 Voice and Data
      if (call_matrix[9-j][0] == 0 || call_matrix[9-j][0] == 1)
      {
        printw ("TID [%8lld] ", call_matrix[9-j][1]);
        printw ("RID [%8lld] ", call_matrix[9-j][2]);
        printw ("NAC [0x%03llX] ", call_matrix[9-j][4]);
      }
      //DMR BS Types
      if (call_matrix[9-j][0] == 12 || call_matrix[9-j][0] == 13 || call_matrix[9-j][0] == 10 || call_matrix[9-j][0] == 11 )
      {
        printw ("S[%d] ", call_matrix[9-j][3]);
        printw ("TGT [%8lld] ", call_matrix[9-j][1]);
        printw ("SRC [%8lld] ", call_matrix[9-j][2]);
        printw ("DCC [%02lld] ", call_matrix[9-j][4]);
      }
      //DMR MS Types
      if (call_matrix[9-j][0] == 32 || call_matrix[9-j][0] == 33 || call_matrix[9-j][0] == 34 )
      {
        //printw ("S[%d] ", call_matrix[9-j][3]);
        printw ("TGT [%8lld] ", call_matrix[9-j][1]);
        printw ("SRC [%8lld] ", call_matrix[9-j][2]);
        printw ("DCC [%02lld] ", call_matrix[9-j][4]);
      }
      printw ("%lld secs ago\n", time(NULL) - call_matrix[9-j][5]);
    }
  } //end Call History
  //fence bottom
 printw ("------------------------------------------------------------------------------\n");
 attroff(COLOR_PAIR(4)); //cyan for history


 refresh();
} //end ncursesPrinter

void ncursesClose ()
{
  endwin();
  printf("Press CTRL+C again to close. Thanks.\n");
  printf("Run 'reset' in your terminal to clean up if necessary.");
}
