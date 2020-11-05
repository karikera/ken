#pragma once

#include "glshader.h"

#include <KR3/meta/if.h>

namespace kr
{
	namespace gl
	{
		namespace _pri_
		{
			template <template <typename> class type>
			struct GetVertexType;
			
			template <template <typename> class... attributes>
			struct LocPack;

			template <template <typename> class... attributes>
			struct VertexMakeImpl;

			template <>
			struct VertexMakeImpl<>
			{
			protected:
				template <template <typename> class T>
				void* _getPointer() noexcept {
					_assert(!"NOT FOUND");
					return nullptr;
				}
			};

			template <template <typename> class noattribute, template <typename> class... attributes>
			struct VertexMakeImpl<noattribute, attributes...> :VertexMakeImpl<attributes ...>
			{
			};

			template <template <typename> class A>
			struct IsAttribute
			{
				static constexpr bool value = false;
			};

			template <template <typename> class A, template <typename> class B>
			struct IsSameAttribute
			{
				static constexpr bool value = false;
			};

			template <template <typename> class A>
			struct IsSameAttribute<A, A>
			{
				static constexpr bool value = true;
			};

			template <typename result, template <typename> class... attributes>
			struct VertexMakeRev;

			template <template <typename> class... reversed, template <typename> class first, template <typename> class... attributes>
			struct VertexMakeRev<VertexMakeImpl<reversed ...>, first, attributes...>: VertexMakeRev<VertexMakeImpl<first, reversed ...>, attributes ...>
			{
			};

			template <template <typename> class... reversed>
			struct VertexMakeRev<VertexMakeImpl<reversed ...>>
			{
				using type = VertexMakeImpl<reversed ...>;
			};
		}

#define MAKE_ATTRIB(name, count) \
		namespace attrib { \
			template <typename Next> \
			class name :public Next \
			{ \
			private: \
				AttribLocation m_attr; \
			protected: \
				static constexpr size_t locationCount = Next::locationCount + 1; \
				static constexpr size_t attribCount = Next::attribCount + 1; \
				void set(View<const char*> names) noexcept \
				{ \
					Next::set(names); \
					m_attr = Next::getAttribLocation(names[locationCount-1]); \
				} \
			public: \
				void enable() noexcept \
				{ \
					Next::enable(); \
					m_attr.enable(); \
				} \
				void disable() noexcept \
				{ \
					Next::disable(); \
					m_attr.disable(); \
				} \
				template <typename T> \
				void pointer() const noexcept \
				{ \
					Next::template pointer<T>(); \
					m_attr.pointer(count, GL_FLOAT, 0, sizeof(T), offsetof(T, name)); \
				} \
			}; \
		} \
		template <> \
		struct _pri_::GetVertexType<attrib::name>{ \
			using type = math::vector<float, count>; \
		}; \
		template <> \
		struct _pri_::IsAttribute<attrib::name> \
		{ \
			static constexpr bool value = true; \
		}; \
		template <template <typename> class... attributes> \
		struct _pri_::VertexMakeImpl<attrib::name, attributes...> : VertexMakeImpl<attributes ...> { \
		private: \
			using vec_t = typename GetVertexType<attrib::name>::type; \
			using Super = VertexMakeImpl<attributes ...>; \
		public: \
			vec_t name; \
		protected: \
			template <template <typename> class T> \
			void* _getPointer() noexcept { \
				if (IsSameAttribute<T, attrib::name>::value) \
					return &name; \
				else \
					return Super::template _getPointer<T>(); \
			} \
		}; \

#define MAKE_UNIFORM(name, type) \
		namespace uniform { \
			template <typename Next> \
			class name :public Next \
			{ \
			private: \
				UniformLocation m_uni; \
			 \
			protected: \
				static constexpr size_t locationCount = Next::locationCount + 1; \
				static constexpr size_t uniformCount = Next::uniformCount + 1; \
				void set(View<const char*> names) noexcept \
				{ \
					Next::set(names); \
					m_uni = Next::getUniformLocation(names[locationCount-1]); \
				} \
			public: \
				void set_##name(const type & value) noexcept \
				{ \
					m_uni.set(value); \
				} \
			}; \
		}

