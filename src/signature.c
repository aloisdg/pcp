/*
    This file is part of Pretty Curved Privacy (pcp1).

    Copyright (C) 2013 T.Linden.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    You can contact me by mail: <tlinden AT cpan DOT org>.
*/


#include "signature.h"
#include "defines.h"

int pcpsign(char *infile, char *outfile, char *passwd, int z85) {
  FILE *in = NULL;
  FILE *out = NULL;
  pcp_key_t *secret = NULL;

  secret = pcp_find_primary_secret();

  if(secret == NULL) {
    fatal("Could not find a secret key in vault %s!\n", vault->filename);
    goto errs1;
  }

  if(infile == NULL)
    in = stdin;
  else {
    if((in = fopen(infile, "rb")) == NULL) {
      fatal("Could not open input file %s\n", infile);
      goto errs1;
    }
  }

  if(outfile == NULL)
    out = stdout;
  else {
    if((out = fopen(outfile, "wb+")) == NULL) {
      fatal("Could not open output file %s\n", outfile);
      goto errs1;
    }
  }

  if(secret->secret[0] == 0) {
    // encrypted, decrypt it
    char *passphrase;
    if(passwd == NULL) {
      pcp_readpass(&passphrase,
		   "Enter passphrase to decrypt your secret key", NULL, 1);
    }
    else {
      passphrase = ucmalloc(strlen(passwd)+1);
      strncpy(passphrase, passwd, strlen(passwd)+1);
    }

    secret = pcpkey_decrypt(secret, passphrase);
    if(secret == NULL)
      goto errs1;
  }

  size_t sigsize = pcp_ed_sign_buffered(in, out, secret, z85);

  if(sigsize == 0)
    goto errs1;

  fprintf(stderr, "Signed %ld bytes successfully\n", sigsize);

  return 0;

 errs1:
  return 1;
}

int pcpverify(char *infile, char *id) {
  FILE *in = NULL;
  pcp_pubkey_t *pub = NULL;
  unsigned char *message = NULL;

  if(infile == NULL)
    in = stdin;
  else {
    if((in = fopen(infile, "rb")) == NULL) {
      fatal("Could not open input file %s\n", infile);
      goto errv1;
    }
  }

  if(id != NULL)
    HASH_FIND_STR(pcppubkey_hash, id, pub);
 
  if(pub != NULL) {
    message = pcp_ed_verify_buffered(in, pub);
    if(message != NULL) {
      fprintf(stderr, "Signature verified (signed by %s <%s>).\n", pub->owner, pub->mail);
    }
  }
  else {
    // put public key as pub, so verify iterates over our keys
    message = pcp_ed_verify_buffered(in, pub);
    if(message != NULL) {
      fprintf(stderr, "Signature verified (signed by %s <%s>).\n", pub->owner, pub->mail);
    }
  }

  if(message != NULL) {
    free(message);
    return 0;
  }

 errv4:

 errv1:
  return 1;
}

