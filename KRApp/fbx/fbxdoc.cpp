#include "fbxdoc.h"

#include <KR3/util/wide.h>
#include <KR3/http/fetch.h>

using namespace kr;
using namespace fbx;

namespace {
	bool checkMagic(io::VIStream<void> &reader) noexcept
	{
		Text magic = "Kaydara FBX Binary  ";
		for (char c : magic) {
			if (reader.readas<char>() != c) return false;
		}
		if (reader.readas<uint8_t>() != 0x00) return false;
		if (reader.readas<uint8_t>() != 0x1A) return false;
		if (reader.readas<uint8_t>() != 0x00) return false;
		return true;
	}

	void writerFooter(io::VOStream<void> &writer) noexcept
	{
		static const uint8_t footer[] = {
			0xfa, 0xbc, 0xab, 0x09,
			0xd0, 0xc8, 0xd4, 0x66, 0xb1, 0x76, 0xfb, 0x83, 0x1c, 0xf7, 0x26, 0x7e, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0xe8, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x5a, 0x8c, 0x6a,
			0xde, 0xf5, 0xd9, 0x7e, 0xec, 0xe9, 0x0c, 0xe3, 0x75, 0x8f, 0x29, 0x0b
		};
		for (unsigned int i = 0; i < sizeof(footer); i++) {
			writer.write(footer[i]);
		}
	}

	constexpr uint32_t VERSION = 7400;
}

FBXDoc::FBXDoc() noexcept
{
}
FBXDoc::FBXDoc(FBXDoc&& _move) noexcept
{
	m_children = move(_move.m_children);
}
FBXDoc& FBXDoc::operator = (FBXDoc&& _move) noexcept
{
	m_children = move(_move.m_children);
	return *this;
}

void FBXDoc::read(io::VIStream<void> reader) throws(InvalidSourceException)
{
	if (!checkMagic(reader)) throw InvalidSourceException();

	uint32_t version = reader.readas<uint32_t>();

	uint32_t maxVersion = VERSION;
	if (version > maxVersion) throw "Unsupported FBX version " + std::to_string(version)
		+ " latest supported version is " + std::to_string(maxVersion);

	size_t start_offset = 27; // magic: 21+2, version: 4
	FBXNode * node = _new FBXNode;
	for (;;)
	{
		start_offset += node->read(reader, start_offset);
		if (node->isNull()) break;
		m_children.attach(node);
		node = _new FBXNode;
	}
	delete node;
}

void FBXDoc::write(io::VOStream<void> writer) noexcept
{
	writer << Text("Kaydara FBX Binary  ").cast<void>();
	writer.writeas<uint8_t>(0);
	writer.writeas<uint8_t>(0x1A);
	writer.writeas<uint8_t>(0);
	writer.writeas<uint32_t>(VERSION);

	size_t offset = 27; // magic: 21+2, version: 4
	for (FBXNode &node : m_children) {
		offset += node.write(writer, offset);
	}
	FBXNode nullNode;
	offset += nullNode.write(writer, offset);
	writerFooter(writer);
}

