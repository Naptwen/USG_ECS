#ifndef __UMEMORY_HPP__
#define __UMEMORY_HPP__

/* Memory system version 1.0.1*/
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
#include <stdarg.h>
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

#if __cplusplus < 201103L
_EMIT_STL_WARNING(STL4038, "The contents are available only with C++11 or later.");
#endif

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
#define ULOG(COLOR, TEXT, ...) printf(COLOR "(%s:%d) " TEXT UEND, __func__, __LINE__, ##__VA_ARGS__)
#define UASSERT(COND, TEXT, ...) do{ if(!(COND)){ULOG(UERROR, TEXT, ##__VA_ARGS__); exit(1); } } while(false)
#else
#define ULOG_F 
#define ULOG(COLOR, TEXT, ...) 
#define UASSERT(COND, TEXT, ...) do{ } while(false)
#endif

#define rule2(X) X (const X& other)
#define rule3(X) X (X&& other) noexcept
#define rule4(X) const X& operator = (X const& other)
#define rule5(X, ...) const X& operator = (X&& other) noexcept

namespace umemory
{
	typedef class uanybuffer
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
		uanybuffer(void) : ref(new int(1)) {}
		template<typename T>
		uanybuffer(T obj, unsigned long long _key = 0) : ref(new int(1))
		{
			memory_sz = (sizeof(T) + 1) + _MOD((sizeof(T) + 1), _OSBYTE);
			memory = new _BTYPE[memory_sz];
			cur_ptr = memory;
			_BTYPE dataSize = ((_BTYPE)(sizeof(T)));
			if (dataSize == 0) return;
			CREATCHUNK(dataSize);
			SETDATA(obj);
		}
		uanybuffer(_BTYPE* memory, _BTYPE* cur_ptr, size_t memory_sz, int* ref) : memory(memory), cur_ptr(cur_ptr), memory_sz(memory_sz), ref(ref) { (*ref)++; }
		rule2(uanybuffer) : uanybuffer(other.memory, other.cur_ptr, other.memory_sz, other.ref) {}
		rule3(uanybuffer)
		{
			this->memory	= std::exchange(other.memory, nullptr);
			this->memory_sz = std::exchange(other.memory_sz, 0);
			this->cur_ptr	= std::exchange(other.cur_ptr, nullptr);
			this->ref		= std::exchange(other.ref, nullptr);
		}
		rule4(uanybuffer)
		{
			return (this == &other) ? *this : *this = uanybuffer(other);
		}
		rule5(uanybuffer)
		{
			this->memory = std::exchange(other.memory, nullptr);
			this->memory_sz = std::exchange(other.memory_sz, 0);
			this->cur_ptr = std::exchange(other.cur_ptr, nullptr);
			this->ref = std::exchange(other.ref, nullptr);
			return *this;
		}
		~uanybuffer()
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
			else if (ref != nullptr)
			{
				--(*ref);
			}
		}
		void release() {
			if (ref && --(*ref) == 0)
				delete ref;
		}
	public:
		template<typename T> T& GET()
		{
			return *reinterpret_cast<T*>(GETDATA());
		}
		void SHOWINFO()
		{
			HEADINFO(cur_ptr);
		}
	protected:
	} UBUFF;

	typedef class usmartAny
	{
	protected:
		struct smart_base {};
		template<typename T>
		struct smart_data : smart_base
		{
			smart_data(T const& data) : _data(data) {}
			T _data;
		};
		smart_base* mData;
		int* ref_ct;
	public:
		usmartAny(void) : ref_ct(new int(1)), mData(nullptr) {};
		template<typename T>
		usmartAny(T const& data) : ref_ct(new int(1)), mData(new smart_data<T>(data)) {};
		usmartAny(int* ref_ct, smart_base* mData) : ref_ct(ref_ct), mData(mData) { ++(*ref_ct); }
		rule2(usmartAny) : usmartAny(other.ref_ct, other.mData) { }
		rule3(usmartAny)
		{
			ref_ct = std::exchange(other.ref_ct, nullptr); 
			mData = std::exchange(other.mData, nullptr);
		}
		rule4(usmartAny)
		{
			release();
			return (this == &other) ? *this : *this = usmartAny(other);
		}
		rule5(usmartAny)
		{ 
			release();
			ref_ct = std::exchange(other.ref_ct, nullptr); 
			mData = std::exchange(other.mData, nullptr); 
			return *this; 
		}
		~usmartAny() { release(); }
		void release() 
		{
			if (ref_ct && --(*ref_ct) == 0)
			{
				delete ref_ct;
				delete mData;
			}				
		}
	public:
		template<typename T>
		T* Get() { return &(static_cast<smart_data<T>*>(mData)->_data); }
	} UANY;

	template<typename T1, typename T2>
	struct UPAIR
	{
	public:
		enum { N = 2 };
		UPAIR() : var1(), var2() {}
		explicit UPAIR(T1 first, T2 second) : var1(first), var2(second) {}
		T1& v1() { return var1; }
		T2& v2() { return var2; }
		template<typename U1, typename U2> UPAIR(UPAIR<U1, U2> const& d) : var1(d.v1()), var2(d.v2()) {}
		template<typename U1, typename U2> UPAIR<T1, T2>& operator = (UPAIR<U1, U2> const& d) { var1 = d.var1; var2 = d.var2; return *this; }
	private:
		T1 var1;
		T2 var2;
	} ;
	template<typename T1, typename T2, typename U1, typename U2>
	inline bool operator == (UPAIR<T1, T2> const& d1, UPAIR<U1, U2> const& d2) { return d1.v1() == d2.v1() && d1.v2() == d2.v2(); }
	template<typename T1, typename T2, typename U1, typename U2>
	inline bool operator != (UPAIR<T1, T2> const& d1, UPAIR<U1, U2> const& d2) { return !(d1 == d2); }
	template<typename T1, typename T2> inline UPAIR<T1, T2> umake_pair (T1 const & a, T2 const & b) { return UPAIR<T1, T2>(a,b); }
}

#endif