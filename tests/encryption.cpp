//
// libsse_crypto - An abstraction layer for high level cryptographic features.
// Copyright (C) 2015-2017 Raphael Bost
//
// This file is part of libsse_crypto.
//
// libsse_crypto is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// libsse_crypto is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with libsse_crypto.  If not, see <http://www.gnu.org/licenses/>.
//


/*******
 *  encryption.cpp
 *
 *  Check that encryption is correctly inverted by decryption.
 *
 ********/

#include <sse/crypto/cipher.hpp>

#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

#include "gtest/gtest.h"

constexpr size_t kCipherKeySize = sse::crypto::Cipher::kKeySize;

TEST(encryption, correctness)
{
    string in_enc = "This is a test input.";
    string out_enc, out_dec;

    array<uint8_t, kCipherKeySize> k;
    k.fill(0x00);

    sse::crypto::Cipher cipher(sse::crypto::Key<kCipherKeySize>(k.data()));
    cipher.encrypt(in_enc, out_enc);

    string in_dec = string(out_enc);

    cipher.decrypt(in_dec, out_dec);

    ASSERT_EQ(in_enc, out_dec);
}


TEST(encryption, exception)
{
    ASSERT_EQ(sse::crypto::Cipher::plaintext_length(0), 0);
    ASSERT_EQ(sse::crypto::Cipher::plaintext_length(
                  sse::crypto::Cipher::ciphertext_length(10)),
              10);

    string in_enc = "";
    string out_enc, out_dec;

    array<uint8_t, sse::crypto::Cipher::kKeySize> k;
    k.fill(0x00);

    sse::crypto::Cipher cipher(sse::crypto::Key<kCipherKeySize>(k.data()));

    ASSERT_THROW(cipher.encrypt(in_enc, out_enc), std::invalid_argument);

    string in_dec = string(3, 'a');

    ASSERT_THROW(cipher.decrypt(in_dec, out_dec), std::invalid_argument);

    in_dec = string(300, 'a'); // long enough to be a 'valid' ciphertext
    ASSERT_THROW(cipher.decrypt(in_dec, out_dec), std::runtime_error);
}
