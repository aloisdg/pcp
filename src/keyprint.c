#include "keyprint.h"


void pcptext_key(char *keyid) {
  pcp_key_t *s = pcpkey_exists(keyid);
  if(s != NULL) {
    pcpkey_print(s, stdout);
    free(s);
  }
  else {
    pcp_pubkey_t *p = pcppubkey_exists(keyid);
    if(p != NULL) {
      pcppubkey_print(p, stdout);
      free(p);
    }
    else {
      fatal("No key with id 0x%s found!\n", keyid);
    }
  }
}

void pcptext_vault(vault_t *vault) {
  printf("    Key vault: %s\n", vault->filename);
  printf("Vault version: %08X\n", vault->version);
  printf("     Checksum: ");

  int i;
  for ( i = 0;i <15 ;++i) printf("%02X:",(unsigned int) vault->checksum[i]);
  printf("%02X", vault->checksum[15]);
  printf("\n               ");
  for ( i = 16;i <31 ;++i) printf("%02X:",(unsigned int) vault->checksum[i]);
  printf("%02X", vault->checksum[31]);
  printf("\n");

  printf("  Secret keys: %d\n", HASH_COUNT(pcpkey_hash));
  printf("  Public keys: %d\n",  HASH_COUNT(pcppubkey_hash));
}

void pcpkey_printlineinfo(pcp_key_t *key) {
  struct tm *c;
  time_t t = (time_t)key->ctime;
  c = localtime(&t);
  printf("0x%s   %s   %04d-%02d-%02dT%02d:%02d:%02d  %s <%s>\n",
	 key->id,
	 (key->type ==  PCP_KEY_TYPE_MAINSECRET) ? "primary" : " secret",  
	 c->tm_year+1900, c->tm_mon+1, c->tm_mday,
	 c->tm_hour, c->tm_min, c->tm_sec,
	 key->owner, key->mail);
}

void pcppubkey_printlineinfo(pcp_pubkey_t *key) {
  struct tm *c;
  time_t t = (time_t)key->ctime;
  c = localtime(&t);
  printf("0x%s    public   %04d-%02d-%02dT%02d:%02d:%02d  %s <%s>\n",
	 key->id,
	 c->tm_year+1900, c->tm_mon+1, c->tm_mday,
	 c->tm_hour, c->tm_min, c->tm_sec,
	 key->owner, key->mail);
}


void pcpkey_print(pcp_key_t *key, FILE* out) {
  size_t zlen;
  key2be(key);
  char *z85encoded = pcp_z85_encode((unsigned char*)key, sizeof(pcp_key_t), &zlen);
  key2native(key);

  struct tm *c;
  time_t t = (time_t)key->ctime;
  c = localtime(&t);

  fprintf(out, "%s\n", PCP_KEY_HEADER);

  fprintf(out, "  Generated by: %s Version %d.%d.%d\n",
	  PCP_ME, PCP_VERSION_MAJOR, PCP_VERSION_MINOR, PCP_VERSION_PATCH);

  fprintf(out, "        Cipher: %s\n", PCP_KEY_PRIMITIVE);

  fprintf(out, "        Key-ID: 0x%s\n", key->id);

  //2004-06-14T23:34:30.
  fprintf(out, " Creation Time: %04d-%02d-%02dT%02d:%02d:%02d\n",
	 c->tm_year+1900, c->tm_mon+1, c->tm_mday,
	 c->tm_hour, c->tm_min, c->tm_sec);

  fprintf(out, " Serial Number: 0x%08X\n", key->serial);
  fprintf(out, "   Key Version: 0x%08X\n", key->version);
  
  fprintf(out, "\n%s\n", z85encoded);

  fprintf(out, "%s\n", PCP_KEY_FOOTER);

  free(z85encoded);
}

