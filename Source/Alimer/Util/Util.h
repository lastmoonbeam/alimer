//
// Copyright (c) 2018 Amer Koleci and contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string>
#include <sstream>
#include <vector>
#include <type_traits>

#ifdef _MSC_VER
#	include <intrin.h>
#endif

namespace Util
{
#ifdef __GNUC__
#	define leading_zeroes(x) ((x) == 0 ? 32 : __builtin_clz(x))
#	define trailing_zeroes(x) ((x) == 0 ? 32 : __builtin_ctz(x))
#	define trailing_ones(x) __builtin_ctz(~(x))
#elif defined(_MSC_VER)
	namespace Internal
	{
		static inline uint32_t clz(uint32_t x)
		{
			unsigned long result;
			if (_BitScanReverse(&result, x))
				return 31 - result;
			else
				return 32;
		}

		static inline uint32_t ctz(uint32_t x)
		{
			unsigned long result;
			if (_BitScanForward(&result, x))
				return result;
			else
				return 32;
		}
	}

#define leading_zeroes(x) ::Util::Internal::clz(x)
#define trailing_zeroes(x) ::Util::Internal::ctz(x)
#define trailing_ones(x) ::Util::Internal::ctz(~(x))
#endif

	template<typename T>
	inline void ForEachBit(uint32_t value, const T &func)
	{
		while (value)
		{
			uint32_t bit = trailing_zeroes(value);
			func(bit);
			value &= ~(1u << bit);
		}
	}

	template<typename T>
	inline void ForEachBitRange(uint32_t value, const T &func)
	{
		while (value)
		{
			uint32_t bit = trailing_zeroes(value);
			uint32_t range = trailing_ones(value >> bit);
			func(bit, range);
			value &= ~((1u << (bit + range)) - 1);
		}
	}

	namespace inner
	{
		template<typename T>
		void join_helper(std::ostringstream &stream, T &&t)
		{
			stream << std::forward<T>(t);
		}

		template<typename T, typename... Ts>
		void join_helper(std::ostringstream &stream, T &&t, Ts &&... ts)
		{
			stream << std::forward<T>(t);
			join_helper(stream, std::forward<Ts>(ts)...);
		}
	}

	template<typename... Ts>
	std::string Join(Ts &&... ts)
	{
		std::ostringstream stream;
		inner::join_helper(stream, std::forward<Ts>(ts)...);
		return stream.str();
	}

	std::vector<std::string> Split(const std::string &str, const char *delim, bool allowEmpty = false);
	std::string Replace(const std::string& str, const std::string& find, const std::string& replace,
		uint32_t maxReplacements = std::numeric_limits<uint32_t>::max());
}
