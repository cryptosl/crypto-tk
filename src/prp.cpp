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

#include "prp.hpp"

#include "random.hpp"

#if __AES__ || __ARM_FEATURE_CRYPTO
#include "aez/aez.h"
#endif

#include <climits>
#include <cstring>

#include <exception>
#include <iomanip>

#include <sodium/runtime.h>

namespace sse {

namespace crypto {

bool Prp::is_available__ = false;

#if __AES__ || __ARM_FEATURE_CRYPTO
class Prp::PrpImpl
{
public:
    PrpImpl();

    explicit PrpImpl(Key<kKeySize>&& k);

    void encrypt(const unsigned char* in,
                 const unsigned int&  len,
                 unsigned char*       out);
    void encrypt(const std::string& in, std::string& out);
    void decrypt(const unsigned char* in,
                 const unsigned int&  len,
                 unsigned char*       out);
    void decrypt(const std::string& in, std::string& out);

private:
    Key<sizeof(aez_ctx_t)> aez_ctx_;
};

#else
#warning PRP is not available without CPU support for AES instructions

class Prp::PrpImpl
{
public:
    PrpImpl(){};

    explicit PrpImpl(Key<kKeySize>&& k){};

    void encrypt(const unsigned char* in,
                 const unsigned int&  len,
                 unsigned char*       out){};
    void encrypt(const std::string& in, std::string& out){};
    void decrypt(const unsigned char* in,
                 const unsigned int&  len,
                 unsigned char*       out){};
    void decrypt(const std::string& in, std::string& out){};
};
#endif /* __AES__ || __ARM_FEATURE_CRYPTO */

void Prp::compute_is_available() noexcept
{
#if __AES__ || __ARM_FEATURE_CRYPTO
    is_available__
        = (sodium_runtime_has_aesni() == 1) || (sodium_runtime_has_neon() == 1);
#else
    is_available__ = false;
#endif
}

Prp::Prp() : prp_imp_(Prp::is_available() ? new PrpImpl() : nullptr)
{
    if (!Prp::is_available()) {
        throw std::runtime_error("PRP is unavailable: AES hardware "
                                 "acceleration not supported by the CPU");
    }
}


Prp::Prp(Key<kKeySize>&& k)
    : prp_imp_(Prp::is_available() ? new PrpImpl(std::move(k)) : nullptr)
{
    if (!Prp::is_available()) {
        throw std::runtime_error("PRP are unavailable: AES hardware "
                                 "acceleration not supported by the CPU");
    }
}


Prp::~Prp()
{
    delete prp_imp_;
}

void Prp::encrypt(const std::string& in, std::string& out)
{
    prp_imp_->encrypt(in, out);
}

std::string Prp::encrypt(const std::string& in)
{
    std::string out;
    prp_imp_->encrypt(in, out);
    return out;
}

uint32_t Prp::encrypt(const uint32_t in)
{
    uint32_t out;
    prp_imp_->encrypt(reinterpret_cast<const unsigned char*>(&in),
                      sizeof(uint32_t),
                      reinterpret_cast<unsigned char*>(&out));
    return out;
}

uint64_t Prp::encrypt_64(const uint64_t in)
{
    uint64_t out;
    prp_imp_->encrypt(reinterpret_cast<const unsigned char*>(&in),
                      sizeof(uint64_t),
                      reinterpret_cast<unsigned char*>(&out));
    return out;
}


void Prp::decrypt(const std::string& in, std::string& out)
{
    prp_imp_->decrypt(in, out);
}

std::string Prp::decrypt(const std::string& in)
{
    std::string out;
    prp_imp_->decrypt(in, out);
    return out;
}

uint32_t Prp::decrypt(const uint32_t in)
{
    uint32_t out;
    prp_imp_->decrypt(reinterpret_cast<const unsigned char*>(&in),
                      sizeof(uint32_t),
                      reinterpret_cast<unsigned char*>(&out));
    return out;
}

uint64_t Prp::decrypt_64(const uint64_t in)
{
    uint64_t out;
    prp_imp_->decrypt(reinterpret_cast<const unsigned char*>(&in),
                      sizeof(uint64_t),
                      reinterpret_cast<unsigned char*>(&out));
    return out;
}

#if __AES__ || __ARM_FEATURE_CRYPTO

Prp::PrpImpl::PrpImpl()
{
    auto callback = [](uint8_t* key_content) {
        Key<kKeySize> r_key;
        aez_setup(static_cast<const unsigned char*>(r_key.unlock_get()),
                  48,
                  reinterpret_cast<aez_ctx_t*>(key_content));
    };

    aez_ctx_ = Key<sizeof(aez_ctx_t)>(callback);
}

Prp::PrpImpl::PrpImpl(Key<kKeySize>&& k)
{
    auto callback = [&k](uint8_t* key_content) {
        aez_setup(static_cast<const unsigned char*>(k.unlock_get()),
                  48,
                  reinterpret_cast<aez_ctx_t*>(key_content));
    };

    aez_ctx_ = Key<sizeof(aez_ctx_t)>(callback);
    k.erase();
}

void Prp::PrpImpl::encrypt(const unsigned char* in,
                           const unsigned int&  len,
                           unsigned char*       out)
{
    if (!Prp::is_available()) {
        throw std::runtime_error("PRP is unavailable: AES hardware "
                                 "acceleration not supported by the CPU");
    }
    char iv[16] = {0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00};
    aez_encrypt(reinterpret_cast<const aez_ctx_t*>(aez_ctx_.unlock_get()),
                iv,
                16,
                0,
                reinterpret_cast<const char*>(in),
                len,
                reinterpret_cast<char*>(out));
}

void Prp::PrpImpl::encrypt(const std::string& in, std::string& out)
{
    if (!Prp::is_available()) {
        throw std::runtime_error("PRP is unavailable: AES hardware "
                                 "acceleration not supported by the CPU");
    }

    size_t len = in.size();

    if (len > UINT_MAX) {
        throw std::runtime_error(
            "The maximum input length of Format Preserving Encryption is "
            "UINT_MAX"); /* LCOV_EXCL_LINE */
    }

    unsigned char* data = new unsigned char[len];

    encrypt(reinterpret_cast<const unsigned char*>(in.data()),
            static_cast<unsigned int>(len),
            data);
    out = std::string(reinterpret_cast<char*>(data), len);
    delete[] data;
}

void Prp::PrpImpl::decrypt(const unsigned char* in,
                           const unsigned int&  len,
                           unsigned char*       out)
{
    if (!Prp::is_available()) {
        throw std::runtime_error("PRP is unavailable: AES hardware "
                                 "acceleration not supported by the CPU");
    }

    char iv[16] = {0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00,
                   0x00};
    aez_decrypt(reinterpret_cast<const aez_ctx_t*>(aez_ctx_.unlock_get()),
                iv,
                16,
                0,
                reinterpret_cast<const char*>(in),
                len,
                reinterpret_cast<char*>(out));

    aez_ctx_.lock();
}

void Prp::PrpImpl::decrypt(const std::string& in, std::string& out)
{
    if (!Prp::is_available()) {
        throw std::runtime_error("PRP is unavailable: AES hardware "
                                 "acceleration not supported by the CPU");
    }

    size_t len = in.size();

    if (len > UINT_MAX) {
        throw std::runtime_error(
            "The maximum input length of Format Preserving Encryption is "
            "UINT_MAX"); /* LCOV_EXCL_LINE */
    }

    unsigned char* data = new unsigned char[len];

    decrypt(reinterpret_cast<const unsigned char*>(in.data()),
            static_cast<unsigned int>(len),
            data);

    out = std::string(reinterpret_cast<const char*>(data), len);
    delete[] data;
}

#endif /* __AES__ || __ARM_FEATURE_CRYPTO */

} // namespace crypto
} // namespace sse
