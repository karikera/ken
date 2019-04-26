#include "fbxproperty.h"
#include "zlib.h"

using namespace kr;
using namespace fbx;

namespace
{
	// helpers for reading properties
	size_t getTypeSize(char type) throws(InvalidSourceException)
	{
		switch (type)
		{
		case 'Y': // 2 byte signed integer
			return 2;
		case 'C': case 'B': // 1 bit boolean (1: true, 0: false) encoded as the LSB of a 1 Byte value.
			return 1;
		case 'I': // 4 byte signed Integer
			return 4;
		case 'F': // 4 byte single-precision IEEE 754 number
			return 4;
		case 'D': // 8 byte double-precision IEEE 754 number
			return 8;
		case 'L': // 8 byte signed Integer
			return 8;
		default:
			throw InvalidSourceException();
		}
	}
   
}

FBXProperty::ValuePtr * FBXProperty::ValuePtr::allocate(size_t size, size_t compsize) noexcept
{
	ValuePtr* ptr = (ValuePtr*)_new byte[size * compsize + sizeof(size_t)];
	ptr->size = size;
	return ptr;
}
FBXProperty::ValuePtr * FBXProperty::ValuePtr::allocate(io::VIStream<void> & reader, size_t size, size_t compsize) noexcept
{
	size_t fullsize = size * compsize;
	ValuePtr* ptr = (ValuePtr*)_new byte[fullsize + sizeof(size_t)];
	ptr->size = size;
	reader.read(ptr->array, fullsize);
	return ptr;
}

FBXProperty::FBXProperty(io::VIStream<void> &reader) throws(InvalidSourceException)
{
    m_type = reader.readas<uint8_t>();
    // std::cout << "  " << m_type << "\n";
    if(m_type == 'S' || m_type == 'R')
	{
        uint32_t length = reader.readas<uint32_t>();
		m_value_ptr = ValuePtr::allocate(length, 1);
		m_value_ptr->size = length;
		reader.read(m_value_ptr->array, length);
    }
	else if(m_type < 'Z')
	{
		// primitive types
		reader.read(&m_value, getTypeSize(m_type));
    }
	else
	{
        uint32_t arrayLength = reader.readas<uint32_t>(); // number of elements in array
        uint32_t encoding = reader.readas<uint32_t>(); // 0 .. uncompressed, 1 .. zlib-compressed
        uint32_t compressedLength = reader.readas<uint32_t>();

		char compType = m_type - ('a' - 'A');
        if(encoding) {
			size_t compSize = getTypeSize(compType);
            size_t uncompressedLength = compSize * arrayLength;

			TmpArray<void> compressedBuffer = reader.read(compressedLength);

			m_value_ptr = ValuePtr::allocate(arrayLength, compSize);

            uLongf destLen = intact<uLongf>(uncompressedLength);
            uLongf srcLen = compressedLength;
            uncompress2((Bytef*)m_value_ptr->array, &destLen, (Bytef*)compressedBuffer.data(), &srcLen);

			if (srcLen != compressedLength) throw InvalidSourceException();
			if (destLen != uncompressedLength) throw InvalidSourceException();
        }
		else
		{
			size_t compsize = getTypeSize(compType);
			m_value_ptr = ValuePtr::allocate(reader, arrayLength, compsize);
        }
    }
}
FBXProperty::~FBXProperty() noexcept
{
	if (m_type > 'Z' || m_type == 'S' || m_type == 'R')
	{
		delete m_value_ptr;
	}
}
FBXProperty::FBXProperty(FBXProperty&& _move)
{
	m_value = _move.m_value;
	_move.m_value = 0;
	m_type = _move.m_type;
	_move.m_type = 'i';
}

void FBXProperty::write(io::VOStream<void> &writer) const noexcept
{
    writer.writeas<byte>(m_type);

    if(m_type == 'R' || m_type == 'S')
	{
        writer.writeas<uint32_t>(intact<uint32_t>(m_value_ptr->size));
		writer.write(m_value_ptr->array, m_value_ptr->size);
    }
	else if (m_type <= 'Z')
	{
		// primitive types
		writer.write(&m_value, getTypeSize(m_type));
	}
	else
	{
        writer.writeas<uint32_t>(intact<uint32_t>(m_value_ptr->size)); // arrayLength
        writer.writeas<uint32_t>(0); // encoding // TODO: support compression
		char compType = m_type - ('a' - 'A');
		size_t fullSize = m_value_ptr->size * getTypeSize(compType);
		writer.writeas<uint32_t>(intact<uint32_t>(fullSize));
		writer.write(m_value_ptr->array, fullSize);
    }
}

