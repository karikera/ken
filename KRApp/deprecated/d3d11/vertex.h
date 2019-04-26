#pragma once

#include <KR3/math/coord.h>


namespace kr
{
	namespace gl
	{
		enum class Semantic
		{
			Position,
			Position2,
			Position3,
			Normal,
			Normal2,
			Normal3,
			Texcoord,
			Texcoord2,
			Texcoord3,
		};
		enum class Format
		{
			Float32x1,
			Float32x2,
			Float32x3,
			Float32x4,
			UNorm8x1, 
			UNorm8x2,
			UNorm8x4,
		};

		namespace __private
		{
			template <Format format> struct TypeByFormat;
			template <> struct TypeByFormat<Format::Float32x2> { using type = vec2; };
			template <> struct TypeByFormat<Format::Float32x3> { using type = vec3; };
			template <> struct TypeByFormat<Format::Float32x4> { using type = vec4; };

			template <Semantic sem, typename T> struct FieldBySemantic;
			template <typename T>  struct FieldBySemantic<Semantic::Position , T> { T position; };
			template <typename T>  struct FieldBySemantic<Semantic::Position2, T> { T position2; };
			template <typename T>  struct FieldBySemantic<Semantic::Position3, T> { T position3; };
			template <typename T>  struct FieldBySemantic<Semantic::Normal,  T> { T normal; };
			template <typename T>  struct FieldBySemantic<Semantic::Normal2, T> { T normal2; };
			template <typename T>  struct FieldBySemantic<Semantic::Normal3, T> { T normal3; };
			template <typename T>  struct FieldBySemantic<Semantic::Texcoord , T> { T texcoord; };
			template <typename T>  struct FieldBySemantic<Semantic::Texcoord2, T> { T texcoord2; };
			template <typename T>  struct FieldBySemantic<Semantic::Texcoord3, T> { T texcoord3; };
		}

		template <Format format>
		using FormatType = typename __private::TypeByFormat<format>::type;
		template <Semantic sem, Format format>
		using SemanticField = __private::FieldBySemantic<sem, FormatType<format>>;
		template <typename attr>
		using SemanticFieldByAttribute = SemanticField<attr::semantic, attr::format>;

		struct AttributeData
		{
			Semantic semantic;
			Format format;
		};


		template <Semantic _semantic, size_t _offset, Format _format> struct Attribute
		{
			static constexpr Semantic semantic = _semantic;
			static constexpr Format format = _format;
			using type = FormatType<_format>;
		};

		namespace vertex
		{
			template <typename ... attrs> struct Attach;
			template <typename attr, typename ... attrs> struct Attach<attr, attrs ...>
				:SemanticFieldByAttribute<attr>, Attach<attrs ...>
			{
			};
			template <> class Attach<>
			{
			};
		}

		template <typename ... attrs> class TVertex:public vertex::Attach<attrs ...>
		{
		public:
			TVertex()
			{
			}
			TVertex(const typename attrs::type & ... data)
			{
				(*this)(data ...);
			}
			TVertex& operator ()(const typename attrs::type & ... data)
			{
				byte * dest = (byte*)this;
				auto loop = [&idx](auto &t)
				{
					*(decltype(t)*)dest = t;
					dest += sizeof(t);
					return 0;
				};

				unwrap(loop(data) ...);
			}

			void TAPosition(const frect &rc)
			{
				(this)[0].position = { rc.x, rc.y, 0, 1 };
				(this)[1].position = { rc.r, rc.y, 0, 1 };
				(this)[2].position = { rc.x, rc.b, 0, 1 };
				(this)[3].position = { rc.r, rc.b, 0, 1 };
			}
			void TATexCoord(float w, float h)
			{
				float ox = 0.5f / w;
				float oy = 0.5f / h;
				float or = 1 + ox;
				float ob = 1 + oy;
				(this)[0].texcoord = { ox, oy };
				(this)[1].texcoord = { or, oy };
				(this)[2].texcoord = { ox, ob };
				(this)[3].texcoord = { or, ob };
			}
			void TATexCoord()
			{
				float w = this[1].x() - this[0].x();
				float h = this[2].y() - this[0].y();
				TATexCoord(w, h);
			}
			void TATexCoord(const frect &rc, float w, float h)
			{
				float tw = 1.f / rc.r;
				float th = 1.f / rc.b;
				float ox = 0.5f / w / rc.r;
				float oy = 0.5f / h / rc.b;

				float u = rc.x*tw + ox;
				float v = rc.y*th + oy;
				float r = u + tw;
				float b = v + th;

				this[0].texcoord = {u, v};
				this[1].texcoord = {r, v};
				this[2].texcoord = {u, b};
				this[3].texcoord = {r, b};
			}
			void TATexCoord(const frect &rc)
			{
				float w = this[1].x() - this[0].x();
				float h = this[2].y() - this[0].y();
				TATexCoord(rc, w, h);
			}

			
			static const AttributeData attributes[sizeof ... (attrs)];
		};

		template <typename ... attrs>
		const AttributeData TVertex<attrs ...>::attributes[sizeof ... (attrs)] = 
		{
			{attrs::semantic , attrs::format}...
		};

	}
}
