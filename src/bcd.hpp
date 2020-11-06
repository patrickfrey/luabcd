/*
  Copyright (c) 2020 Patrick P. Frey

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
///\file bcd.hpp
///\brief Defines some operations on arbitrary sized packed bcd numbers
///\note For the Addition,Subtraction and Verification thanks to Douglas W. Jones for http://www.divms.uiowa.edu/~jones/bcd/bcd.html
#ifndef _BCD_ARITHMETIC_HPP_INCLUDED
#define _BCD_ARITHMETIC_HPP_INCLUDED
#include <string>
#include <vector>
#include <cstdint>

namespace bcd {

class BigNumber
{
public:
	/// \brief Constructor
	BigNumber( const char* val, std::size_t valsize);
	/// \brief Destructor
	~BigNumber();

	/// \brief Get the number of significant digits (also the number of elements in digits())
	/// \return the precision
	unsigned int precision() const noexcept		{return m_size;}
	/// \brief Get the number of digits right of the comma
	/// \return the scale
	signed int scale() const noexcept		{return m_scale;}
	/// \brief Get the size of the number (number of significant digits)
	/// \return the size
	unsigned int size() const noexcept		{return m_size;}
	/// \brief Get the sign of the number (true -> negative number)
	/// \return the sign
	bool sign() const noexcept			{return m_sign;}
	/// \brief Get the significant digits of the number
	/// \return the digits [0x00..0x09]
	const unsigned char* digits() const noexcept	{return m_ar;}

private:
	signed int m_scale;		///< scale number of digits right of the comma
	bool m_sign;			///< sign (true, if the number is negative, false if positive or 0)
	std::size_t m_size;		///< size of 'm_ar' in bytes
	unsigned char* m_ar;		///< decimal digits of the number [0x00..0x09]
};


///\class BigInt
///\brief Arbitrary size BCD number type with basic arithmetic operations
class BigInt
{
public:
	typedef std::uint64_t Element;
	typedef std::uint64_t FactorType;

	BigInt() noexcept;
	BigInt( const std::string& numstr);
	BigInt( const char* numstr, std::size_t numlen);
	BigInt( long num);
	BigInt( unsigned long num);
	BigInt( double num);
	BigInt( const BigInt& o);
	BigInt( const BigNumber& num);
	~BigInt();

	BigInt& operator=( const BigInt& o)		{copy( o); return *this;}
	void init( const BigInt& o)			{copy( o);}
	void init( const std::string& str);
	void init( const char* str, std::size_t strsize);
	void init( long num);
	void init( unsigned long num);
	void init( double num);
	void init( const BigNumber& num);
	void init();

	std::string tostring() const;
	long toint() const;
	double todouble() const;
	void swap( BigInt& o) noexcept;

	std::pair<BigInt,BigInt> operator /( const BigInt& opr) const	{return div( opr);}
	BigInt operator *( const BigInt& opr) const			{return mul( opr);}
	BigInt operator *( long opr) const				{return mul( opr);}
	BigInt operator +( const BigInt& opr) const			{return add( opr);}
	BigInt operator -( const BigInt& opr) const			{return sub( opr);}
	BigInt operator -() const					{return neg();}

	BigInt add( const BigInt& opr) const;
	BigInt sub( const BigInt& opr) const;
	BigInt mul( FactorType opr) const;
	BigInt mul( long opr) const;
	BigInt mul( const BigInt& opr) const;
	std::pair<BigInt,BigInt> div( const BigInt& opr) const;
	BigInt mod( const BigInt& opr) const;
	BigInt neg() const;
	BigInt pow( unsigned long opr) const;

	//\brief Get Values of bits needed for bitwise operations
	static std::vector<BigInt> getBitValues( int nofBits);
	//\brief Bitwise AND
	//\note This operation is very inefficient on BCD
	BigInt bitwise_and( const BigInt& opr, const std::vector<BigInt>& bitvalues) const;
	//\brief Bitwise OR
	//\note This operation is very inefficient on BCD
	BigInt bitwise_or( const BigInt& opr, const std::vector<BigInt>& bitvalues) const;
	//\brief Bitwise XOR
	//\note This operation is very inefficient on BCD
	BigInt bitwise_xor( const BigInt& opr, const std::vector<BigInt>& bitvalues) const;
	//\brief Bitwise NOT
	//\note This operation is very inefficient on BCD
	BigInt bitwise_not( const std::vector<BigInt>& bitvalues) const;

	BigInt shift( int digits) const;
	BigInt cut( unsigned int digits) const;
	BigInt round( const BigInt& gran) const;

	void invert_sign()					{m_sign = !m_sign; normalize();}
	char sign() const noexcept				{return m_sign?'-':'+';}

	bool operator==( const BigInt& o) const noexcept	{return compare(o)==0;}
	bool operator!=( const BigInt& o) const noexcept	{return compare(o)!=0;}
	bool operator<=( const BigInt& o) const noexcept	{return compare(o)<=0;}
	bool operator<( const BigInt& o) const noexcept		{return compare(o)<0;}
	bool operator>=( const BigInt& o) const noexcept	{return compare(o)>=0;}
	bool operator>( const BigInt& o) const noexcept		{return compare(o)>0;}
	int compare( const BigInt& o) const noexcept;

	bool cmpeq( const BigInt& o) const noexcept		{return compare(o)==0;}
	bool cmple( const BigInt& o) const noexcept		{return compare(o)<=0;}
	bool cmplt( const BigInt& o) const noexcept		{return compare(o)<0;}

	bool isValid() const noexcept;
	bool isNull() const noexcept;
	std::size_t nof_digits() const noexcept			{return begin().size();}

	friend class const_iterator;
	class const_iterator
	{
	public:
		const_iterator() noexcept;
		const_iterator( const BigInt& bcd) noexcept;
		const_iterator( const const_iterator& o) noexcept	:m_idx(o.m_idx),m_shf(o.m_shf),m_ar(o.m_ar){}
		const_iterator& operator++() noexcept			{increment(); return *this;}
		const_iterator operator++(int) noexcept			{const_iterator rt(*this); increment(); return rt;}
		unsigned char operator*() const;
		const const_iterator* operator->() const noexcept	{return this;}

		char ascii() const noexcept				{return operator*() + '0';}
		std::size_t size() const noexcept;

		bool operator==( const const_iterator& o) const noexcept	{return isequal(o);}
		bool operator!=( const const_iterator& o) const noexcept	{return !isequal(o);}
		bool operator<=( const const_iterator& o) const noexcept	{return isle(o);}
		bool operator<( const const_iterator& o) const noexcept		{return islt(o);}
		bool operator>=( const const_iterator& o) const noexcept	{return !islt(o);}
		bool operator>( const const_iterator& o) const noexcept		{return !isle(o);}

	private:
		void increment() noexcept;
		bool isequal( const const_iterator& other) const noexcept;
		bool islt( const const_iterator& other) const noexcept;
		bool isle( const const_iterator& other) const noexcept;

		std::size_t m_idx;
		unsigned char m_shf;
		const Element* m_ar;
	};

	const_iterator begin() const noexcept				{return const_iterator(*this);}
	const_iterator end() const noexcept				{return const_iterator();}

private:
	void allocate( std::size_t size_);
	void copy( const BigInt& o);
	void normalize();

	static void digits_addition( BigInt& dest, const BigInt& this_, const BigInt& opr);
	static void digits_subtraction( BigInt& dest, const BigInt& this_, const BigInt& opr);
	static void digits_shift( BigInt& dest, const BigInt& this_, int nof_digits);
	static void digits_cut( BigInt& dest, const BigInt& this_, unsigned int nof_digits);
	static void digits_nibble_multiplication( BigInt& dest, const BigInt& this_, unsigned char factor);
	static void digits_16_multiplication( BigInt& dest, const BigInt& this_);
	static void digits_multiplication( BigInt& dest, const BigInt& this_, FactorType factor);
	static void digits_multiplication( BigInt& dest, const BigInt& this_, const BigInt& factor);
	static void digits_division( BigInt& result, BigInt& remainder, const BigInt& this_, const BigInt& factor);
	static FactorType division_estimate( const BigInt& this_, const BigInt& opr) noexcept;
	static BigInt estimate_as_bcd( FactorType estimate, int estshift);

private:
	std::size_t m_size;
	Element* m_ar;
	bool m_sign;
	bool m_allocated;
};

}//namespace
#endif
