/*-------------------------------------------------------------------------------
 * p25p1_tsbk.c
 * P25p1 Trunking Signal Block Handler
 *
 * LWVMOBILE
 * 2022-10 DSD-FME Florida Man Edition
 *-----------------------------------------------------------------------------*/

#include "dsd.h"

void processTSBK(dsd_opts * opts, dsd_state * state)
{

  //reset some strings when returning from a call in case they didn't get zipped already
  sprintf (state->call_string[0], "%s", "                     "); //21 spaces
  sprintf (state->call_string[1], "%s", "                     "); //21 spaces

  //clear stale Active Channel messages here
  if ( (time(NULL) - state->last_active_time) > 3 )
  {
    memset (state->active_channel, 0, sizeof(state->active_channel));
  }
  
  int tsbkbit[196]; //tsbk bit array, 196 trellis encoded bits
  int tsbk_dibit[98];

  memset (tsbkbit, 0, sizeof(tsbkbit));
  memset (tsbk_dibit, 0, sizeof(tsbk_dibit));

  int dibit = 0;

  uint8_t tsbk_byte[12]; //12 byte return from bd_bridge (block_deinterleave)
  memset (tsbk_byte, 0, sizeof(tsbk_byte));

  unsigned long long int PDU[24]; //24 byte PDU to send to the tsbk_byte vPDU handler, should be same formats (mostly)
  memset (PDU, 0, sizeof(PDU));

  int tsbk_decoded_bits[190]; //decoded bits from tsbk_bytes for sending to crc16_lb_bridge
  memset (tsbk_decoded_bits, 0, sizeof(tsbk_decoded_bits));

  int flushing_bits[196]; //for flushing the trellis state machine
  memset (flushing_bits, 0, sizeof(flushing_bits));
  
  int i, j, k, b, x, y;
  int ec = -2; //error value returned from (block_deinterleave)
  int err = -2; //error value returned from crc16_lb_bridge
  int skipdibit = 14; //initial status dibit will occur at 14, then add 36 each time it occurs
  int protectbit = 0;
  int MFID = 0xFF; //Manufacturer ID - Might be beneficial to NOT send anything but standard 0x00 or 0x01 messages

  //TODO: Don't Print TSBKs/vPDUs unless verbosity levels set higher

  //collect three reps of 101 dibits (98 valid dibits with status dibits interlaced)
  for (j = 0; j < 3; j++)
  {
    k = 0;
    for (i = 0; i < 101; i++)
    {

      dibit = getDibit(opts, state);

      if (i+(j*101) != skipdibit) //
      {
        tsbk_dibit[k] =   dibit;
        tsbkbit[k*2]   = (dibit >> 1) & 1;
        tsbkbit[k*2+1] = (dibit & 1);
        k++;
      }

      if (i+(j*101) == skipdibit) 
      {
        skipdibit += 36;
      }

    }

    //flushing the trellis state machine
    if (j == 0)
    {
      bd_bridge(flushing_bits, tsbk_byte);
      //reset tsbk_byte afterwards
      memset (tsbk_byte, 0, sizeof(tsbk_byte));
    }

    //send tsbkbit to block_deinterleave and return tsbk_byte
    ec = bd_bridge(tsbkbit, tsbk_byte);

    //too many bit manipulations!
    k = 0;
    for (i = 0; i < 12; i++)
    {
      for (x = 0; x < 8; x++)
      {
        tsbk_decoded_bits[k] = ((tsbk_byte[i] << x) & 0x80) >> 7;
        k++;
      }
    }

    err = crc16_lb_bridge(tsbk_decoded_bits, 80);

    //shuffle corrected bits back into tsbk_byte
    k = 0;
    for (i = 0; i < 12; i++)
    {
      int byte = 0;
      for (x = 0; x < 8; x++)
      {
        byte = byte << 1;
        byte = byte | tsbk_decoded_bits[k];
        k++;
      }
      tsbk_byte[i] = byte;
    }

    //convert tsbk_byte to vPDU and send to vPDU handler
    //...may or may not be entirely compatible,
    MFID   = tsbk_byte[1];
    PDU[0] = 0x07; //P25p1 TSBK Duid 0x07
    PDU[1] = tsbk_byte[0] & 0x3F;
    PDU[2] = tsbk_byte[2];
    PDU[3] = tsbk_byte[3];
    PDU[4] = tsbk_byte[4];
    PDU[5] = tsbk_byte[5];
    PDU[6] = tsbk_byte[6];
    PDU[7] = tsbk_byte[7];
    PDU[8] = tsbk_byte[8];
    PDU[9] = tsbk_byte[9];
    //remove CRC to prevent false positive when vPDU goes to look for additional message in block
    PDU[10] = 0; //tsbk_byte[10]; 
    PDU[11] = 0; //tsbk_byte[11];
    PDU[1] = PDU[1] ^ 0x40; //flip bit to make it compatible with MAC_PDUs, i.e. 3D to 7D

    //check the protect bit, don't run if protected
    protectbit = (tsbk_byte[0] >> 6) & 0x1;

    if (opts->payload == 1)
    {
      fprintf (stderr, "\n%s TSBK%d %sPayload ", KNRM, j+1, KCYN);
      for (i = 0; i < 12; i++)
      {
        fprintf (stderr, "%02X", tsbk_byte[i]);
      }
    }

    //Don't run NET_STS out of this, or will set wrong NAC/CC //&& PDU[1] != 0x49 && PDU[1] != 0x45
    //0x49 is telephone grant, 0x46 Unit to Unit Channel Answer Request (seems bogus) (mfid 90)
    // if (MFID < 0x2 && protectbit == 0 && err == 0 && ec == 0 && PDU[1] != 0x7B )
    //running with A4 and 90 causing some false positives when sending to vPDU, issuing bad call grants to nowhere!
    // if ( (MFID < 0x2 || MFID == 0xA4 || MFID == 0x90) && protectbit == 0 && err == 0 && ec == 0 && PDU[1] != 0x7B )  
    if (MFID < 0x2 && protectbit == 0 && err == 0 && ec == 0 && PDU[1] != 0x7B ) 
    {
      fprintf (stderr, "%s",KYEL);
      process_MAC_VPDU(opts, state, 0, PDU);
      fprintf (stderr, "%s",KNRM);
    }

    //set our WACN and SYSID here now that we have valid ec and crc/checksum
    else if (protectbit == 0 && err == 0 && ec == 0 && (tsbk_byte[0] & 0x3F) == 0x3B) 
    {
      long int wacn = (tsbk_byte[3] << 12) | (tsbk_byte[4] << 4) | (tsbk_byte[5] >> 4);
      int sysid = ((tsbk_byte[5] & 0xF) << 8) | tsbk_byte[6];
      int channel = (tsbk_byte[7] << 8) | tsbk_byte[8];
      fprintf (stderr, "%s",KYEL);
      fprintf (stderr, "\n Network Status Broadcast TSBK - Abbreviated \n");
      fprintf (stderr, "  WACN [%05lX] SYSID [%03X] NAC [%03llX]", wacn, sysid, state->p2_cc);
      state->p25_cc_freq = process_channel_to_freq(opts, state, channel);
      state->p25_cc_is_tdma = 0; //flag off for CC tuning purposes when system is qpsk

      //place the cc freq into the list at index 0 if 0 is empty, or not the same, 
      //so we can hunt for rotating CCs without user LCN list
      if (state->trunk_lcn_freq[0] == 0 || state->trunk_lcn_freq[0] != state->p25_cc_freq)
      {
        state->trunk_lcn_freq[0] = state->p25_cc_freq; 
      } 

      //only set IF these values aren't already hard set by the user
      if (state->p2_hardset == 0)
      {
        state->p2_wacn = wacn;
        state->p2_sysid = sysid;
      }  

      if (opts->payload == 1)
      {
          fprintf (stderr, "%s", KCYN);
          fprintf (stderr, "\n P25 MAC PDU Payload\n  ");
          for (int i = 0; i < 12; i++)
          {
              fprintf (stderr, "%02llX", tsbk_byte[i]);
              if (i == 11) fprintf (stderr, "\n  ");
          }
          fprintf (stderr, "%s", KNRM);
      }
    }

    if (opts->payload == 1)
    {
      fprintf (stderr, "%s",KMAG);
      fprintf (stderr, "\n LCO %02X MFID %02X Protected: %d Last Block: %d", PDU[1] ^ 0x40, MFID, protectbit, (tsbk_byte[0] >> 7) );
        
      if (ec != 0) 
      {
        fprintf (stderr, "%s",KRED);
        fprintf (stderr, " (FEC ERR)");
      }
      else if (err != 0) 
      {
        fprintf (stderr, "%s",KRED);
        fprintf (stderr, " (CRC ERR)");
      }
      fprintf (stderr, "%s ", KNRM);
    }

    //reset for next rep
    ec = -2;
    err = -2;
    protectbit = 0;
    MFID = 0xFF;

    //check for last block bit
    if ( (tsbk_byte[0] >> 7) == 1 ) 
    {
      j = 4; //set j to break the loop early
    }
  }

  fprintf (stderr, "%s ", KNRM);
  fprintf (stderr, "\n"); 
}