void pcppubkey_print(pcp_pubkey_t *key, FILE* out) {
  size_t zlen;
  pubkey2be(key);
  char *z85encoded = pcp_z85_encode((unsigned char*)key, sizeof(pcp_pubkey_t), &zlen);
  pubkey2native(key);

  struct tm *c;
  time_t t = (time_t)key->ctime;
  c = localtime(&t);

  fprintf(out, "%s\n", PCP_PUBKEY_HEADER);

  fprintf(out, "  Generated by: %s Version %d.%d.%d\n",
	  PCP_ME, PCP_VERSION_MAJOR, PCP_VERSION_MINOR, PCP_VERSION_PATCH);

  fprintf(out, "        Cipher: %s\n", PCP_KEY_PRIMITIVE);

  fprintf(out, "         Owner: %s\n", key->owner);
  fprintf(out, "          Mail: %s\n", key->mail);

  fprintf(out, "        Key-ID: 0x%s\n", key->id);
  fprintf(out, "    Public-Key: %s\n", pcp_z85_encode(key->public, 32, &zlen));

  //2004-06-14T23:34:30.
  fprintf(out, " Creation Time: %04d-%02d-%02dT%02d:%02d:%02d\n",
	 c->tm_year+1900, c->tm_mon+1, c->tm_mday,
	 c->tm_hour, c->tm_min, c->tm_sec);

  unsigned char *hash = pcppubkey_getchecksum(key);
  fprintf(out, "      Checksum: ");

  int i;
  for ( i = 0;i <15 ;++i) fprintf(out, "%02X:",(unsigned int) hash[i]);
  fprintf(out, "%02X", hash[15]);
  fprintf(out, "\n                ");
  for ( i = 16;i <31 ;++i) fprintf(out, "%02X:",(unsigned int) hash[i]);
  fprintf(out, "%02X", hash[31]);
  fprintf(out, "\n");
  fprintf(out, " Serial Number: 0x%08X\n", key->serial);
  fprintf(out, "   Key Version: 0x%08X\n", key->version);
  
  char *r = pcppubkey_get_art(key);
  fprintf(out, " Random Art ID: ");
  for (i=0; i<strlen(r); ++i) {
    if(r[i] == '\n') {
      fprintf(out, "\n                ");
    }
    else {
      fprintf(out, "%c", r[i]);
    }
  }
  fprintf(out, "\n");
  
  fprintf(out, "\n%s\n", z85encoded);

  fprintf(out, "%s\n", PCP_PUBKEY_FOOTER);

  free(hash);
  free(r);
  free(z85encoded);
}


void pcp_dumpkey(pcp_key_t *k) {
  int i;

  printf("Dumping pcp_key_t raw values:\n");
  printf("   public: ");
  for ( i = 0;i < 32;++i) printf("%02x",(unsigned int) k->public[i]);
  printf("\n");

  printf("   secret: ");
  for ( i = 0;i < 32;++i) printf("%02x",(unsigned int) k->secret[i]);
  printf("\n");

  printf("    nonce: ");
  for ( i = 0;i < 24;++i) printf("%02x",(unsigned int) k->nonce[i]);
  printf("\n");

  printf("encrypted: ");
  for ( i = 0;i < 48;++i) printf("%02x",(unsigned int) k->encrypted[i]);
  printf("\n");

  printf("    owner: %s\n", k->owner);

  printf("     mail: %s\n", k->mail);

  printf("       id: %s\n", k->id);

  printf("    ctime: %ld\n", k->ctime);

  printf("  version: 0x%08X\n", k->version);

  printf("   serial: 0x%08X\n", k->serial);

  printf("     type: 0x%02X\n", k->type);
}


void pcp_dumppubkey(pcp_pubkey_t *k) {
  int i;
  printf("Dumping pcp_pubkey_t raw values:\n");
  printf("   public: ");
  for ( i = 0;i < 32;++i) printf("%02x",(unsigned int) k->public[i]);
  printf("\n");

  printf("    owner: %s\n", k->owner);

  printf("     mail: %s\n", k->mail);

  printf("       id: %s\n", k->id);

  printf("    ctime: %ld\n", k->ctime);

  printf("  version: 0x%08X\n", k->version);

  printf("   serial: 0x%08X\n", k->serial);

  printf("     type: 0x%02X\n", k->type);
}

void pcpkey_printshortinfo(pcp_key_t *key) {
  int i;
  printf("        Key-ID: 0x%s\n", key->id);
  printf("         Owner: %s\n", key->owner);
  char *r = pcpkey_get_art(key);
  printf(" Random Art ID: ");
  for (i=0; i<strlen(r); ++i) {
    if(r[i] == '\n') {
      printf("\n                ");
    }
    else {
      printf("%c", r[i]);
    }
  }
  printf("\n");
  free(r);
}

void pcppubkey_printshortinfo(pcp_pubkey_t *key) {
  int i;
  printf("        Key-ID: 0x%s\n", key->id);
  printf("         Owner: %s\n", key->owner);
  char *r = pcppubkey_get_art(key);
  printf(" Random Art ID: ");
  for (i=0; i<strlen(r); ++i) {
    if(r[i] == '\n') {
      printf("\n                ");
    }
    else {
      printf("%c", r[i]);
    }
  }
  printf("\n");
  free(r);
}
