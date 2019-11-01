#pragma once


namespace kr
{

	// 자동 컴포넌트, 복사 함수가 탬플릿 함수로 구현되어,
	// 어떠한 컴포넌트 대상으로도 복사가 가능한 컴포넌트이다.
	struct AutoComponent {};

	// 빈 클래스이다.
	struct Empty {};
	
	// meta
	namespace meta
	{
		template <size_t ... nums>
		struct numbers;
		template <typename dorder, typename sorder, size_t count>
		struct order_util;
	}

	// coord
	namespace math
	{

		using OrderRGBA = meta::numbers<0, 1, 2, 3>;
		using OrderBasic = OrderRGBA;
		using OrderBGRA = meta::numbers<2, 1, 0, 3>;


		enum class matrix_data_type
		{
			none,
			right_bottom,
			width_height
		};

		template <typename T, size_t size, bool aligned = false, typename order = OrderBasic>
		class vector;
		template <typename T, size_t rows, size_t cols, bool aligned, matrix_data_type type>
		class matrix_data;
		template <typename T, size_t rows, size_t cols, bool aligned, matrix_data_type type, typename vectors, typename scalars>
		class matrix_constructor;
		template <typename T, size_t rows, size_t cols, bool aligned = false, matrix_data_type type = matrix_data_type::none>
		class matrix_method;
		template <typename T, size_t rows, size_t cols, bool aligned = false, matrix_data_type type = matrix_data_type::none>
		class matrix;
		template <bool aligned>
		class quaternionT;

		using ipvec2 = vector<intptr_t, 2, false, OrderRGBA>;
		using ipvec3 = vector<intptr_t, 3, false, OrderRGBA>;
		using ipvec4 = vector<intptr_t, 4, false, OrderRGBA>;
		using upvec2 = vector<uintptr_t, 2, false, OrderRGBA>;
		using upvec3 = vector<uintptr_t, 3, false, OrderRGBA>;
		using upvec4 = vector<uintptr_t, 4, false, OrderRGBA>;
		using ivec2 = vector<int, 2, false, OrderRGBA>;
		using ivec3 = vector<int, 3, false, OrderRGBA>;
		using ivec4 = vector<int, 4, false, OrderRGBA>;
		using uvec2 = vector<unsigned int, 2, false, OrderRGBA>;
		using uvec3 = vector<unsigned int, 3, false, OrderRGBA>;
		using uvec4 = vector<unsigned int, 4, false, OrderRGBA>;
		using svec2 = vector<short, 2, false, OrderRGBA>;
		using svec3 = vector<short, 3, false, OrderRGBA>;
		using svec4 = vector<short, 4, false, OrderRGBA>;
		using usvec2 = vector<unsigned short, 2, false, OrderRGBA>;
		using usvec3 = vector<unsigned short, 3, false, OrderRGBA>;
		using usvec4 = vector<unsigned short, 4, false, OrderRGBA>;
		using vec4 = vector<float, 4, false, OrderRGBA>;
		using vec3 = vector<float, 3, false, OrderRGBA>;
		using vec2 = vector<float, 2, false, OrderRGBA>;
		using bvec4 = vector<byte, 4, false, OrderRGBA>;
		using bvec3 = vector<byte, 3, false, OrderRGBA>;
		using bvec2 = vector<byte, 2, false, OrderRGBA>;
		using color = vector<byte, 4, false, OrderBGRA>;
		using mat4 = matrix<float, 4, 4, false, matrix_data_type::none>;
		using mat3 = matrix<float, 3, 3, false, matrix_data_type::none>;
		using mat2 = matrix<float, 2, 2, false, matrix_data_type::none>;
		using mat2p = matrix<float, 3, 2, false, matrix_data_type::none>;
		using quaternion = quaternionT<false>;

