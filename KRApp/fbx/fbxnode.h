#pragma once

#include <KR3/main.h>
#include <KR3/data/linkedlist.h>
#include "fbxproperty.h"

namespace kr
{
	template <typename Derived, typename Iterable, typename T>
	class FilterIterableIterator:public MakeIterableIterator<FilterIterableIterator<Derived, Iterable, T>, T&>
	{
	public:
		using BaseIterator = decltype(((Iterable*)0)->begin());
		using BaseIteratorEnd = decltype(((Iterable*)0)->end());

	private:
		BaseIterator m_iter;
		BaseIteratorEnd m_end;

	public:

		FilterIterableIterator(Iterable* iterable) noexcept
			:m_iter(iterable->begin()), m_end(iterable->end())
		{
			for (;;)
			{
				if (static_cast<Derived*>(this)->test(*m_iter)) break;
				++m_iter;
				if (m_iter == m_end) break;
			}
		}
		void next() noexcept
		{
			for (;;)
			{
				++m_iter;
				if (m_iter == m_end) break;
				if (static_cast<Derived*>(this)->test(*m_iter)) break;
			}
		}
		T& value() const noexcept
		{
			return *m_iter;
		}
		bool isEnd() const noexcept
		{
			return m_iter == m_end;
		}
	};

	namespace fbx {

		class FBXNode;
		class FBXNodeList;

		class FBXNodeList
		{
		public:
			void addPropertyNode(Text name, int16_t) noexcept;
			void addPropertyNode(Text name, bool) noexcept;
			void addPropertyNode(Text name, int32_t) noexcept;
			void addPropertyNode(Text name, float) noexcept;
			void addPropertyNode(Text name, double) noexcept;
			void addPropertyNode(Text name, int64_t) noexcept;
			void addPropertyNode(Text name, View<bool>) noexcept;
			void addPropertyNode(Text name, View<int32_t>) noexcept;
			void addPropertyNode(Text name, View<float>) noexcept;
			void addPropertyNode(Text name, View<double>) noexcept;
			void addPropertyNode(Text name, View<int64_t>) noexcept;
			void addPropertyNode(Text name, View<uint8_t>) noexcept;
			void addPropertyNode(Text name, Text) noexcept;

			void addChild(FBXNode * child) noexcept;
			const LinkedList<FBXNode>& getChildren() const noexcept;

			class Filter :public FilterIterableIterator<Filter, const LinkedList<FBXNode>, FBXNode>
			{
			public:
				Filter(const LinkedList<FBXNode> * list, Text name) noexcept;
				bool test(FBXNode & node) noexcept;

			private:
				Text m_name;
			};

			FBXNode* getChild(Text name) const noexcept;
			FBXNode & operator [](Text name) const throws(NotFoundException);
			Filter getChildren(Text name) const noexcept;

		protected:
			LinkedList<FBXNode> m_children;
		};

		class FBXNode:public Node<FBXNode, true>, public FBXNodeList
		{
		public:
			FBXNode() noexcept;
			FBXNode(AText name) noexcept;
			FBXNode(const FBXNode&) = delete;

			size_t read(io::VIStream<void> &reader, size_t start_offset) noexcept;
			size_t write(io::VOStream<void> &writer, size_t start_offset) const noexcept;
			bool isNull() const noexcept;

			void addProperty(int16_t) noexcept;
			void addProperty(bool) noexcept;
			void addProperty(int32_t) noexcept;
			void addProperty(float) noexcept;
			void addProperty(double) noexcept;
			void addProperty(int64_t) noexcept;
			void addProperty(View<bool>) noexcept;
			void addProperty(View<int32_t>) noexcept;
			void addProperty(View<float>) noexcept;
			void addProperty(View<double>) noexcept;
			void addProperty(View<int64_t>) noexcept;
			void addProperty(View<uint8_t>) noexcept;
			void addProperty(Text) noexcept;
			void addProperty(FBXProperty) noexcept;

			size_t getBytes() const noexcept;

			Text getName() const noexcept;
			View<FBXProperty> getProperties() const noexcept;

		private:
			Array<FBXProperty> m_properties;
			AText m_name;
		};

	} // namespace fbx
}