		MAKE_ATTRIB(pos, 3);
		MAKE_ATTRIB(uv, 2);
		MAKE_UNIFORM(texture, int);
		MAKE_UNIFORM(transform, mat4a);
		MAKE_UNIFORM(color, vec4a);

#undef MAKE_ATTRIB
#undef MAKE_UNIFORM

		namespace _pri_
		{
			template <typename out, template <typename> class... inputs>
			struct VertexMakeFilter;

			template <template <typename> class ... outs, template <typename> class first, template <typename> class... next>
			struct VertexMakeFilter<LocPack<outs ...>, first, next ...> : meta::if_t<IsAttribute<first>::value,
				VertexMakeFilter<LocPack<outs ..., first>, next ...>,
				VertexMakeFilter<LocPack<outs ...>, next ...>
			>
			{
			};

			template <template <typename> class ... attributes>
			struct VertexMakeFilter<LocPack<attributes ...>>
			{
				struct Vertex : VertexMakeRev<VertexMakeImpl<>, attributes ...>::type
				{
				private:
					using Super = typename VertexMakeRev<VertexMakeImpl<>, attributes ...>::type;

				public:
					Vertex() = default;
					Vertex(const typename _pri_::GetVertexType<attributes>::type& ... values) noexcept
					{	
						unpack(*(decay_t<decltype(values)>*)Super::template _getPointer<attributes>() = values);
					}
				};
			};

			class RenderProgBase:public Program
			{
			protected:
				static constexpr size_t locationCount = 0;
				static constexpr size_t attribCount = 0;
				static constexpr size_t uniformCount = 0;
			public:
				void enable() noexcept
				{
					use();
				}
				void disable() noexcept
				{
				}
				void set(View<const char*> names) noexcept
				{
				}
				template <typename T>
				void pointer() const noexcept
				{
				}
			};

			template <template <typename> class ... LOCS>
			struct RenderProgMake;

			template <>
			struct RenderProgMake<>:protected Program
			{
			public:
				using type = RenderProgBase;
			};

			template <template <typename> class LOC, template <typename> class ... LOCS>
			struct RenderProgMake<LOC, LOCS ...>
			{
			public:
				using type = LOC<typename RenderProgMake<LOCS ...>::type>;
			};

			template <typename ti, typename to>
			struct RenderProgRev;

			template <
				template <typename> class t1, 
				template <typename> class ... ti,
				template <typename> class ... to>
			struct RenderProgRev<LocPack<t1, ti...>, LocPack<to ...>>:RenderProgRev<LocPack<ti ...>, LocPack<t1, to ...>>
			{
			};

			template <template <typename> class ... to>
			struct RenderProgRev<LocPack<>, LocPack<to ...>>:RenderProgMake<to ...>
			{
			};
		}

		template <template <typename> class ... LOCS>
 		class RenderProg :public _pri_::RenderProgRev<_pri_::LocPack<LOCS ...>, _pri_::LocPack<>>::type
		{
			using Super = typename _pri_::RenderProgRev<_pri_::LocPack<LOCS ...>, _pri_::LocPack<>>::type;
		private:
			using Super::set;

		public:
			using Vertex = typename _pri_::VertexMakeFilter<_pri_::LocPack<>, LOCS ...>::Vertex;

			using Super::locationCount;
			using Super::uniformCount;
			using Super::attribCount;
			using Super::enable;
			using Super::disable;
			void create(Shader vs, Shader fs, View<const char*> names) noexcept
			{
				_assert(names.size() == locationCount);

				Super::create(vs, fs);
				Super::set(names);
			}
			template <typename T = Vertex>
			void pointer() const noexcept
			{
				Super::template pointer<T>();
			}
			template <typename T = Vertex>
			void drawDynamic(GLenum mode, kr::Buffer data) const noexcept
			{
				arrayBuffer.dataDynamic(data);
				pointer<T>();
				glDrawArrays(mode, 0, intact<GLsizei>(data.size() / sizeof(T)));
			}
		};
	}
}
