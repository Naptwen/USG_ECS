#ifndef __UMEMORY_HPP__
#define __UMEMORY_HPP__
/* USG (c) July 16th, 2023.
Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:
The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/
#include <stdio.h>
#include <assert.h>
#include <compare>
#include <type_traits>
#include <utility>
/* Chunk Shape:
It is assumed that the chunk is composed of multiple blocks.
The size of each block depends on the _UWSIZE.

Each block consists of several sections:

[HEADER]: The first 5 bits indicate the size of the data, followed by either 001 or 000, indicating the availability of the block.
[DATA]: This section contains the actual data.
[PADDING]: Padding is included to optimize CPU cache hit.
[FOOTER]: The footer section is the same as the header.

Therefore the total size is sizeof(DATA) + 2 with padding size.
*/

// Set Size macro
typedef unsigned char _BTYPE;
#define _OSBYTE 8  // Reading Byte size depends on the OS bit
#define _UWSIZE 8  // One block bit size
#define _UDSIZE 16 // Double block
// Operator macro
#define _PACK(s,a) (((s) << 1) | (a)) // Setting the size and the avaliablity
#define _MOD(n,m) (((m * ((n / m) + 1) - n)) % m) //MOD °è»ê
#define _ODDCONV(x) (x % 2)? (x + 1) : x // Convert Odd to Even
// Get Do somthing for Pointer Macro
#define _PTR(p) (_BTYPE*)(p) //Casting p to the pointer
#define _BLOCK(p) *(_BTYPE*)(p) //Get the block value
#define _PUT(p, v) (_BLOCK(p) = (_BTYPE)(v)) // Set the bock value
#define _ISUSE(p) (_BLOCK(p) & 0x01) // Check abaliablity
#define _GETSZ(p) (_BLOCK(p)  >> 1) // Get ths size of chunk
#define _GETPAD(p) ( _MOD( (_GETSZ(p) + 1), _OSBYTE ) )// Get the size of Padding
// Address Macro
#define _HDPTR(p) (_PTR(p) - _UWSIZE) // Get the pointer current header address
#define _FDPTR(p) (_PTR(p) + (_GETSZ(_HDPTR(p)) + _GETPAD(_HDPTR(p))) * _UWSIZE ) //G et the footer address
#define _PREV_FDPTR(p) (_PTR(p) - _UDSIZE) // Get the previous footer address
#define _NEXT_HDPTR(p) (_FDPTR(p) + _UDSIZE) // Get the next header address
#define _PREV_HDPTR(p) ( _PREV_FDPTR(p) - ( (_GETSZ(_PREV_FDPTR(p))) + _GETPAD(_PREV_FDPTR(p)) ) * _UWSIZE ) // Get the previous header address

// Color CODE
#define UERROR "\033[31m"
#define UWARNING "\033[33m"
#define UINFO "\033[34m"
#define UDEBUG "\033[97m"
#define UEND "\033[0m\n"
#define UPASS "\033[92m"
#define USPECIAL "\033[95m"
#define UPOINT "\033[96m"
// LOG
#ifdef _DEBUG
#define ULOG_F printf("(%s:%s):",__func__,__LINE__)
#define ULOG(COLOR, TEXT, ...) printf(COLOR); printf(TEXT, ##__VA_ARGS__); printf(UEND)
#else
#define ULOG_F 
#define ULOG(COLOR, TEXT, ...) 
#endif

#define _UM umemory:: 