		using ivec4a = vector<int, 4, true, OrderRGBA>;
		using vec4a = vector<float, 4, true, OrderRGBA>;
		using imat4a = matrix<int, 4, 4, true, matrix_data_type::none>;
		using mat4a = matrix<float, 4, 4, true, matrix_data_type::none>;
		using quaterniona = quaternionT<true>;

		template <typename T, size_t size> using region = matrix<T, 2, size, false, matrix_data_type::right_bottom>;
		template <typename T, size_t size> using regionwh = matrix<T, 2, size, false, matrix_data_type::width_height>;
		template <typename T> using rect = matrix<T, 2, 2, false, matrix_data_type::right_bottom>;
		template <typename T> using rectwh = matrix<T, 2, 2, false, matrix_data_type::width_height>;
		template <typename T> using cube = matrix<T, 3, 2, false, matrix_data_type::right_bottom>;

		using irect = rect<int>;
		using irectwh = rectwh<int>;
		using icube = cube<int>;
		using urect = rect<unsigned int>;
		using urectwh = rectwh<unsigned int>;
		using ucube = cube<unsigned int>;
		using frect = rect<float>;
		using frectwh = rectwh<float>;
		using fcube = cube<float>;

	}

	using math::ipvec2;
	using math::ipvec3;
	using math::ipvec4;
	using math::upvec2;
	using math::upvec3;
	using math::upvec4;
	using math::ivec2;
	using math::ivec3;
	using math::ivec4;
	using math::uvec2;
	using math::uvec3;
	using math::uvec4;
	using math::svec2;
	using math::svec3;
	using math::svec4;
	using math::usvec2;
	using math::usvec3;
	using math::usvec4;
	using math::vec2;
	using math::vec3;
	using math::vec4;
	using math::bvec2;
	using math::bvec3;
	using math::bvec4;
	using math::color;
	using math::mat2;
	using math::mat2p;
	using math::mat3;
	using math::mat4;
	using math::irect;
	using math::irectwh;
	using math::icube;
	using math::urect;
	using math::urectwh;
	using math::ucube;
	using math::frect;
	using math::frectwh;
	using math::fcube;
	using math::quaternion;

	using math::vec4a;
	using math::ivec4a;
	using math::mat4a;
	using math::imat4a;
	using math::quaterniona;

	// util
	struct BitmapInfo;

	// win
	template <typename T = void> class Handle;

	namespace win
	{
		class Library;
		using Module = Library;
		using Instance = Library;
		class Thread;
		class Window;
		class Dialog;
		class Static;
		class EditBox;
	}

	class EmbeddedBrowser;
	
	// net
	class Socket;
	class Client;
	class SocketException;

	// mt
	class EventHandle;
	class Event;
	class EventDispatcher;

	// data
	template <typename Node> class Chain;
	template <typename Node> class LinkedList;
	template <typename C, bool isCRTP = false> class Node;
	template <class ITERABLE, class PARENT> class TIterable;

	template <class DATA, size_t size = (size_t)-1> class ReadLock;
	template <class DATA, size_t size = (size_t)-1, bool nullterm = false> class WriteLock;

	namespace _pri_
	{
		template <class Derived, typename C, typename Info>
		class IStream_cmpAccessable;
		template <class Derived, typename C, typename Info>
		class OStream_cmpAccessable;
	}

	// 스트림 정보
	// 스트림 정보를 보관한다.
	// _accessable: 버퍼로 직접 액세스 가능한 여부
	// Parent: 추가로 상속되는 클래스
	template <bool _accessable = false, class Parent = Empty>
	class StreamInfo;

	// 읽기 스트림
	// Derived: 자식 클래스
	// Component: 컴포넌트 타입
	// Info: 스트림 상세 정보
	template <class Derived, typename Component, class Info = StreamInfo<>>
	class InStream;

	// 쓰기 스트림
	// Derived: 자식 클래스
	// Component: 컴포넌트 타입
	// Info: 스트림 상세 정보
	template <class Derived, typename Component, class Info = StreamInfo<>>
	class OutStream;

