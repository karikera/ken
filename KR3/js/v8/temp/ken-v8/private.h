#pragma once

namespace kr
{
	template <typename T>
	class JsField
	{
	public:
		JsField() noexcept = default;
		JsField(int index) noexcept;
		int getIndex() const noexcept;

	private:
		int m_index;
	};

	class JsFieldMaker
	{
	public:
		JsFieldMaker(kr::V8Class * cls) noexcept;
		~JsFieldMaker() noexcept;

		template <typename T>
		void make(JsField<T> * field) noexcept;

#ifdef __KR3_INCLUDED
		template <typename T>
		void make(Text16 name, JsField<T> * field) noexcept;
		template <typename T>
		void makeReadOnly(Text16 name, JsField<T> * field) noexcept;
#endif

	private:
		kr::V8Class * m_class;
		int m_count;
	};

}