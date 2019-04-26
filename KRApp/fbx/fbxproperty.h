#pragma once

#include <KR3/main.h>

namespace kr
{
	namespace fbx
	{
		class FBXProperty
		{
		public:
			struct ValuePtr
			{
				size_t size;
				byte array[1];

				ValuePtr() = delete;

				static ValuePtr * allocate(size_t size, size_t compsize) noexcept;
				static ValuePtr * allocate(io::VIStream<void> & reader, size_t size, size_t compsize) noexcept;
				template <typename T>
				static ValuePtr * allocate(View<T> view) noexcept
				{
					size_t size = view.size();
					size_t fullsize = size * sizeof(T);
					ValuePtr* ptr = (ValuePtr*)_new byte[fullsize + sizeof(size_t)];
					ptr->size = size;
					memcpy(ptr->array, view.data(), fullsize);
					return ptr;
				}
				template <typename T>
				View<T> view() const noexcept
				{
					return View<T>((T*)array, size);
				}
			};

			FBXProperty(io::VIStream<void> &reader) throws(InvalidSourceException);
			~FBXProperty() noexcept;
			FBXProperty(const FBXProperty&) = delete;
			FBXProperty(FBXProperty&& _move);
			// primitive values
			FBXProperty(int16_t) noexcept;
			FBXProperty(bool) noexcept;
			FBXProperty(int32_t) noexcept;
			FBXProperty(float) noexcept;
			FBXProperty(double) noexcept;
			FBXProperty(int64_t) noexcept;
			// arrays
			FBXProperty(View<bool>) noexcept;
			FBXProperty(View<int32_t>) noexcept;
			FBXProperty(View<float>) noexcept;
			FBXProperty(View<double>) noexcept;
			FBXProperty(View<int64_t>) noexcept;
			// raw
			FBXProperty(View<uint8_t>) noexcept;
			// string
			FBXProperty(Text) noexcept;

			void write(io::VOStream<void> &writer) const noexcept;

			AText to_string() noexcept;
			char getType() const noexcept;

			size_t getBytes() const noexcept;

			template <typename T>
			T as() const noexcept;

		private:
			union
			{
				uint64_t m_value;
				bool m_value_bool;
				int16_t m_value_i16;
				int32_t m_value_i32;
				int64_t m_value_i64;
				float m_value_f32;
				double m_value_f64;

				ValuePtr * m_value_ptr;
			};
			char m_type;
		};

	} // namespace fbx


}
