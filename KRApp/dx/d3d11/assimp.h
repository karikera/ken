#pragma once

#include "common.h"
#include "buffer.h"
#include "texture.h"
#include "state.h"
#include "vertexlayout.h"
#include <cbs/alignedarray.h>
#include <cbs/datalist.h>

struct aiAnimation;
struct aiNode;
struct aiNodeAnim;
struct aiScene;

namespace cbs
{
	struct Material;
	class AssimpLogger;
	class Model;
	class ModelRenderer;
	typedef unsigned short vindex_t;

	const double DEFAULT_TICK_PER_SECOND = 10.0;

	struct Material
	{
		enum {MAX_TEXTURE = 8};
		TextureFile textures[MAX_TEXTURE];
		vec4 diffuse;
		vec4 specular;
		vec4 ambient;
		vec4 emissive;
		RasterizerState rasterizer;
		unsigned int texCount;
		float shininess;
	};

	class AssimpLogger
	{
	public:
		CBS_DX11LIB_EXPORT AssimpLogger();
		CBS_DX11LIB_EXPORT ~AssimpLogger();
	};

	class CBS_DEPRECATED Assimp: public AssimpLogger
	{
	};

	const int AI_VERTEX_WEIGHT_COUNT = 4;
	const int AI_BONE_LIMIT = 80;

	// 3D 모델
	class Model
	{
		friend ModelRenderer;

	private:
		struct AnimationExtra;
		struct MeshExtra;

	public:
		class Animation;

		// 애니메이션 상태
		struct AnimationStatus
		{
			// 재생할 애니메이션 (in)
			Animation * animation; 

			// 현재 애니메이션 재생 위치 (in out)
			double time;

			// 애니메이션이 넘어간 여부 (out)
			bool overed;

		};

		// 3D 모델의 포즈
		class Pose
		{
			friend Model;
			friend ModelRenderer;
		private:
			inline void _resize(size_t size);

			AlignedArray<Matrix, sizeof(__m128)> m_transforms;

		public:
			inline Pose();
			inline ~Pose();
			inline Pose(const Pose& _copy);
			inline Pose(Pose&& _move);
			inline Pose& operator =(const Pose& _copy);
			inline Pose& operator =(Pose&& _move);

			// 할당되어있는 메모리를 지웁니다.
			inline void clear();

			// 특정 노드의 행렬을 변경합니다.
			// Row-Based 행렬
			inline void setNodeTransform(size_t idx, const Matrix& m);

			// 특정 노드의 행렬을 가져옵니다.
			// Row-Based 행렬
			inline const Matrix& getNodeTransform(size_t idx) const;

			// 해당 행렬로 모델의 포즈를 변형시킵니다.
			inline void transform(const Matrix& m);

			// 애니메이션에서 포즈 가져오기
			inline bool set(AnimationStatus* status);
			
			// 행렬 상수 배
			inline Pose& operator *= (float weight);
			
			// 행렬 덧셈
			inline Pose& operator += (const Pose & other);
			
			// 해당 행렬 곱셈
			inline Pose& operator *= (const Matrix& m);

			// 행렬 상수 배
			inline Pose operator * (float weight) const;

			// 행렬 덧셈
			inline const Pose operator + (const Pose & other) const;

			// 행렬 곱셈
			inline const Pose operator * (const Matrix& m) const;

			// 행렬 상수 배
			friend inline const Pose operator * (float weight, const Pose & other)
			{
				return other * weight;
			}

			// 행렬 곱셈
			friend inline const Pose operator * (const Matrix& m, const Pose & other)
			{
				Pose pose = other;
				pose.transform(m);
				return pose;
			}
			
		};

		// 애니메이션 정보
		class Animation
		{
			friend Model;
			friend Pose;
		private:
			double m_tps;
			double m_duration;
			aiAnimation * m_animation;
			aiNode * m_root;
			size_t m_nodeCount;
			size_t m_index;

		public:
			inline Animation();

			// 모델에서 애니메이션 생성
			CBS_DX11LIB_EXPORT Animation(Model * model, size_t idx);

			// 애니메이션 길이 (초 단위)
			inline double getDuration() const;

			// 애니메이션의 Tick 당 초(Seconds)
			inline double getTPS() const;

			// 애니메이션의 Tick 당 초(Seconds) 설정
			CBS_DX11LIB_EXPORT void setTPS(double tps);
		};

		class NodeExtra
		{
		private:
			const size_t m_id;
			AutoDeleteArray<aiNodeAnim *> m_nodeanim;

		public:
			inline NodeExtra(size_t id);
			inline ~NodeExtra();

			inline size_t getId();
			inline bool hasAnimation();
			inline void setAnimationCount(size_t count);
			inline void setAnimation(size_t id, aiNodeAnim * anim);
			inline aiNodeAnim * getAnimation(size_t id);
		};

		static const VertexLayout DEFAULT_BASIC_LAYOUT[3];
		static const VertexLayout DEFAULT_SKINNED_LAYOUT[5];

		inline Model();
		inline explicit Model(D3D11Device * device, const char * strName);
		inline Model(D3D11Device * device, const char * strName, DataList<VertexLayout> basic_vl, DataList<VertexLayout> skinned_vl);
		CBS_DEPRECATED inline explicit Model(const char * strName);
		CBS_DEPRECATED inline Model(const char * strName, DataList<VertexLayout> basic_vl, DataList<VertexLayout> skinned_vl);
		CBS_DX11LIB_EXPORT ~Model();
	
