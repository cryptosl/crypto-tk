//
// libsse_crypto - An abstraction layer for high level cryptographic features.
// Copyright (C) 2015 Raphael Bost
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

#include "hash.hpp"

#include "hash/sha512.hpp"
#include "hash/blake2b/blake2b.hpp"

#include <cassert>
#include <cstring>

namespace sse
{
	
namespace crypto
{
	
using hash_function = hash::sha512;
	
void Hash::hash(const unsigned char *in, const size_t &len, unsigned char *out)
{
	// memset(out,0x00, kDigestSize);
	static_assert(kDigestSize == hash_function::kDigestSize, "Declared digest size and hash_function digest size do not match");
	static_assert(kBlockSize == hash_function::kBlockSize, "Declared block size and hash_function block size do not match");
	hash_function::hash(in, len, out);
}

void Hash::hash(const unsigned char *in, const size_t &len, const size_t &out_len, unsigned char *out)
{
	assert(out_len <= kDigestSize);
	unsigned char digest[kDigestSize];

	hash(in, len, digest);
	memcpy(out, digest, out_len);
}

void Hash::hash(const std::string &in, std::string &out)
{
	out.resize(kDigestSize);
	hash((unsigned char*)in.data(),in.length(),(unsigned char*)out.data());
}

void Hash::hash(const std::string &in, const size_t &out_len, std::string &out)
{
	out.resize(out_len);
	hash((unsigned char*)in.data(), in.length(), out_len, (unsigned char*)out.data());
}

std::string Hash::hash(const std::string &in)
{
	std::string out;
	hash(in,out);
	return out;
}

std::string Hash::hash(const std::string &in, const size_t &out_len)
{
	std::string out;
	hash(in,out_len,out);
	return out;
}

}
}