	template <typename Derived, typename C, typename _Info>
	using InOutStream = OutStream<Derived, C, StreamInfo<_Info::accessable, InStream<Derived, C, _Info>>>;

	// io
	namespace io
	{
		// 쓰기 필터 스트림
		// Derived: 자식 클래스
		// Base: 내부에 보관할 스트림
		// autoClose: 소멸자에서 내부 스트림을 delete 할지 여부
		// NewComponent: 변환될 컴포넌트 종류
		template <class Derived, class Base, bool autoClose = false, typename NewComponent = typename Base::Component>
		class FilterOStream;

		// 읽기 필터 스트림
		// Derived: 자식 클래스
		// Base: 내부에 보관할 스트림
		// autoClose: 소멸자에서 내부 스트림림을 delete 할지 여부
		// NewComponent: 변환될 컴포넌트 종류
		template <class Derived, class Base, bool autoClose = false, typename NewComponent = typename Base::Component>
		class FilterIStream;

		template <class Base, typename C>
		class StreamableStream;

		// 읽기전에 버퍼에 한번에 읽어오게 한다.
		// 읽어온 버퍼의 데이터를 참조할 수 있는 함수를 지원한다.
		// 바로 읽어서 구분 후, 버려질 데이터에 대해서 메모리 복사하는 수고를 덜게 된다.
		template <typename Derived, bool autoClose = false>
		class SelfBufferedIStream;

		// 읽기전에 버퍼에 한번에 읽어오게 한다.
		template <typename Derived, bool autoClose = false, size_t BUFFER_SIZE = 8192>
		class BufferedIStream;

		// 쓰기전에 버퍼에 모아서 한번에 쓰기를 하게된다.
		template <typename Derived, bool endFlush = false, bool autoClose = false, size_t BUFFER_SIZE = 8192>
		class BufferedOStream;
	}

	// fs
	class File;

	namespace io
	{
		// 파일 스트림
		// 버퍼를 사용하지 않고, 직접 액세스한다.
		// 버퍼를 사용하려면, FIStream 혹은 FOStream을 사용해야한다.
		template <typename C>
		using FileStream = StreamableStream<File, C>;

		// 파일 읽기 스트림
		// 버퍼에 한번에 읽어서 가져오며,
		// SelfBufferedIStream을 사용한다.
		template <typename C, bool autoClose = true>
		using FIStream = io::SelfBufferedIStream<io::FileStream<C>, autoClose>;
		// 파일 쓰기 스트림
		// 버퍼를 모아 기록하며,
		// BufferedOStream을 사용한다.
		template <typename C, bool endFlush = true, bool autoClose = true>
		using FOStream = io::BufferedOStream<io::FileStream<C>, endFlush, autoClose>;
	}


	// text
	namespace ary
	{
		namespace _pri_
		{
			template <class Data, typename C> class WrapImpl;
		}

		namespace data
		{
			template <size_t CAP, typename C, class Parent> class BufferedData;
			template <typename C, class Parent> class AllocatedData;
			template <typename C, class Parent> class TemporaryData;
			template <typename C, class Parent> class ReadableData;
			template <typename C, class Parent> class WritableData;
			template <typename C, class Parent> class AccessableData;
		}
		namespace method
		{
			template <class Parent> class BufferIOMethod;
			template <class Parent> class BufferIMethod;
			template <class Parent> class IStreamMethod;
			template <class Parent> class OStreamMethod;
			template <class Parent> class IRStreamMethod;
		}

		template <class Data> using Wrap = _pri_::WrapImpl<Data, typename Data::Component>;
	}
	
	// 컨테이너
	// 버퍼나, 스트림의 경우 상속하고 있으며, 컴포넌트 타입을 보관한다.
	// C: 컴포넌트 타입
	// rdonly: 읽기 전용인지 여부
	// Parent: 추가로 상속되는 클래스
	template <typename C, bool rdonly = true, class Parent = Empty>
	class Container;
	
