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

#pragma once

/*******
*  prf_mac.cpp
*
*  Implementation of the HMAC's test vector verification.
*  Reference vectors are taken from RFC 4231 [https://tools.ietf.org/html/rfc4231]
*  Only the first four test cases are implemented: 
*  the HMAC-based PRF implementation does not support keys larger than 64 bytes
********/	

bool hmac_tests();
bool hmac_test_case_1();
bool hmac_test_case_2();
bool hmac_test_case_3();
bool hmac_test_case_4();