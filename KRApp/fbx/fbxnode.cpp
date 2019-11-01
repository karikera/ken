#include "fbxnode.h"

using namespace kr;
using namespace fbx;


FBXNodeList::Filter::Filter(const LinkedList<FBXNode> * list, Text name) noexcept
	:FilterIterableIterator(list), m_name(name)
{
}
bool FBXNodeList::Filter::test(FBXNode & node) noexcept
{
	return node.getName() == m_name;
}

void FBXNodeList::addPropertyNode(Text name, int16_t v) noexcept { auto * n = _new FBXNode(name); n->addProperty(v); addChild(n); }
void FBXNodeList::addPropertyNode(Text name, bool v) noexcept { auto * n = _new FBXNode(name); n->addProperty(v); addChild(n); }
void FBXNodeList::addPropertyNode(Text name, int32_t v) noexcept { auto * n = _new FBXNode(name); n->addProperty(v); addChild(n); }
void FBXNodeList::addPropertyNode(Text name, float v) noexcept { auto * n = _new FBXNode(name); n->addProperty(v); addChild(n); }
void FBXNodeList::addPropertyNode(Text name, double v) noexcept { auto * n = _new FBXNode(name); n->addProperty(v); addChild(n); }
void FBXNodeList::addPropertyNode(Text name, int64_t v) noexcept { auto * n = _new FBXNode(name); n->addProperty(v); addChild(n); }
void FBXNodeList::addPropertyNode(Text name, View<bool> v) noexcept { auto * n = _new FBXNode(name); n->addProperty(v); addChild(n); }
void FBXNodeList::addPropertyNode(Text name, View<int32_t> v) noexcept { auto * n = _new FBXNode(name); n->addProperty(v); addChild(n); }
void FBXNodeList::addPropertyNode(Text name, View<float> v) noexcept { auto * n = _new FBXNode(name); n->addProperty(v); addChild(n); }
void FBXNodeList::addPropertyNode(Text name, View<double> v) noexcept { auto * n = _new FBXNode(name); n->addProperty(v); addChild(n); }
void FBXNodeList::addPropertyNode(Text name, View<int64_t> v) noexcept { auto * n = _new FBXNode(name); n->addProperty(v); addChild(n); }
void FBXNodeList::addPropertyNode(Text name, View<uint8_t> v) noexcept { auto * n = _new FBXNode(name); n->addProperty(v); addChild(n); }
void FBXNodeList::addPropertyNode(Text name, Text v) noexcept { auto * n = _new FBXNode(name); n->addProperty(v); addChild(n); }

void FBXNodeList::addChild(FBXNode * child) noexcept { m_children.attach(child); }

const LinkedList<FBXNode>& FBXNodeList::getChildren() const noexcept
{
	return m_children;
}

FBXNode* FBXNodeList::getChild(Text name) const noexcept
{
	for (FBXNode & node : m_children)
	{
		if (node.getName() == name)
		{
			return &node;
		}
	}
	return nullptr;
}

FBXNode & FBXNodeList::operator [](Text name) const throws(NotFoundException)
{
	FBXNode * finded = getChild(name);
	if (finded == nullptr) throw NotFoundException();
	return *finded;
}
FBXNodeList::Filter FBXNodeList::getChildren(Text name) const noexcept
{
	return Filter(&m_children, name);
}

FBXNode::FBXNode() noexcept
{
}

FBXNode::FBXNode(AText name) noexcept :m_name(move(name))
{
}

size_t FBXNode::read(io::VIStream<void> &reader, size_t start_offset) noexcept
{
    size_t bytes = 0;

    uint32_t endOffset = reader.readas<uint32_t>();
    uint32_t numProperties = reader.readas<uint32_t>();
    uint32_t propertyListLength = reader.readas<uint32_t>();
    uint8_t nameLength = reader.readas<uint8_t>();
	m_name.resize(nameLength);
    reader.read(m_name.data(), nameLength);
    bytes += 13 + nameLength;

    //std::cout << "so: " << start_offset
    //          << "\tbytes: " << (endOffset == 0 ? 0 : (endOffset - start_offset))
    //          << "\tnumProp: " << numProperties
    //          << "\tpropListLen: " << propertyListLength
    //          << "\tnameLen: " << std::to_string(nameLength)
    //          << "\tname: " << name << "\n";

    for(uint32_t i = 0; i < numProperties; i++) {
        addProperty(FBXProperty(reader));
    }
    bytes += propertyListLength;

    while(start_offset + bytes < endOffset) {
        auto * child = _new FBXNode;
        bytes += child->read(reader, start_offset + bytes);
        addChild(child);
    }
    return bytes;
}

