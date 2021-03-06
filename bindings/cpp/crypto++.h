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


#ifndef _HAVE_PCPPP_CRYPTO_H
#define _HAVE_PCPPP_CRYPTO_H

#include <pcp.h>
#include <string>
#include <iostream>

#include "vault++.h"
#include "key++.h"
#include "helpers++.h"

namespace pcp {

  class Crypto {
  private:
    PcpContext *PTX;
    bool havevault;

  public:
    PubKey P;
    Key S;
    Vault vault;

    // constructors
    Crypto(PcpContext *C, Key &skey, PubKey &pkey);
    Crypto(PcpContext *C, Vault &v, Key &skey, PubKey &pkey);

    // destructor
    ~Crypto();

    // PK encryption methods
    // sender pubkey is P
    bool encrypt(FILE *in, FILE *out, bool sign);

    // decrypt using P or use vault if defined
    bool decrypt(FILE *in, FILE *out, bool verify);
  };
};


#endif // _HAVE_PCPPP_CRYPTO_H