	// 버퍼의 정보
	// C: 컴포넌트의 타입
	// _nullspace: 마지막에 null 종료 문자열이 들어가는지 여부이다.
	// _readonly: 읽기 전용인지 여부이다.
	// Parent: 추가로 상속할 부모 클래스
	template <typename C, bool copyTo = true, bool writeTo = false, bool _nullspace = false, bool _readonly = true, class Parent = Empty>
	class BufferInfo;

	// 버퍼일 수 있는 클래스
	// Derived: 자식 클래스
	// BufferInfo: 버퍼의 상세 정보
	template <class Derived, typename BufferInfo> 
	class Bufferable;

	template <typename Derived, typename C, typename Parent = Empty>
	using Printable = Bufferable<Derived, BufferInfo<C, false, true, false, true, Parent>>;
			
	// 참조하는 배열(쓰기 가능), 근원이 사라지면 안된다.
	template <typename C> using WView = ary::_pri_::WrapImpl<ary::data::AccessableData<C, Empty>, C>;

	// 참조하는 배열(읽기 전용), 근원이 사라지면 안된다.
	template <typename C> using View = ary::_pri_::WrapImpl<ary::data::ReadableData<C, Empty>, C>;

	// 할당된 배열, 복사시 메모리가 할당되어진다.
	template <typename C> using Array = ary::_pri_::WrapImpl<ary::data::AllocatedData<C, Empty>, C>;

	// 임시 배열, 메모리가 스택형 임시 버퍼에 할당된다.
	// 임시 메모리 끼리는 할당과 삭제 순서가 스택 순서여야한다.
	template <typename C> using TmpArray = ary::_pri_::WrapImpl<ary::data::TemporaryData<C, Empty>, C>;

	// 배열 라이터, C로 이루어진 배열등에 기록시킨다.
	template <typename C> using ArrayWriter = ary::_pri_::WrapImpl<ary::data::WritableData<C, Empty>, C>;

	// 정적 배열, 최대 CAP 개의 요소를 담을 수 있다.
	template <typename C, size_t CAP> using BArray = ary::_pri_::WrapImpl<ary::data::BufferedData<CAP, C, Empty>, C>;

	// 2D 할당 배열, 복사시 메모리가 할당되며, 2차원 배열이다.
	template <typename T> using Array2D = Array<Array<T>>;

	// 2D 참조 배열, 할당 배열의 참조이다, 안의 요소는 할당 배열이다.
	template <typename T> using Array2DView = View<Array<T>>;

	// 임시 널 종료 문자열 텍스트, 메모리가 스택형 임시 버퍼에 할당된다.
	// 임시 메모리 끼리는 할당과 삭제 순서가 스택 순서여야한다.
	// const C * 로 자동 캐스팅 가능하며, 캐스팅시 널 종료 문자열을 붙인다.
	// 1024개의 컴포넌트를 미리 할당하여 놓는다.
	template <typename C> class TempSzText;

	// 버퍼를 +연산을 했을 경우 반환된다.
	// 1024개의 컴포넌트를 미리 할당하여 놓는다.
	template <typename C> class RightArray;

	// 임시 버퍼, 메모리가 스택형 임시 버퍼에 할당된다.
	// 임시 메모리 끼리는 할당과 삭제 순서가 스택 순서여야한다.
	using TBuffer = TmpArray<void>;

	// 참조하는 버퍼(읽기 전용), 근원이 사라지면 안된다.
	using Buffer = View<void>;

	// 참조하는 버퍼(읽기+쓰기), 근원이 사라지면 안된다.
	using WBuffer = WView<void>;

	// 할당된 버퍼, 복사시 메모리가 할당되어진다.
	using ABuffer = Array<void>;

	using BufferWriter = ArrayWriter<void>;

