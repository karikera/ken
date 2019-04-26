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

	// 3D ��
	class Model
	{
		friend ModelRenderer;

	private:
		struct AnimationExtra;
		struct MeshExtra;

	public:
		class Animation;

		// �ִϸ��̼� ����
		struct AnimationStatus
		{
			// ����� �ִϸ��̼� (in)
			Animation * animation; 

			// ���� �ִϸ��̼� ��� ��ġ (in out)
			double time;

			// �ִϸ��̼��� �Ѿ ���� (out)
			bool overed;

		};

		// 3D ���� ����
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

			// �Ҵ�Ǿ��ִ� �޸𸮸� ����ϴ�.
			inline void clear();

			// Ư�� ����� ����� �����մϴ�.
			// Row-Based ���
			inline void setNodeTransform(size_t idx, const Matrix& m);

			// Ư�� ����� ����� �����ɴϴ�.
			// Row-Based ���
			inline const Matrix& getNodeTransform(size_t idx) const;

			// �ش� ��ķ� ���� ��� ������ŵ�ϴ�.
			inline void transform(const Matrix& m);

			// �ִϸ��̼ǿ��� ���� ��������
			inline bool set(AnimationStatus* status);
			
			// ��� ��� ��
			inline Pose& operator *= (float weight);
			
			// ��� ����
			inline Pose& operator += (const Pose & other);
			
			// �ش� ��� ����
			inline Pose& operator *= (const Matrix& m);

			// ��� ��� ��
			inline Pose operator * (float weight) const;

			// ��� ����
			inline const Pose operator + (const Pose & other) const;

			// ��� ����
			inline const Pose operator * (const Matrix& m) const;

			// ��� ��� ��
			friend inline const Pose operator * (float weight, const Pose & other)
			{
				return other * weight;
			}

			// ��� ����
			friend inline const Pose operator * (const Matrix& m, const Pose & other)
			{
				Pose pose = other;
				pose.transform(m);
				return pose;
			}
			
		};

		// �ִϸ��̼� ����
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

			// �𵨿��� �ִϸ��̼� ����
			CBS_DX11LIB_EXPORT Animation(Model * model, size_t idx);

			// �ִϸ��̼� ���� (�� ����)
			inline double getDuration() const;

			// �ִϸ��̼��� Tick �� ��(Seconds)
			inline double getTPS() const;

			// �ִϸ��̼��� Tick �� ��(Seconds) ����
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

		// ��ü �ִϸ��̼��� �ʴ� Tick Count�� ����
		inline void setAnimationTPS(double tps);

		// �ִϸ��̼� ���� ��������
		CBS_DX11LIB_EXPORT size_t getAnimationCount() const;

		// �ִϸ��̼� ���� ��������
		inline Animation * getAnimation(size_t animation) const;

		// �ִϸ��̼� ���� ��������
		CBS_DX11LIB_EXPORT size_t getMaterialCount() const;

		inline Material * getMaterial(size_t idx) const;


		// Assimp Scene ��������
		inline const aiScene * getAIScene() const;

		// ���� ��ü�� �����ϴ����� ���� ��ȯ
		inline operator bool();

		// ���� ��ü�� �������� ���θ� ��ȯ
		inline bool operator !();

	private:
		CBS_DX11LIB_EXPORT void _create(D3D11Device * device, const char * strName, DataList<VertexLayout> basic_vl, DataList<VertexLayout> skinned_vl);
		Model(const Model& _copy) = delete;
		Model & operator =(const Model & _copy) = delete;
		inline void _makeTexture(D3D11Device* device, const char * strName);
		inline void _makeBuffer(D3D11Device * device, DataList<VertexLayout> basic_vl, DataList<VertexLayout> skinned_vl);
		template <typename LAMBDA>
		inline void _callEachNode(aiNode * node, LAMBDA &lambda);

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
	
	// �� ��±�
	// setMaterial�� �������̵� �Ͽ�, ���� ���� ������ ����
	// setWorld�� �������̵� �Ͽ�, ���� ����� �����ϸ�, �Ϲ� ���̴��� �����ϵ��� ���� �ʿ�
	// setBoneWorlds�� �������̵� �Ͽ�, ���� ����� �����ϸ�, ��Ų�� ���̴��� �����ϵ��� ���� �ʿ�
	class ModelRenderer
	{
	private:
		AutoRelease<ID3D11DeviceContext> m_mrContext;

		inline void _renderNode(const Model & model, aiNode * node, const Matrix & mParent);
		inline void _renderNode(const Model & model, aiNode * node, const Model::Pose & pose);
		
	public:
		CBS_DEPRECATED inline ModelRenderer();
		inline ModelRenderer(D3D11Device * device);

		// �ִϸ��̼� ���� ������
		CBS_DX11LIB_EXPORT void render(const Model & model, const Matrix & world);

		// �ش� ����� ������
		CBS_DX11LIB_EXPORT void render(const Model & model, const Model::Pose & pose);

		// �������� �������� ����� ������ �޾ƿ´�.
		virtual void setMaterial(const Material & mtl) = 0;

		// �������� �������� ����� ����� �޾ƿ´�
		// �Ϲ� ���̴��� ����ϵ��� ������ �ʿ�
		// Row-Based ���
		virtual void setWorld(const Matrix & matrix) = 0;

		// �������� �������� ����� ��� ����� �޾ƿ´�
		// ��Ų�� ���̴��� ����ϵ��� ������ �ʿ�
		// 4x3 Row-Based ���
		virtual void setSkinnedWorlds(const Matrix4x3 * world, size_t m4x3count);

		// �������� �������� ����� ��� ����� �޾ƿ´�
		// 4x3 Row-Based ���
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