void FBXDoc::createBasicStructure() noexcept
{
	FBXNode * headerExtension = _new FBXNode("FBXHeaderExtension");
	headerExtension->addPropertyNode("FBXHeaderVersion", (int32_t)1003);
	headerExtension->addPropertyNode("FBXVersion", (int32_t)getVersion());
	headerExtension->addPropertyNode("EncryptionType", (int32_t)0);
	{
		FBXNode * creationTimeStamp = _new FBXNode("CreationTimeStamp");
		creationTimeStamp->addPropertyNode("Version", (int32_t)1000);
		creationTimeStamp->addPropertyNode("Year", (int32_t)2017);
		creationTimeStamp->addPropertyNode("Month", (int32_t)5);
		creationTimeStamp->addPropertyNode("Day", (int32_t)2);
		creationTimeStamp->addPropertyNode("Hour", (int32_t)14);
		creationTimeStamp->addPropertyNode("Minute", (int32_t)11);
		creationTimeStamp->addPropertyNode("Second", (int32_t)46);
		creationTimeStamp->addPropertyNode("Millisecond", (int32_t)917);
		headerExtension->addChild(creationTimeStamp);
	}
	headerExtension->addPropertyNode("Creator", "Blender (stable FBX IO) - 2.78 (sub 0) - 3.7.7");
	{
		FBXNode * sceneInfo = _new FBXNode("SceneInfo");
		
		sceneInfo->addProperty(Text({ 'G','l','o','b','a','l','I','n','f','o',0,1,'S','c','e','n','e','I','n','f','o' }));
		sceneInfo->addProperty("UserData");
		sceneInfo->addPropertyNode("Type", "UserData");
		sceneInfo->addPropertyNode("Version", 100);
		{
			FBXNode * metadata = _new FBXNode("MetaData");
			metadata->addPropertyNode("Version", 100);
			metadata->addPropertyNode("Title", "");
			metadata->addPropertyNode("Subject", "");
			metadata->addPropertyNode("Author", "");
			metadata->addPropertyNode("Keywords", "");
			metadata->addPropertyNode("Revision", "");
			metadata->addPropertyNode("Comment", "");
			sceneInfo->addChild(metadata);
		}
		{
			auto * properties = _new FBXNode("Properties70");
			{
				auto * p = _new FBXNode("P");
				p->addProperty("DocumentUrl");
				p->addProperty("KString");
				p->addProperty("Url");
				p->addProperty("");
				p->addProperty("/foobar.fbx");
				properties->addChild(p);
			}
			{
				auto * p = _new FBXNode("P");
				p->addProperty("SrcDocumentUrl");
				p->addProperty("KString");
				p->addProperty("Url");
				p->addProperty("");
				p->addProperty("/foobar.fbx");
				properties->addChild(p);
			}
			{
				auto * p = _new FBXNode("P");
				p->addProperty("Original");
				p->addProperty("Compound");
				p->addProperty("");
				p->addProperty("");
				properties->addChild(p);
			}
			{
				auto * p = _new FBXNode("P");
				p->addProperty("Original|ApplicationVendor");
				p->addProperty("KString");
				p->addProperty("");
				p->addProperty("");
				p->addProperty("Blender Foundation");
				properties->addChild(p);
			}
			{
				auto * p = _new FBXNode("P");
				p->addProperty("Original|ApplicationName");
				p->addProperty("KString");
				p->addProperty("");
				p->addProperty("");
				p->addProperty("Blender (stable FBX IO)");
				properties->addChild(p);
			}
			{
				auto * p = _new FBXNode("P");
				p->addProperty("Original|ApplicationVersion");
				p->addProperty("KString");
				p->addProperty("");
				p->addProperty("");
				p->addProperty("2.78 (sub 0)");
				properties->addChild(p);
			}
			{
				auto * p = _new FBXNode("P");
				p->addProperty("Original|DateTime_GMT");
				p->addProperty("DateTime");
				p->addProperty("");
				p->addProperty("");
				p->addProperty("01/01/1970 00:00:00.000");
				properties->addChild(p);
			}
			{
				auto * p = _new FBXNode("P");
				p->addProperty("Original|FileName");
				p->addProperty("KString");
				p->addProperty("");
				p->addProperty("");
				p->addProperty("/foobar.fbx");
				properties->addChild(p);
			}
			{
				auto * p = _new FBXNode("P");
				p->addProperty("LastSaved");
				p->addProperty("Compound");
				p->addProperty("");
				p->addProperty("");
				properties->addChild(p);
			}
			{
				auto * p = _new FBXNode("P");
				p->addProperty("LastSaved|ApplicationVendor");
				p->addProperty("KString");
				p->addProperty("");
				p->addProperty("");
				p->addProperty("Blender Foundation");
				properties->addChild(p);
			}
			{
				auto * p = _new FBXNode("P");
				p->addProperty("LastSaved|ApplicationName");
				p->addProperty("KString");
				p->addProperty("");
				p->addProperty("");
				p->addProperty("Blender (stable FBX IO)");
				properties->addChild(p);
			}
			{
				auto * p = _new FBXNode("P");
				p->addProperty("LastSaved|DateTime_GMT");
				p->addProperty("DateTime");
				p->addProperty("");
				p->addProperty("");
				p->addProperty("01/01/1970 00:00:00.000");
				properties->addChild(p);
			}
			sceneInfo->addChild(properties);
		}
		headerExtension->addChild(sceneInfo);
	}
	m_children.attach(headerExtension);
}

std::uint32_t FBXDoc::getVersion() noexcept
{
	return VERSION;
}

Promise<FBXDoc>* FBXDoc::load(Text16 url) noexcept
{
	return fetch::text(url)->then([](AText & data) {
		FBXDoc model;
		Buffer stream = data;
		model.read(&stream);
		data = nullptr;
		return model;
	});
}