// primitive m_values
FBXProperty::FBXProperty(int16_t a) noexcept { m_type = 'Y'; m_value_i16 = a; }
FBXProperty::FBXProperty(bool a) noexcept { m_type = 'C'; m_value_bool = a; }
FBXProperty::FBXProperty(int32_t a) noexcept { m_type = 'I'; m_value_i32 = a; }
FBXProperty::FBXProperty(float a) noexcept { m_type = 'F'; m_value_f32 = a; }
FBXProperty::FBXProperty(double a) noexcept { m_type = 'D'; m_value_f64 = a; }
FBXProperty::FBXProperty(int64_t a) noexcept { m_type = 'L'; m_value_i64 = a; }
// arrays
FBXProperty::FBXProperty(View<bool> a) noexcept : m_type('b') {
	m_value_ptr = ValuePtr::allocate(a);
}
FBXProperty::FBXProperty(View<int32_t> a) noexcept : m_type('i') {
	m_value_ptr = ValuePtr::allocate(a);
}
FBXProperty::FBXProperty(View<float> a) noexcept : m_type('f') {
	m_value_ptr = ValuePtr::allocate(a);
}
FBXProperty::FBXProperty(View<double> a) noexcept : m_type('d') {
	m_value_ptr = ValuePtr::allocate(a);
}
FBXProperty::FBXProperty(View<int64_t> a) noexcept : m_type('l') {
	m_value_ptr = ValuePtr::allocate(a);
}
FBXProperty::FBXProperty(View<uint8_t> a) noexcept
	: m_type('R') {
	m_value_ptr = ValuePtr::allocate(a);
}
FBXProperty::FBXProperty(Text a) noexcept
	:m_type('S') {
	m_value_ptr = ValuePtr::allocate(a);
    ;
}

char FBXProperty::getType() const noexcept
{
    return m_type;
}

AText FBXProperty::to_string() noexcept
{
	AText out;
    if(m_type == 'Y') out << m_value_i16;
	else if (m_type == 'C') out << m_value_bool;
    else if(m_type == 'I') out << m_value_i32;
    else if(m_type == 'F') out << m_value_f32;
    else if(m_type == 'D') out << m_value_f64;
    else if(m_type == 'L') out << m_value_i64;
    else if(m_type == 'R') {
		out << "buffer";
		//out << '[';
		//out << Text((char*)m_value_ptr->array, m_value_ptr->size);
		//out << ']';
    } else if(m_type == 'S') {
		out << '\"';
		out << Text((char*)m_value_ptr->array, m_value_ptr->size);
		out << '\"';
    } else {
		out << '[';
		switch (m_type)
		{
		case 'f': 
			for (float v : m_value_ptr->view<float>())
			{
				out << v;
				out << ", ";
			}
			break;
		case 'd':
			for (double v : m_value_ptr->view<double>())
			{
				out << v;
				out << ", ";
			}
			break;
		case 'l':
			for (int64_t v : m_value_ptr->view<int64_t>())
			{
				out << v;
				out << ", ";
			}
			break;
		case 'i':
			for (int32_t v : m_value_ptr->view<int32_t>())
			{
				out << v;
				out << ", ";
			}
			break;
		case 'b':
			for (bool v : m_value_ptr->view<bool>())
			{
				out << v;
				out << ", ";
			}
			break;
		}
		out.resize(out.size() - 2);
		out << ']';
    }

	return out;
}

size_t FBXProperty::getBytes() const noexcept
{
    if(m_type == 'Y') return 2 + 1; // 2 for int16, 1 for m_type spec
	else if (m_type == 'R') return m_value_ptr->size + 5;
	else if (m_type == 'S') return m_value_ptr->size + 5;
	else if (m_type <= 'Z')
	{
		return getTypeSize(m_type) + 1;
	}
	else
	{
		return getTypeSize(m_type - ('a' - 'A')) * m_value_ptr->size + 13;
	}
}

template <>
int32_t FBXProperty::as<int32_t>() const noexcept
{
	_assert(m_type == 'I');
	return m_value_i32;
}
template <>
int16_t FBXProperty::as<int16_t>() const noexcept
{
	_assert(m_type == 'Y');
	return m_value_i16;
}
template <>
int64_t FBXProperty::as<int64_t>() const noexcept
{
	_assert(m_type == 'L');
	return m_value_i16;
}
template <>
float FBXProperty::as<float>() const noexcept
{
	_assert(m_type == 'F');
	return m_value_f32;
}
template <>
double FBXProperty::as<double>() const noexcept
{
	_assert(m_type == 'D');
	return m_value_f64;
}
template <>
bool FBXProperty::as<bool>() const noexcept
{
	_assert(m_type == 'B' || m_type == 'C');
	return m_value_bool;
}

template <>
View<int32_t> FBXProperty::as<View<int32_t>>() const noexcept
{
	_assert(m_type == 'i');
	return View<int32_t>((int32_t*)m_value_ptr->array, m_value_ptr->size);
}
template <>
View<int16_t> FBXProperty::as<View<int16_t>>() const noexcept
{
	_assert(m_type == 'y');
	return View<int16_t>((int16_t*)m_value_ptr->array, m_value_ptr->size);
}
template <>
View<int64_t> FBXProperty::as<View<int64_t>>() const noexcept
{
	_assert(m_type == 'l');
	return View<int64_t>((int64_t*)m_value_ptr->array, m_value_ptr->size);
}
template <>
View<float> FBXProperty::as<View<float>>() const noexcept
{
	_assert(m_type == 'f');
	return View<float>((float*)m_value_ptr->array, m_value_ptr->size);
}
template <>
View<double> FBXProperty::as<View<double>>() const noexcept
{
	_assert(m_type == 'd');
	return View<double>((double*)m_value_ptr->array, m_value_ptr->size);
}
template <>
View<bool> FBXProperty::as<View<bool>>() const noexcept
{
	_assert(m_type == 'b' || m_type == 'c');
	return View<bool>((bool*)m_value_ptr->array, m_value_ptr->size);
}
