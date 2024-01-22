#include "cppTreeNode.h"

#include <catch2/catch_all.hpp>
#include <cstdint>

class TestNode : public cpptree::BaseNode {
public:
	struct TestFields {
		std::uint8_t name : 1;
		std::uint8_t parent_change : 1;
	} fields;

	std::string parentName;
	std::string parentType;

public:
	TestNode(const std::string &name,
	         const std::string &i_parentName, const std::string &i_parentType)
	    : BaseNode(name), fields(),
	      parentName(i_parentName), parentType(i_parentType)
	{
		fields.name = getName() == name;
	}

	void addDummyChild(const std::string &name)
	{
		addChild(cpptree::BaseNode::create(name));
	}

protected:
	inline virtual bool isValidParent(const BaseNode *parent) const final
	{
		return true;
	}

	inline virtual void onParentChange(cpptree::BaseNode::Change type, const cpptree::BaseNode *newParent) final
	{
		fields.parent_change = type == Change::ADD && newParent->getName() == parentName && newParent->getType() == parentType;
	}
};

TEST_CASE("cpptree", "[cpptree]")
{
	cpptree::NodePtr ptr;
	std::string parentName = "test1";
	SECTION("construction")
	{
		ptr = cpptree::Node::create(parentName);

		REQUIRE(ptr != nullptr);
		REQUIRE(ptr->getName() == parentName);
		REQUIRE(ptr->getName_c() == parentName);
		REQUIRE(ptr->getType() == std::string(cpptree::Node::nodeTypeName));
		REQUIRE(ptr->getTypeHash() == cpptree::Node::nodeType);
		REQUIRE(ptr->getChildren_c().empty());
		SECTION("special character removal from names")
		{
			std::string name = "test1/test2/test3";
			auto childPtr = cpptree::BaseNode::create(name);

			REQUIRE(childPtr->getName() == "test1_test2_test3");
		}

		std::string childName = "test4";

		SECTION("adding children")
		{
			auto childPtr = std::make_shared<TestNode>(childName, parentName, cpptree::Node::nodeTypeName);
			ptr->addLocalNode(childPtr);

			REQUIRE(childPtr->fields.name);
			REQUIRE(childPtr->fields.parent_change);

			SECTION("preserving children, getNodeByPath")
			{
				REQUIRE(ptr->getNodeByPath(childName) != nullptr);

				ptr->getNodeByPath<TestNode>(childName)->addDummyChild("test5");
				REQUIRE(ptr->getNodeByPath(std::string(childName) + "/test5") != nullptr);
			}
		}
	}

	//! @todo
}
