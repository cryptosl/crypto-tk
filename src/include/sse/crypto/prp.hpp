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


#pragma once

#include <sse/crypto/key.hpp>

#include <cstdint>

#include <array>
#include <string>

namespace sse {
namespace crypto {

/// @class Prp
/// @brief Random permutation.
///
/// Prp is an opaque class implementing a length-preserving pseudorandom
/// permutation (PRP) for any input size. It uses the AEZ construction (by
/// Krovetz, Hoang and Rogaway). Because it relies on the reference
/// implementation, Prp requires support of AES-NI (on x86 CPUs) or of ARM NEON
/// instructions (or ARM CPUs). See the is_available static function to check
/// for availability.
///

class Prp
{
public:
    /// @internal
    friend void init_crypto_lib();

    /// @brief Prp key size (in bytes)
    static constexpr uint8_t kKeySize = 48;

    ///
    /// @brief Check availability of the Prp class
    ///
    /// Checks if the Prp class is available, i.e. that the code have been
    /// compiled with AES-NI or ARM NEON instructions enabled, and that these
    /// instructions are indeed available on the host CPU.
    ///
    /// @return true if the Prp class can be used, false otherwise.
    ///
    inline static bool is_available() noexcept
    {
        return is_available__;
    }

    ///
    /// @brief Constructor
    ///
    /// Creates a PRP with a new randomly generated key.
    ///
    /// @exception std::runtime_error The Prp class is not available.
    ///
    Prp();

    ///
    /// @brief Constructor
    ///
    /// Creates a PRP from a 48 bytes (384 bits) key.
    /// After a call to the constructor, the input key is
    /// held by the Prp object, and cannot be re-used.
    ///
    /// @param k    The key used to initialize the PRP.
    ///             Upon return, k is empty
    ///
    /// @exception std::runtime_error The Prp class is not available.
    ///
    explicit Prp(Key<kKeySize>&& k);

    ///
    /// @brief Destructor
    ///
    /// Destructs the Prp object and erase its key.
    ///
    ///
    ~Prp();

    // we should not be able to duplicate Fpe objects
    Prp(const Prp& c)  = delete;
    Prp(Prp& c)        = delete;
    Prp(const Prp&& c) = delete;
    Prp(Prp&& c)       = delete;

    ///
    /// @brief PRP evaluation
    ///
    /// Evaluates the pseudo random permutation on the input string.
    ///
    /// @param in    The input of the PRP.
    /// @param out   The evaluation of PRP(in).
    ///
    /// @exception std::runtime_error The Prp class is not available.
    ///
    void encrypt(const std::string& in, std::string& out);

    ///
    /// @brief PRP evaluation
    ///
    /// Evaluates the pseudo random permutation on the input string.
    ///
    /// @param in    The input of the PRP.
    /// @return      The evaluation of PRP(in).
    ///
    /// @exception std::runtime_error The Prp class is not available.
    ///
    std::string encrypt(const std::string& in);

    ///
    /// @brief PRP evaluation
    ///
    /// Evaluates the pseudo random permutation on the input 32 bits integer.
    ///
    /// @param in    The input of the PRP.
    /// @return      The evaluation of PRP(in).
    ///
    /// @exception std::runtime_error The Prp class is not available.
    ///
    uint32_t encrypt(const uint32_t in);

    ///
    /// @brief PRP evaluation
    ///
    /// Evaluates the pseudo random permutation on the input 64 bits integer.
    ///
    /// @param in    The input of the PRP.
    /// @return      The evaluation of PRP(in).
    ///
    /// @exception std::runtime_error The Prp class is not available.
    ///
    uint64_t encrypt_64(const uint64_t in);

    ///
    /// @brief PRP inversion
    ///
    /// Inverts the pseudo random permutation on the input string.
    ///
    /// @param in    The input for the PRP inversion.
    /// @param out   The evaluation of PRP^{-1}(in).
    ///
    /// @exception std::runtime_error The Prp class is not available.
    ///
    void decrypt(const std::string& in, std::string& out);

    ///
    /// @brief PRP inversion
    ///
    /// Inverts the pseudo random permutation on the input string.
    ///
    /// @param in   The input for the PRP inversion.
    /// @return     The evaluation of PRP^{-1}(in).
    ///
    /// @exception std::runtime_error The Prp class is not available.
    ///
    std::string decrypt(const std::string& in);
    ///
    /// @brief PRP inversion
    ///
    /// Inverts the pseudo random permutation on the input 32 bits integer.
    ///
    /// @param in   The input for the PRP inversion.
    /// @return     The evaluation of PRP^{-1}(in).
    ///
    /// @exception std::runtime_error The Prp class is not available.
    ///
    uint32_t decrypt(const uint32_t in);
    ///
    /// @brief PRP inversion
    ///
    /// Inverts the pseudo random permutation on the input 64 bits integer.
    ///
    /// @param in   The input for the PRP inversion.
    /// @return     The evaluation of PRP^{-1}(in).
    ///
    /// @exception std::runtime_error The Prp class is not available.
    ///
    uint64_t decrypt_64(const uint64_t in);

    // Again, avoid any assignement of Cipher objects
    Prp& operator=(const Prp& h) = delete;
    Prp& operator=(Prp& h) = delete;

private:
    class PrpImpl;     // not defined in the header
    PrpImpl* prp_imp_; // opaque pointer


    ///
    /// @brief Initialize the availability flag.
    ///
    /// Calls libsodium to determine if the aesni feature or the neon features
    /// are enabled on the host CPU. This function **must** be called before any
    /// use of the Prp class. Otherwise, the availability flag is set to false
    /// by default.
    ///
    ///
    static void compute_is_available() noexcept;

    static bool is_available__;
};


} // namespace crypto
} // namespace sse