namespace umemory
{
	typedef class uecs_any_with_key
	{
	protected:
		_BTYPE* memory = nullptr;
		_BTYPE* cur_ptr = nullptr;
		size_t memory_sz = 0;
		int* ref = nullptr;

		void HEADINFO(void* ptr)
		{
			ULOG(UERROR, "The warraper		[%p]", this);
			ULOG(UINFO, "Header			[%p]", _HDPTR(ptr));
			ULOG(UPOINT, "current ptr		[%p]", _PTR(ptr));
			ULOG(UPOINT, "Memory Size		[%lld]", memory_sz);
			ULOG(UWARNING, "Total Size		[%d]", _GETSZ(_HDPTR(ptr)) + 1 + _GETPAD(_HDPTR(ptr)));
			ULOG(UWARNING, "Data Size		[%d]", (int)(_GETSZ(_HDPTR(ptr))));
			//ULOG(UERROR, "KEY			[%s]", GETKEY());
			ULOG(UINFO, "Padding			[%d]", _GETPAD(_HDPTR(ptr)));
			ULOG(UINFO, "AVALIABLE		[%d]", _ISUSE(_HDPTR(ptr)));
		}
		void CREATCHUNK(const _BTYPE size)
		{
			_PUT(cur_ptr, _PACK(0, 1));
			cur_ptr += _UWSIZE;
			_PUT(_HDPTR(cur_ptr), _PACK(size, 1));
		}
		template<typename T> void SETDATA(T obj)
		{
			*(reinterpret_cast<T*>(cur_ptr)) = obj;
		}
		void NEXTCHUNK(_BTYPE*& _ptr)
		{
			_ptr = _PREV_HDPTR(_ptr);
		}
		_BTYPE* GETDATA()
		{
			if (cur_ptr == memory)
				cur_ptr = memory + _UWSIZE;
			return ((cur_ptr));
		}
	public:
		// default constructor
		uecs_any_with_key(void) : ref(new int(1)) {}
		// template constructor
		template<typename T>
		uecs_any_with_key(T obj, unsigned long long _key = 0) : ref(new int(1))
		{
			memory_sz = (sizeof(T) + 1) + _MOD((sizeof(T) + 1), _OSBYTE);
			memory = new _BTYPE[memory_sz];
			cur_ptr = memory;
			_BTYPE dataSize = ((_BTYPE)(sizeof(T)));
			if (dataSize == 0) return;
			CREATCHUNK(dataSize);
			SETDATA(obj);
		}
		// copy constructor (lvalue : const structure)
		uecs_any_with_key(uecs_any_with_key const& other) : 
		memory(other.memory), cur_ptr(other.memory), memory_sz(other.memory_sz),ref(other.ref) 
		{
			(*ref)++;
		}
		// move constructor (rvlaue : temp structure)
		uecs_any_with_key(uecs_any_with_key&& other) noexcept
		{
			this->memory	= std::exchange(other.memory, nullptr);
			this->memory_sz = std::exchange(other.memory_sz, 0);
			this->cur_ptr	= std::exchange(other.cur_ptr, nullptr);
			this->ref		= std::exchange(other.ref, nullptr);
		}
		// copy assigment (lvalue : const structure)
		uecs_any_with_key& operator = (uecs_any_with_key const& other)
		{
			return *this = uecs_any_with_key(other);
		}
		// move assigment (rvalue : temp structure)
		uecs_any_with_key& operator = (uecs_any_with_key&& other) noexcept
		{
			this->memory	= std::exchange(other.memory, nullptr);
			this->memory_sz = std::exchange(other.memory_sz, 0);
			this->cur_ptr	= std::exchange(other.cur_ptr, nullptr);
			this->ref		= std::exchange(other.ref, nullptr);
			return *this;
		}
		// destruction
		~uecs_any_with_key()
		{
			if (ref == nullptr || (*ref) == 0)
			{
				delete ref;
				cur_ptr = nullptr;
				memory_sz = 0;
				if (memory != nullptr)
				{
					delete[] memory;
					memory = nullptr;
				}
			}
			else if(ref != nullptr)
			{
				--(*ref);
			}
		}

		template<typename T> T& GET()
		{
			return *reinterpret_cast<T*>(GETDATA());
		}
		void SHOWINFO()
		{
			HEADINFO(cur_ptr);
		}
	protected:
	} UANY;


	struct uany
	{
		struct _base {};
		_base* const mData;
		template<typename _Type>struct _src : _base { _Type data;  _src(_Type const& var) : data(var) {} };
		template<typename _Type>  uany(_Type const& type) : mData(new _src<_Type>(type)) {}
		template<typename _Type> _Type* get(void) { return &static_cast<_src<_Type>*>(mData)->data; }
		uany(void) = default;
		~uany(void){if (mData) delete mData;}
	};
	template<typename T1, typename T2>
	
	struct upair
	{
		enum { N = 2 };
		upair() : var1(), var2() {}
		explicit upair(T1 first, T2 second) : var1(first), var2(second) {}
		T1& v1() { return var1; }
		T2& v2() { return var2; }
		template<typename U1, typename U2> upair(upair<U1, U2> const& d) : var1(d.v1()), var2(d.v2()) {}
		template<typename U1, typename U2> upair<T1, T2>& operator = (upair<U1, U2> const& d) { var1 = d.var1; var2 = d.var2; return *this; }
	private:
		T1 var1;
		T2 var2;
	};
	template<typename T1, typename T2, typename U1, typename U2>
	inline bool operator == (upair<T1, T2> const& d1, upair<U1, U2> const& d2) { return d1.v1() == d2.v1() && d1.v2() == d2.v2(); }
	template<typename T1, typename T2, typename U1, typename U2>
	inline bool operator != (upair<T1, T2> const& d1, upair<U1, U2> const& d2) { return !(d1 == d2); }
	template<typename T1, typename T2> inline upair<T1, T2> umake_pair (T1 const & a, T2 const & b) { return upair<T1, T2>(a,b); }
}

#endif