#pragma once

#include <KR3/main.h>
#include <KR3/msg/promise.h>
#include "fbxnode.h"


namespace kr
{
	namespace fbx {

		class FBXDoc:public FBXNodeList
		{
		public:
			static Promise<FBXDoc>* load(Text16 url) noexcept;

			FBXDoc() noexcept;
			FBXDoc(FBXDoc&& _move) noexcept;
			FBXDoc& operator = (FBXDoc&& _move) noexcept;
			void read(io::VIStream<void> reader) throws(InvalidSourceException);
			void write(io::VOStream<void> writer) noexcept;

			void createBasicStructure() noexcept;

			std::uint32_t getVersion() noexcept;
		};

	} // namespace fbx
}
