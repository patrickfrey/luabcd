/*
  Copyright (c) 2020 Patrick P. Frey

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
///\file bcd.cpp
///\brief Implements some operations on arbitrary sized packed bcd numbers

#include "bcd.hpp"
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <limits>
#include <cmath>
#include <algorithm>

#define NumMask 0x0fffFFFFffffFFFFULL
#define NumHighShift 60
#define NumDigits 15
#define MaxEstimate 100000000000000ULL

#define long_DIGITS 20

using namespace bcd;

void BigInt::swap( BigInt& o) noexcept
{
	std::swap( m_ar, o.m_ar);
	std::swap( m_size, o.m_size);
	std::swap( m_sign, o.m_sign);
	std::swap( m_allocated, o.m_allocated);
}

void BigInt::allocate( std::size_t nn)
{
	if (m_ar && m_allocated) free( m_ar);
	m_size = nn;
	if (m_size)
	{
		std::size_t mm = nn * sizeof(*m_ar);
		if (mm < nn) throw std::bad_alloc();
		m_ar = (Element*)std::malloc( mm);
		if (!m_ar) throw std::bad_alloc();
		std::memset( m_ar, 0, mm);
		m_allocated = true;
	}
	else
	{
		m_ar = nullptr;
		m_allocated = false;
	}
	m_sign = false;
}

BigInt::BigInt() noexcept
	:m_size(0)
	,m_ar(0)
	,m_sign(false)
	,m_allocated(false)
{}

void BigInt::init()
{
	m_size = 0;
	m_ar = nullptr;
	m_sign = false;
	m_allocated = false;
}

BigNumber::~BigNumber()
{
	if (m_ar) std::free( m_ar);
}

BigNumber::BigNumber( const char* val, std::size_t valsize)
{
	m_scale = 0;
	m_sign = false;
	m_size = 0;
	m_ar = nullptr;

	std::size_t vi = 0, ve = valsize;
	std::size_t leadingZeros = 0;
	for (; vi != ve && val[vi] == '0'; ++vi,++leadingZeros){}
	valsize -= leadingZeros;
	if (!valsize) return;
	if (valsize > std::numeric_limits<unsigned short>::max()) throw std::bad_alloc();

	m_ar = (unsigned char*)std::calloc( valsize, 1);
	if (!m_ar) throw std::bad_alloc();

	enum State {NUMS,NUM0,NUM1,FRC0,FRC1,EXPE,EXPS,EXP0,EXP1}; //< parsing states
	State state = NUMS;

	short scaleinc = 0;
	short prev_scaleinc = 0;
	bool expsign = false;

	for (; vi != ve; ++vi)
	{
		switch (state)
		{
			case NUMS://sign of number:
				state = NUM0;
				if (val[vi] == '-')
				{
					m_sign = true;
					continue;
				}
				if (val[vi] == '+')
				{
					m_sign = false;
					continue;
				}
				//...no break here !

			case NUM0://leading zeros:
				if (val[vi] == '0') continue;
				state = NUM1;
				//...no break here !

			case NUM1://significant pre-decimal digits:
				if (val[vi] >= '0' && val[vi] <= '9')
				{
					m_ar[ m_size++] = val[vi] - '0';
					continue;
				}
				if (val[vi] == '.')
				{
					if (m_size)
					{
						state = FRC1;
						//.. there are already significant digits, so all following are significant too
					}
					else
					{
						state = FRC0;
						//.. no significant digits found yet, so leading zeros of the fractional part are not significant either
					}
					continue;
				}
				state = EXPE;
				continue;

			case FRC0://leading zeros of fractional part when no significant digits found yet (only influencing scale):
				if (val[vi] == '0')
				{
					m_scale++;
					continue;
				}
				state = FRC1;
				//...no break here !

			case FRC1://significant digits of fractional part:
				if (val[vi] >= '0' && val[vi] <= '9')
				{
					m_ar[ m_size++] = val[vi] - '0';
					m_scale++;
					continue;
				}
				state = EXPE;
				//...no break here !

			case EXPE://exponent marker:
				if (val[vi] == ' ') continue;
				if (val[vi] == 'E')
				{
					state = EXPS;
					continue;
				}
				throw std::runtime_error("syntax error in big number string");

			case EXPS://exponent sign:
				state = EXP0;
				if (val[vi] == '-')
				{
					expsign = true;
					continue;
				}
				if (val[vi] == '+')
				{
					expsign = false;
					continue;
				}
				//...no break here !

			case EXP0://leading zeros of the exponent (ignored):
				if (val[vi] == '0') continue;
				state = EXP1;
				//...no break here !

			case EXP1://exponent:
				if (val[vi] >= '0' && val[vi] <= '9')
				{
					scaleinc = scaleinc * 10 + val[vi] - '0';
					if (prev_scaleinc > scaleinc) throw std::runtime_error("conversion error: big number value in string is out of range");
					prev_scaleinc = scaleinc;
					continue;
				}
				throw std::runtime_error("syntax error in big number string");
		}
	}
	if (expsign)
	{
		short prev_scale = m_scale;
		m_scale += scaleinc;
		if (prev_scale > m_scale) throw std::runtime_error("conversion error: big number value in string is out of range");
	}
	else
	{
		short prev_scale = m_scale;
		m_scale -= scaleinc;
		if (prev_scale < m_scale) throw std::runtime_error("conversion error: big number value in string is out of range");
	}
	if (m_size == 0)
	{
		m_scale = 0;
		m_sign = 0;
	}
	unsigned char* ar_ = (unsigned char*)std::realloc( m_ar, m_size);
	if (ar_) m_ar = ar_;
}

void BigInt::init( const BigNumber& num)
{
	init();
	static const unsigned char digits_zero[1] = {0};

	unsigned int ii = 0, nn = num.size(), nofDigits = num.size();
	const unsigned char* digits = num.digits();
	if (nn == 0)
	{
		digits = digits_zero;
		nofDigits = nn = 1;
	}
	if (num.scale() > 0)
	{
		if ((unsigned int)num.scale() > nn)
		{
			digits = digits_zero;
			nn = nofDigits = 1;
		}
		else
		{
			nn -= (unsigned int)num.scale();
			nofDigits = nn;
		}
	}
	else
	{
		nn += (unsigned int)-num.scale();
	}
	unsigned int bb = ((nn+NumDigits-1) / NumDigits);
	unsigned int tt = ((nn+NumDigits-1) % NumDigits) * 4;

	allocate( bb);
	for (; ii<nn; ++ii)
	{
		Element digit;
		if (ii > nofDigits)
		{
			digit = 0;
		}
		else
		{
			digit = digits[ ii];
			if (digit > 9) throw std::runtime_error( "illegal bcd number");
		}
		m_ar[ bb-1] += (digit << tt);

		if (tt == 0)
		{
			--bb;
			if (!bb && (ii+1) != nn) throw std::runtime_error( "illegal state in bcd number constructor");
			tt = (NumHighShift-4);
		}
		else
		{
			tt -= 4;
		}
	}
	m_sign = num.sign();
	normalize();
}

void BigInt::init( const std::string& str)
{
	BigNumber num( str.c_str(), str.size());
	init( num);
}

void BigInt::init( const char* str, std::size_t strsize)
{
	BigNumber num( str, strsize);
	init( num);
}

void BigInt::init( long num)
{
	bool ng = false;
	if (num < 0)
	{
		ng = true;
		num = -num;
	}
	BigInt th = estimate_as_bcd( (FactorType)num, 0);
	th.m_sign ^= ng;
	copy( th);
}

void BigInt::init( unsigned long num)
{
	BigInt th = estimate_as_bcd( (FactorType)num, 0);
	copy( th);
}

void BigInt::init( double num)
{
	BigInt th = estimate_as_bcd( (FactorType)(num+0.5-std::numeric_limits<double>::epsilon()), 0);
	copy( th);
}

BigInt::BigInt( const std::string& numstr)
	:m_size(0)
	,m_ar(0)
	,m_sign(false)
	,m_allocated(false)
{
	init( numstr);
}

BigInt::BigInt( const char* numstr, std::size_t numlen)
	:m_size(0)
	,m_ar(0)
	,m_sign(false)
	,m_allocated(false)
{
	init( numstr, numlen);
}

BigInt::BigInt( const BigNumber& num)
	:m_size(0)
	,m_ar(0)
	,m_sign(false)
	,m_allocated(false)
{
	BigInt::init( num);
}

BigInt::BigInt( long num)
	:m_size(0)
	,m_ar(0)
	,m_sign(false)
	,m_allocated(false)
{
	BigInt::init( num);
}

BigInt::BigInt( unsigned long num)
	:m_size(0)
	,m_ar(0)
	,m_sign(false)
	,m_allocated(false)
{
	BigInt::init( num);
}

BigInt::BigInt( double num)
	:m_size(0)
	,m_ar(0)
	,m_sign(false)
	,m_allocated(false)
{
	BigInt::init( num);
}

BigInt::BigInt( const BigInt& o)
	:m_size(o.m_size)
	,m_ar(0)
	,m_sign(o.m_sign)
	,m_allocated(false)
{
	allocate( m_size);
	m_sign = o.m_sign;
	std::memcpy( m_ar, o.m_ar, m_size * sizeof(*m_ar));
}

void BigInt::copy( const BigInt& o)
{
	allocate( o.m_size);
	m_sign = o.m_sign;
	std::memcpy( m_ar, o.m_ar, m_size * sizeof(*m_ar));
}

BigInt::~BigInt()
{
	if (m_ar && m_allocated) free( m_ar);
}

std::string BigInt::tostring() const
{
	std::string rt;
	const_iterator ii = begin(), ee = end();
	if (ii == ee) return "0";

	if (m_sign)
	{
		rt.push_back('-');
	}
	for (; ii != ee; ++ii)
	{
		rt.push_back( ii->ascii());
	}
	return rt;
}

long BigInt::toint() const
{
	long rt = 0;
	int cnt = 0;

	const_iterator ii = begin(), ee = end();
	if (ii == ee) return 0;

	for (; cnt < long_DIGITS && ii != ee; ++ii,++cnt)
	{
		rt = rt * 10 + *ii;
	}
	if (m_sign)
	{
		rt = -rt;
	}
	if (ii != ee)
	{
		throw std::runtime_error("number out of range to convert it to a long integer");
	}
	return rt;
}

double BigInt::todouble() const
{
	double rt = 0.0;

	const_iterator ii = begin(), ee = end();
	if (ii == ee) return 0;

	for (; ii != ee; ++ii)
	{
		rt = rt * 10 + *ii;
	}
	if (m_sign)
	{
		rt = -rt;
	}
	return rt;
}

BigInt::const_iterator::const_iterator() noexcept
	:m_idx(0),m_shf(NumHighShift-4),m_ar(0)
{}

BigInt::const_iterator::const_iterator( const BigInt& bcd) noexcept
	:m_idx(bcd.m_size),m_shf(NumHighShift-4),m_ar(bcd.m_ar)
{
	while (m_idx>0)
	{
		unsigned char digit = (unsigned char)((m_ar[ m_idx-1] >> m_shf) & 0xf);
		if (digit != 0) break;
		increment();
	}
}

std::size_t BigInt::const_iterator::size() const noexcept
{
	return (m_idx == 0)?0:((m_idx-1)*NumDigits + m_shf/4 + 1);
}

void BigInt::const_iterator::increment() noexcept
{
	if (m_shf == 0)
	{
		m_shf = NumHighShift-4;
		m_idx -= 1;
	}
	else
	{
		m_shf -= 4;
	}
}

unsigned char BigInt::const_iterator::operator*() const
{
	unsigned char rt = (unsigned char)(m_ar[ m_idx-1] >> m_shf) & 0xf;
	if (rt > 9) throw std::runtime_error( "corrupt bcd number");
	return rt;
}

bool BigInt::const_iterator::isequal( const const_iterator& other) const noexcept
{
	return m_idx==other.m_idx && m_shf==other.m_shf;
}

bool BigInt::const_iterator::islt( const const_iterator& other) const noexcept
{
	return m_idx>other.m_idx || m_shf>other.m_shf;
}

bool BigInt::const_iterator::isle( const const_iterator& other) const noexcept
{
	return m_idx>=other.m_idx || m_shf>=other.m_shf;
}

static BigInt::Element checkvalue( std::uint64_t a) noexcept
{
	// thanks to http://www.divms.uiowa.edu/~jones/bcd/bcd.html:
	std::uint64_t t1,t2;
	t1 = a + 0x0666666666666666ULL;
	t2 = t1 ^ a;
	return (t2 & 0x1111111111111110ULL);
}

static std::uint64_t add_bcd( std::uint64_t a, std::uint64_t b) noexcept
{
	// thanks to http://www.divms.uiowa.edu/~jones/bcd/bcd.html:
	std::uint64_t t1,t2,t3,t4,t5,t6;
	t1 = a + 0x0666666666666666ULL;
	t2 = t1 + b;
	t3 = t1 ^ b;
	t4 = t2 ^ t3;
	t5 = ~t4 & 0x1111111111111110ULL;
	t6 = (t5 >> 2) | (t5 >> 3);
	return t2 - t6;
}

static std::uint64_t tencomp( std::uint64_t a) noexcept
{
	// thanks to http://www.divms.uiowa.edu/~jones/bcd/bcd.html:
	std::uint64_t t1,t2,t3,t4,t5,t6;
	t1 = 0xffffFFFFffffFFFFULL - a;
	t2 = (std::uint64_t) (- (std::int64_t)a);
	t3 = t1 ^  0x0000000000000001ULL;
	t4 = t2 ^ t3;
	t5 = ~t4 & 0x1111111111111110ULL;
	t6 = (t5 >> 2) | (t5 >> 3);
	return t2 - t6;
}

static std::uint64_t getcarry( std::uint64_t& a) noexcept
{
	std::uint64_t carry = (a >> 60);
	a &= 0x0fffFFFFffffFFFFULL;
	return carry;
}

static BigInt::Element sub_bcd( BigInt::Element a, BigInt::Element b)
{
	BigInt::Element rt = add_bcd( a, tencomp(b));
	return rt;
}

static BigInt::Element increment( BigInt::Element a)
{
	return add_bcd( a, 1);
}

static BigInt::Element decrement( BigInt::Element a)
{
	return sub_bcd( a, 1);
}

bool BigInt::isValid() const noexcept
{
	std::size_t ii;
	Element chkval = 0;
	for (ii=0; ii<m_size; ++ii)
	{
		chkval |= checkvalue( m_ar[ii]);
	}
	return (chkval == 0);
}

bool BigInt::isNull() const noexcept
{
	const_iterator ii=begin(),ee=end();
	return (ii==ee);
}

void BigInt::normalize()
{
	if (!isValid()) throw std::logic_error( "bad bcd calculation");
	std::size_t ii = 0, nn = m_size;

	for (ii=nn; ii>0; --ii)
	{
		if (m_ar[ii-1]) break;
	}
	if (ii > 0)
	{
		m_size = ii;
	}
	else
	{
		m_sign = false;
		m_size = 0;
	}
}

void BigInt::digits_addition( BigInt& rt, const BigInt& this_, const BigInt& opr)
{
	Element carry;
	std::size_t ii=0, nn = (opr.m_size > this_.m_size)?opr.m_size:this_.m_size;
	if (nn == 0) return;
	rt.allocate( nn+1);
	rt.m_sign = this_.m_sign;
	carry = 0;
	for (;ii<nn; ++ii)
	{
		Element op1 = (ii>=this_.m_size)?0:this_.m_ar[ii];
		Element op2 = (ii>=opr.m_size)?0:opr.m_ar[ii];
		Element res = add_bcd( op1, op2);
		if (carry) res = increment( res);
		carry = getcarry( res);
		rt.m_ar[ ii] = res;
	}
	if (carry)
	{
		rt.m_ar[ nn++] = carry;
		if (!rt.isValid()) throw std::logic_error( "bad bcd calculation");
	}
	else
	{
		rt.normalize();
	}
}

void BigInt::digits_subtraction( BigInt& rt, const BigInt& this_, const BigInt& opr)
{
	std::size_t ii = 0, mm = 0, nn = (opr.m_size > this_.m_size)?opr.m_size:this_.m_size;
	if (nn == 0) return;
	rt.allocate( nn);
	rt.m_sign = this_.m_sign;
	Element carry = 0;
	for (;ii<nn; ++ii)
	{
		Element op1 = (ii>=this_.m_size)?0:this_.m_ar[ii];
		Element op2 = (ii>=opr.m_size)?0:opr.m_ar[ii];
		Element res = add_bcd( op1, tencomp(op2));
		if (carry)
		{
			res = decrement( res);
			carry = (op1 <= op2);
		}
		else
		{
			carry = (op1 < op2);
		}
		rt.m_ar[ ii] = res;
	}
	if (carry)
	{
		for (mm=nn; mm>0; mm--)
		{
			Element res = rt.m_ar[ mm-1];
			if (mm>1) res = increment( res);
			res = tencomp(res) & NumMask;
			rt.m_ar[ mm-1] = res;
		}
		rt.m_sign = !rt.m_sign;
	}
	else
	{
		for (mm=nn; mm>0; mm--) rt.m_ar[ mm-1] &= NumMask;
	}
	rt.normalize();
}

void BigInt::digits_shift( BigInt& rt, const BigInt& this_, int nof_digits)
{
	if (nof_digits > 0)
	{
		unsigned int ofs = (unsigned int)nof_digits / NumDigits;
		unsigned int sfh = (unsigned int)nof_digits % NumDigits;
		std::size_t ii,nn;

		rt.allocate( this_.m_size + ofs + 1);
		rt.m_sign = this_.m_sign;
		for (ii=0,nn=ofs; ii<nn; ++ii)
		{
			rt.m_ar[ ii] = 0;
		}
		if (sfh == 0)
		{
			for (ii=0,nn=this_.m_size; ii<nn; ++ii)
			{
				rt.m_ar[ ii+ofs] = this_.m_ar[ ii];
			}
		}
		else if (this_.m_size)
		{
			unsigned char upshift=NumHighShift-(sfh*4),doshift=sfh*4;
			rt.m_ar[ ofs++] = (this_.m_ar[ 0] << doshift) & NumMask;
			for (ii=0,nn=this_.m_size-1; ii<nn; ++ii)
			{
				Element aa = this_.m_ar[ ii] >> upshift;
				Element bb = (this_.m_ar[ ii+1] << doshift) & NumMask;
				rt.m_ar[ ii + ofs] = aa | bb;
			}
			rt.m_ar[ ii + ofs] = this_.m_ar[ ii] >> upshift;
		}
	}
	else if (nof_digits < 0)
	{
		nof_digits = -nof_digits;
		unsigned int ofs = (unsigned int)nof_digits / NumDigits;
		unsigned int sfh = (unsigned int)nof_digits % NumDigits;
		std::size_t ii,nn;

		rt.allocate( this_.m_size - ofs + 1);
		rt.m_sign = this_.m_sign;
		if (sfh == 0)
		{
			for (ii=ofs,nn=this_.m_size; ii<nn; ++ii)
			{
				rt.m_ar[ ii-ofs] = this_.m_ar[ ii];
			}
		}
		else if (this_.m_size)
		{
			unsigned char upshift=NumHighShift-(sfh*4),doshift=sfh*4;
			for (ii=ofs,nn=this_.m_size-1; ii<nn; ++ii)
			{
				Element aa = this_.m_ar[ ii] >> doshift;
				Element bb = (this_.m_ar[ ii+1] << upshift) & NumMask;
				rt.m_ar[ ii - ofs] = aa | bb;
			}
			rt.m_ar[ ii - ofs] = this_.m_ar[ ii] >> doshift;
		}
	}
	else
	{
		rt.copy( this_);
	}
	rt.normalize();
}

void BigInt::digits_cut( BigInt& rt, const BigInt& this_, unsigned int nof_digits)
{
	unsigned int ofs = (unsigned int)nof_digits / NumDigits;
	unsigned char sfh = (unsigned char)nof_digits % NumDigits;
	std::size_t ii,nn;

	rt.allocate( ofs + 1);
	rt.m_sign = this_.m_sign;
	for (ii=0,nn=ofs; ii<nn; ++ii)
	{
		rt.m_ar[ ii] = this_.m_ar[ ii];
	}
	unsigned int mask = NumMask >> ((NumDigits - sfh) * 4);
	rt.m_ar[ ii] = this_.m_ar[ ii] & mask;
	rt.normalize();
}

BigInt BigInt::shift( int digits) const
{
	BigInt rt;
	digits_shift( rt, *this, digits);
	return rt;
}

BigInt BigInt::cut( unsigned int digits) const
{
	BigInt rt;
	digits_cut( rt, *this, digits);
	return rt;
}

BigInt BigInt::round( const BigInt& gran) const
{
	unsigned int nn = gran.nof_digits();
	if (gran.m_sign || !nn) throw std::runtime_error( "rounding granularity must be a positive number");

	BigInt aa;
	aa.copy( *this);
	BigInt ct;
	digits_cut( ct, aa, nn);
	ct.m_sign = false;

	unsigned int ft = 0;
	BigInt zt;
	for (;;)
	{
		++ft;
		digits_subtraction( zt, ct, gran);
		if (zt.m_sign) break;
		ct.copy( zt);
	}
	if (m_sign)
	{
		return *this + zt;
	}
	else
	{
		return *this - zt;
	}
}

void BigInt::digits_16_multiplication( BigInt& rt, const BigInt& this_)
{
	BigInt x2,x4,x8;
	digits_addition( x2, this_, this_);
	digits_addition( x4, x2, x2);
	digits_addition( x8, x4, x4);
	digits_addition( rt, x8, x8);
}

void BigInt::digits_nibble_multiplication( BigInt& rt, const BigInt& this_, unsigned char factor)
{
	BigInt x2,x4,x8;
	if ((factor & 0xE) != 0)
	{
		digits_addition( x2, this_, this_);
		if ((factor & 0xC) != 0)
		{
			digits_addition( x4, x2, x2);
			if ((factor & 0x8) != 0)
			{
				digits_addition( x8, x4, x4);
			}
		}
	}
	switch (factor)
	{
		case 0:
		break;
		case 1:
			rt.copy( this_);
		break;
		case 2:
			rt.copy( x2);
		break;
		case 3:
			digits_addition( rt, x2, this_);
		break;
		case 4:
			rt.copy( x4);
		break;
		case 5:
			digits_addition( rt, x4, this_);
		break;
		case 6:
			digits_addition( rt, x4, x2);
		break;
		case 7:
		{
			BigInt x6;
			digits_addition( x6, x4, x2);
			digits_addition( rt, x6, this_);
		}
		break;
		case 8:
			rt.copy( x8);
		break;
		case 9:
			digits_addition( rt, x8, this_);
		break;
		case 10:
			digits_addition( rt, x8, x2);
		break;
		case 11:
		{
			BigInt x10;
			digits_addition( x10, x8, x2);
			digits_addition( rt, x10, this_);
		}
		break;
		case 12:
			digits_addition( rt, x8, x4);
		break;
		case 13:
		{
			BigInt x12;
			digits_addition( x12, x8, x4);
			digits_addition( rt, x12, this_);
		}
		break;
		case 14:
		{
			BigInt x12;
			digits_addition( x12, x8, x4);
			digits_addition( rt, x12, x2);
		}
		break;
		case 15:
		{
			BigInt x12,x14;
			digits_addition( x12, x8, x4);
			digits_addition( x14, x12, x2);
			digits_addition( rt, x14, this_);
		}
		break;
		default:
			throw std::logic_error( "multiplication nibble out of range");
	}
}

void BigInt::digits_multiplication( BigInt& rt, const BigInt& this_, FactorType factor)
{
	if (factor == 0)
	{
		rt.allocate( 0);
		return;
	}
	BigInt part,fac;
	digits_nibble_multiplication( rt, this_, factor & 0x0f);
	fac.copy( this_);
	factor >>= 4;
	while (factor > 0)
	{
		BigInt newfac;
		digits_16_multiplication( newfac, fac);
		fac.swap( newfac);
		digits_nibble_multiplication( part, fac, factor & 0x0f);
		BigInt sum;
		digits_addition( sum, part, rt);
		sum.swap( rt);
		factor >>= 4;
	}
}

void BigInt::digits_multiplication( BigInt& rt, const BigInt& this_, const BigInt& opr)
{
	const_iterator ii = opr.begin(), ee = opr.end();
	if (ii == ee) return;

	digits_nibble_multiplication( rt, this_, *ii);
	++ii;
	while (ii != ee)
	{
		BigInt sum;
		digits_shift( sum, rt, 1);
		BigInt part;
		digits_nibble_multiplication( part, this_, *ii);
		digits_addition( rt, sum, part);
		++ii;
	}
}

static int estimate_shifts( const BigInt& this_, const BigInt& match)
{
	int rt = (int)(this_.nof_digits() - match.nof_digits());
	if (*this_.begin() == *match.begin())
	{
		return rt;
	}
	else
	{
		return rt - 1;
	}
}

void BigInt::digits_division( BigInt& result, BigInt& remainder, const BigInt& this_, const BigInt& opr)
{
	remainder.copy( this_);
	remainder.m_sign = false;

	if (opr.isNull()) throw std::runtime_error( "division by zero");

	while (!remainder.isNull() && remainder.compare( opr) >= 0)
	{
		FactorType estimate = division_estimate( remainder, opr);
		if (estimate == 0) throw std::runtime_error( "illegal state calculating division estimate");
		BigInt part;
		digits_multiplication( part, opr, estimate);
		part.m_sign = false;
		int estshift = estimate_shifts( remainder, part);
		BigInt corr;
		digits_shift( corr, part, estshift);

		while (remainder < corr)
		{
			if (estimate < 16)
			{
				estimate--;
				if (estimate == 0) throw std::logic_error( "division estimate got zero");
			}
			else
			{
				estimate -= estimate >> 4;
			}
			digits_multiplication( part, opr, estimate);
			part.m_sign = false;
			digits_shift( corr, part, estshift);
		}
		BigInt bcdest;
		bcdest.copy( estimate_as_bcd( estimate, estshift));

		digits_multiplication( part, opr, bcdest);

		BigInt newresult;
		digits_addition( newresult, result, bcdest);
		result.swap( newresult);
		BigInt newremainder;
		digits_subtraction( newremainder, remainder, part);
		remainder.swap( newremainder);
	}
	if (opr.sign() != this_.sign())
	{
		result.m_sign = true;
		result.normalize();
	}
}

BigInt BigInt::add( const BigInt& opr) const
{
	BigInt rt;
	if (m_sign == opr.m_sign)
	{
		digits_addition( rt, *this, opr);
	}
	else
	{
		digits_subtraction( rt, *this, opr);
	}
	return rt;
}

BigInt BigInt::sub( const BigInt& opr) const
{
	BigInt rt;
	if (m_sign == opr.m_sign)
	{
		digits_subtraction( rt, *this, opr);
	}
	else
	{
		digits_addition( rt, *this, opr);
	}
	return rt;
}

BigInt BigInt::mul( FactorType opr) const
{
	BigInt val;
	digits_multiplication( val, *this, opr);
	return val;
}

BigInt BigInt::mul( long opr) const
{
	bool ng = false;
	if (opr < 0)
	{
		opr = -opr;
		ng = true;
	}
	BigInt val;
	digits_multiplication( val, *this, opr);
	val.m_sign ^= ng;
	return val;
}

BigInt BigInt::mul( const BigInt& opr) const
{
	BigInt val;
	digits_multiplication( val, *this, opr);
	return val;
}

int BigInt::compare( const BigInt& o) const noexcept
{
	if (sign() != o.sign())
	{
		return (sign() == '-')?-1:+1;
	}
	else
	{
		int resOtherSmaller = (sign() == '-')?-1:+1;
		BigInt::const_iterator ii = begin(), ee = end(), oo = o.begin();
		if (ii.size() > oo.size()) return resOtherSmaller;
		if (ii.size() < oo.size()) return -resOtherSmaller;
		for (; ii != ee; ++ii,++oo)
		{
			if (*ii > *oo) return resOtherSmaller;
			if (*ii < *oo) return -resOtherSmaller;
		}
		return 0;
	}
}

static BigInt::FactorType estimate_to_uint( double val) noexcept
{
	std::uint64_t rt = static_cast<std::uint64_t>( std::floor( val * MaxEstimate));
	while (rt >= (std::numeric_limits<BigInt::FactorType>::max() * 1000)) rt /= 1000;
	while (rt >= std::numeric_limits<BigInt::FactorType>::max()) rt /= 10;
	return rt;
}

BigInt::FactorType BigInt::division_estimate( const BigInt& this_, const BigInt& opr) noexcept
{
	double est = 0;
	double div = 0;

	BigInt::const_iterator ic = this_.begin(), ie = this_.end(), oc = opr.begin(), oe = opr.end();
	for (unsigned int kk=0; kk<24; ++kk)
	{
		est *= 10.0;
		if (ic < ie)
		{
			est += *ic;
			++ic;
		}
		div *= 10.0;
		if (oc < oe)
		{
			div += *oc;
			++oc;
		}
	}
	return estimate_to_uint( est / (div+1));
}

BigInt BigInt::estimate_as_bcd( FactorType estimate, int estshift)
{
	unsigned int mm = (estshift>0)?(3+estshift/4):(3-estshift/4);
	BigInt rt;
	rt.allocate( mm);

	if (estimate >= std::numeric_limits<FactorType>::max())
	{
		throw std::logic_error( "division estimate out of range");
	}
	while (estshift < -6) { estimate /= 1000000; estshift += 6;}
	while (estshift < -3) { estimate /= 1000; estshift += 3;}
	while (estshift < -1) { estimate /= 100; estshift += 2;}
	while (estshift <  0) { estimate /= 10; estshift += 1;}
	if (estimate == 0)
	{
		estimate = 1;
	}
	unsigned int bb = estshift/NumDigits, tt = 4*(estshift%NumDigits);
	while (estimate > 0)
	{
		Element dd = estimate % 10;
		estimate /= 10;
		rt.m_ar[ bb] |= dd << tt;

		if (tt == (NumHighShift-4))
		{
			tt = 0;
			bb += 1;
		}
		else
		{
			tt += 4;
		}
	}
	rt.normalize();
	return rt;
}

std::pair<BigInt,BigInt> BigInt::div( const BigInt& opr) const
{
	std::pair<BigInt,BigInt> rt;
	digits_division( rt.first, rt.second, *this, opr);
	return rt;
}

BigInt BigInt::mod( const BigInt& opr) const
{
	std::pair<BigInt,BigInt> rt;
	digits_division( rt.first, rt.second, *this, opr);
	return rt.second;
}

BigInt BigInt::neg() const
{
	BigInt rt(*this);
	rt.m_sign = !rt.m_sign;
	rt.normalize();
	return rt;
}

BigInt BigInt::pow( unsigned long opr) const
{
	BigInt ar[ sizeof opr * 8];
	std::size_t ai = 1, ae = sizeof opr * 8;
	std::size_t mask = 1;
	ar[ 0] = *this;
	for (; ai != ae && (unsigned long)mask <= opr; ++ai,mask <<= 1)
	{
		ar[ ai] = ar[ ai-1] * ar[ ai-1];
	}
	BigInt rt( "1", 1);
	mask = 1;
	ai = 0;
	for (; ai != ae && (unsigned long)mask <= opr; ++ai,mask <<= 1)
	{
		if (opr & mask)
		{
			rt = rt * ar[ ai];
		}
	}
	return rt;
}