	// 참조하는 텍스트(읽기 전용), 근원이 사라지면 안된다.
	using Text = View<char>;
	
	// 참조하는 UTF-16 텍스트(읽기 전용), 근원이 사라지면 안된다.
	using Text16 = View<char16>;

	// 참조하는 UTF-32 텍스트(읽기 전용), 근원이 사라지면 안된다.
	using Text32 = View<char32>;

	// 참조하는 텍스트(읽기+쓰기), 근원이 사라지면 안된다.
	using WText = WView<char>;

	// 참조하는 UTF-16 텍스트(읽기+쓰기), 근원이 사라지면 안된다.
	using WText16 = WView<char16>;

	// 참조하는 UTF-32 텍스트(읽기+쓰기), 근원이 사라지면 안된다.
	using WText32 = WView<char32>;

	// 텍스트 라이터, char 배열등에 기록시킨다.
	using Writer = ArrayWriter<char>;

	// UTF-16 텍스트 라이터, char16 배열등에 기록시킨다.
	using Writer16 = ArrayWriter<char16>;

	// UTF-32 텍스트 라이터, char32 배열등에 기록시킨다.
	using Writer32 = ArrayWriter<char32>;

	// 할당된 텍스트, 복사시 메모리가 할당되어진다.
	using AText = Array<char>;

	// 할당된 UTF-16 텍스트, 복사시 메모리가 할당되어진다.
	using AText16 = Array<char16>;

	// 할당된 UTF-32 텍스트, 복사시 메모리가 할당되어진다.
	using AText32 = Array<char32>;

	// 임시 텍스트, 메모리가 스택형 임시 버퍼에 할당된다.
	// 임시 메모리 끼리는 할당과 삭제 순서가 스택 순서여야한다.
	using TText = TmpArray<char>;

	// 임시 UTF-16 텍스트, 메모리가 스택형 임시 버퍼에 할당된다.
	// 임시 메모리 끼리는 할당과 삭제 순서가 스택 순서여야한다.
	using TText16 = TmpArray<char16>;

	// 임시 UTF-32 텍스트, 메모리가 스택형 임시 버퍼에 할당된다.
	// 임시 메모리 끼리는 할당과 삭제 순서가 스택 순서여야한다.
	using TText32 = TmpArray<char32>;
	
	// 정적 버퍼, 최대 CAP 바이트를 담을 수 있다.
	template <size_t size> using BBuffer = BArray<void, size>;

	// 정적 텍스트, 최대 CAP 개의 char을 담을 수 있다.
	template <size_t size> using BText = BArray<char, size>;

	// 정적 UTF-16 텍스트, 최대 CAP 개의 char16을 담을 수 있다.
	template <size_t size> using BText16 = BArray<char16, size>;

	// 정적 UTF-32 텍스트, 최대 CAP 개의 char32를 담을 수 있다.
	template <size_t size> using BText32 = BArray<char32, size>;

	// 임시 텍스트, 스택형 메모리에 동적할당 되며, 스택 규칙에 따라 할당/해제 되어야한다.
	using TSZ = TempSzText<char>;

	// 임시 UTF-16 텍스트, 스택형 메모리에 동적할당 되며, 스택 규칙에 따라 할당/해제 되어야한다.
	using TSZ16 = TempSzText<char16>;

	// 임시 UTF-32 텍스트, 스택형 메모리에 동적할당 되며, 스택 규칙에 따라 할당/해제 되어야한다.
	using TSZ32 = TempSzText<char32>;
	
	// 버퍼
	template <typename T> class Number;
	template <typename T> class NumberFixed;
	class NumberAddress;
	template <dword FIXED, typename T = float>
	class NumberFloat;

	namespace _pri_
	{
		template <typename T>
		inline Number<T> numberBufferize(T v);
		inline NumberFloat<0> numberBufferize(float v);
		inline NumberFloat<0, double> numberBufferize(double v);
		inline NumberFloat<0, long double> numberBufferize(long double v);
	}