size_t FBXNode::write(io::VOStream<void> &writer, size_t start_offset) const noexcept
{
    if(isNull()) {
        //std::cout << "so: " << start_offset
        //          << "\tbytes: 0"
        //          << "\tnumProp: 0"
        //          << "\tpropListLen: 0"
        //          << "\tnameLen: 0"
        //          << "\tname: \n";
        for(int i = 0; i < 13; i++) writer.writeas<byte>(0);
        return 13;
    }

    size_t propertyListLength = 0;
    for(auto &prop : m_properties) propertyListLength += prop.getBytes();
	size_t bytes = 13 + m_name.size() + propertyListLength;
    for(auto &child : m_children) bytes += child.getBytes();

	_assert(bytes == getBytes());
    writer.writeas(intact<uint32_t>(start_offset + bytes)); // endOffset
    writer.writeas(intact<uint32_t>(m_properties.size())); // numProperties
    writer.writeas(intact<uint32_t>(propertyListLength)); // propertyListLength
    writer.writeas(intact<uint8_t>(m_name.size()));
    writer.write(m_name.data(), m_name.size());

    //std::cout << "so: " << start_offset
    //          << "\tbytes: " << bytes
    //          << "\tnumProp: " << m_properties.size()
    //          << "\tpropListLen: " << propertyListLength
    //          << "\tnameLen: " << name.length()
    //          << "\tname: " << name << "\n";

    bytes = 13 + m_name.size() + propertyListLength;

    for(auto & prop : m_properties) prop.write(writer);
    for(auto &child : m_children) bytes += child.write(writer,  start_offset + bytes);

    return bytes;
}

bool FBXNode::isNull() const noexcept
{
    return m_children.size() == 0
            && m_properties.size() == 0
            && m_name.size() == 0;
}

// primitive values
void FBXNode::addProperty(int16_t v) noexcept { addProperty(FBXProperty(v)); }
void FBXNode::addProperty(bool v) noexcept { addProperty(FBXProperty(v)); }
void FBXNode::addProperty(int32_t v) noexcept { addProperty(FBXProperty(v)); }
void FBXNode::addProperty(float v) noexcept { addProperty(FBXProperty(v)); }
void FBXNode::addProperty(double v) noexcept { addProperty(FBXProperty(v)); }
void FBXNode::addProperty(int64_t v) noexcept { addProperty(FBXProperty(v)); }
// arrays
void FBXNode::addProperty(View<bool> v) noexcept { addProperty(FBXProperty(v)); }
void FBXNode::addProperty(View<int32_t> v) noexcept { addProperty(FBXProperty(v)); }
void FBXNode::addProperty(View<float> v) noexcept { addProperty(FBXProperty(v)); }
void FBXNode::addProperty(View<double> v) noexcept { addProperty(FBXProperty(v)); }
void FBXNode::addProperty(View<int64_t> v) noexcept { addProperty(FBXProperty(v)); }
// raw / string
void FBXNode::addProperty(View<uint8_t> v) noexcept { addProperty(FBXProperty(v)); }
void FBXNode::addProperty(Text v) noexcept { addProperty(FBXProperty(v)); }

void FBXNode::addProperty(FBXProperty prop) noexcept { m_properties.push(move(prop)); }

size_t FBXNode::getBytes() const noexcept {
	size_t bytes = 13 + m_name.size();
	for (FBXNode& child : m_children) {
		bytes += child.getBytes();
	}
	for (auto &prop : m_properties) {
		bytes += prop.getBytes();
	}
	return bytes;
}

Text FBXNode::getName() const noexcept
{
    return m_name;
}
View<FBXProperty> FBXNode::getProperties() const noexcept
{
	return m_properties;
}