		inline Model(Model&& _move);
		inline Model & operator =(Model && _move);

		// 전체 애니메이션의 초당 Tick Count를 변경
		inline void setAnimationTPS(double tps);

		// 애니메이션 개수 가져오기
		CBS_DX11LIB_EXPORT size_t getAnimationCount() const;

		// 애니메이션 정보 가져오기
		inline Animation * getAnimation(size_t animation) const;

		// 애니메이션 개수 가져오기
		CBS_DX11LIB_EXPORT size_t getMaterialCount() const;

		inline Material * getMaterial(size_t idx) const;


		// Assimp Scene 가져오기
		inline const aiScene * getAIScene() const;

		// 현재 객체가 존재하는지의 여부 반환
		inline operator bool();

		// 현재 객체가 없는지의 여부를 반환
		inline bool operator !();

	private:
		CBS_DX11LIB_EXPORT void _create(D3D11Device * device, const char * strName, DataList<VertexLayout> basic_vl, DataList<VertexLayout> skinned_vl);
		Model(const Model& _copy) = delete;
		Model & operator =(const Model & _copy) = delete;
		inline void _makeTexture(D3D11Device* device, const char * strName);
		inline void _makeBuffer(D3D11Device * device, DataList<VertexLayout> basic_vl, DataList<VertexLayout> skinned_vl);
		template <typename LAMBDA>
		inline void _callEachNode(aiNode * node, LAMBDA &&lambda);

		struct MeshExtra
		{
			UINT stride;
			UINT offset;
			UINT ioffset;
			UINT icount;
			AutoDeleteArray<size_t> boneToNode;
			D3D11_PRIMITIVE_TOPOLOGY topology;
			bool skinned;
		};

		const aiScene* m_scene;
		AutoDeleteArray<MeshExtra> m_meshes;
		AutoDeleteArray<Material> m_materials;
		AutoDeleteArray<Animation> m_animations;
		Buffer m_vb;
		Buffer m_ib;
		size_t m_nodeCount;


	};
	
	// 모델 출력기
	// setMaterial을 오버라이드 하여, 제질 설정 구문을 지정
	// setWorld를 오버라이드 하여, 월드 행렬을 지정하며, 일반 셰이더를 설정하도록 구현 필요
	// setBoneWorlds를 오버라이드 하여, 월드 행렬을 지정하며, 스킨드 셰이더를 설정하도록 구현 필요
	class ModelRenderer
	{
	private:
		AutoRelease<ID3D11DeviceContext> m_mrContext;

		inline void _renderNode(const Model & model, aiNode * node, const Matrix & mParent);
		inline void _renderNode(const Model & model, aiNode * node, const Model::Pose & pose);
		
	public:
		CBS_DEPRECATED inline ModelRenderer();
		inline ModelRenderer(D3D11Device * device);

		// 애니메이션 없이 렌더링
		CBS_DX11LIB_EXPORT void render(const Model & model, const Matrix & world);

		// 해당 포즈로 렌더링
		CBS_DX11LIB_EXPORT void render(const Model & model, const Model::Pose & pose);

		// 렌더러가 렌더링시 사용할 제질을 받아온다.
		virtual void setMaterial(const Material & mtl) = 0;

		// 렌더러가 렌더링시 사용할 행렬을 받아온다
		// 일반 셰이더를 사용하도록 구현이 필요
		// Row-Based 행렬
		virtual void setWorld(const Matrix & matrix) = 0;

		// 렌더러가 렌더링시 사용할 행렬 목록을 받아온다
		// 스킨드 셰이더를 사용하도록 구현이 필요
		// 4x3 Row-Based 행렬
		virtual void setSkinnedWorlds(const Matrix4x3 * world, size_t m4x3count);

		// 렌더러가 렌더링시 사용할 행렬 목록을 받아온다
		// 4x3 Row-Based 행렬
		CBS_DEPRECATED virtual void setBoneWorlds(const Matrix4x3 * matrix, size_t m4x3count);

	};

	class AnimationReader
	{
	public:
		size_t m_index;
		cbs::Model::Pose * m_pose;
		double m_time;
		double m_duration;

		CBS_DX11LIB_EXPORT void getAnimation(aiNode * node, const Matrix & mParent);

	};
}

CBS_SELECTANY const cbs::VertexLayout cbs::Model::DEFAULT_BASIC_LAYOUT[3] =
{
	VertexLayout(VertexLayout::Position, DXGI_FORMAT_R32G32B32_FLOAT),
	VertexLayout(VertexLayout::Normal, DXGI_FORMAT_R32G32B32_FLOAT),
	VertexLayout(VertexLayout::Texcoord, DXGI_FORMAT_R32G32_FLOAT),
};
CBS_SELECTANY const cbs::VertexLayout cbs::Model::DEFAULT_SKINNED_LAYOUT[5] =
{
	VertexLayout(VertexLayout::Position, DXGI_FORMAT_R32G32B32_FLOAT),
	VertexLayout(VertexLayout::Normal, DXGI_FORMAT_R32G32B32_FLOAT),
	VertexLayout(VertexLayout::Texcoord, DXGI_FORMAT_R32G32_FLOAT),
	VertexLayout(VertexLayout::BlendIndices, DXGI_FORMAT_R8G8B8A8_UINT),
	VertexLayout(VertexLayout::BlendWeight, DXGI_FORMAT_R32G32B32A32_FLOAT),
};