	template <typename T, typename C>
	struct Bufferize;
	template <typename T, typename C>
	using bufferize_t = typename Bufferize<T, C>::type;

	class nullterm_t;
	static nullterm_t &nullterm = nullref;

	// is ~~
	template <template <typename...> class BASE, typename TARGET>
	struct is_base_of_t
	{
		struct conversion_tester
		{
			template <typename ... T>
			conversion_tester(const BASE<T...> &);
		};

		static constexpr bool value = is_convertible<TARGET, conversion_tester>::value;
	};

	template <typename T> struct IsOStream : is_base_of_t<OutStream, T> {};
	template <typename T> struct IsIStream : is_base_of_t<InStream, T> {};
	template <typename T> struct IsContainer
	{
		struct conversion_tester
		{
			template <typename C, bool rdonly, typename Parent>
			conversion_tester(const Container<C, rdonly, Parent> &);
		};

		static constexpr bool value = is_convertible<T, conversion_tester>::value;
	};
	template <typename T> struct IsBuffer : is_base_of_t<Bufferable, T> {};
	template <typename T> struct IsMemBuffer
	{
		struct conversion_tester
		{
			template <typename Derived, typename C, bool _szable, bool _readonly, typename Parent>
			conversion_tester(const Bufferable<Derived, BufferInfo<C, false, false, _szable, _readonly, Parent>>&);
		};

		static constexpr bool value = is_convertible<T, conversion_tester>::value;
	};
	template <typename T> struct IsTransBeffer
	{
		static constexpr bool value = IsBuffer<T>::value && !IsMemBuffer<T>::value;
	};

	// add ~~
	namespace _pri_
	{
		template <typename C, bool rdonly, class Parent, bool exists>
		struct AddContainerCond {
			static_assert(is_same<C, typename Parent::Component>::value, "Container type unmatch");
			static_assert(rdonly == Parent::readonly, "Container readonly unmatch");
			using type = Parent;
		};
		template <typename C, bool rdonly, class Parent>
		struct AddContainerCond<C, rdonly, Parent, false> {
			using type = Container<C, rdonly, Parent>;
		};
		template <typename C, bool rdonly, class Parent>
		struct AddContainerType :AddContainerCond<C, rdonly, Parent, IsContainer<Parent>::value>
		{
		};

		template <typename Derived, typename C, bool copyTo, bool writeTo, bool szable, bool readonly, class Parent, bool exists>
		struct AddBufferableCond {
			static_assert(is_same<C, typename Parent::Component>::value, "Container type unmatch");
			static_assert(szable == Parent::szable, "Container szable unmatch");
			static_assert(readonly == Parent::readonly, "Container readonly unmatch");
			using type = Parent;
		};
		template <typename Derived, typename C, bool copyTo, bool writeTo, bool szable, bool readonly, class Parent>
		struct AddBufferableCond<Derived, C, copyTo, writeTo, szable, readonly, Parent, false> {
			using type = Bufferable<Derived, BufferInfo<C, copyTo, writeTo, szable, readonly, Parent> >;
		};
		template <typename Derived, typename Info>
		struct AddBufferableType;
		template <typename Derived, typename C, bool copyTo, bool writeTo, bool szable, bool readonly, class Parent>
		struct AddBufferableType<Derived, BufferInfo<C, copyTo, writeTo, szable, readonly, Parent> >
			:AddBufferableCond<Derived, C, copyTo, writeTo, szable, readonly, Parent, IsBuffer<Parent>::value>
		{
		};
	}

	template <typename C, bool rdonly, class Parent>
	using AddContainer = typename _pri_::AddContainerType<C, rdonly, Parent>::type;
	template <typename Derived, typename Info>
	using AddBufferable = typename _pri_::AddBufferableType<Derived, Info>::type;

}
