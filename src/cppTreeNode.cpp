#include "cppTreeNode.h"

#include <algorithm>
#include <functional>
#include <queue>

// clang-format off
#define CPPTREE_IMPL_DEF_FOR_MULTIPLE_SIGNATURES(sig1, sig2, impl) \
	sig1 impl                                                      \
	sig2 impl
// clang-format on

namespace cpptree {

#pragma region BaseNode

/* protected virtual */ bool BaseNode::isValidParent(const BaseNode *parent) const
{
	return parent != this;
}

/* protected */ bool BaseNode::addChild(std::shared_ptr<BaseNode> newChild)
{
	if (!newChild->isValidParent(this))
		return false;

	if (std::find_if(m_children.begin(), m_children.end(), [newChild](const auto &child) {
		    return child->m_nameHash == newChild->m_nameHash;
	    }) != m_children.end()) {
		return false;
	}

	// children rely on the parent's resources, so it takes precedence
	onChildChange(Change::ADD, newChild);
	newChild->onParentChange(Change::ADD, this);

	if (newChild->m_parent)
		newChild->m_previousParents.push_back(newChild->m_parent);

	newChild->m_parent = this;

	m_children.push_back(newChild);

	propagateSubChildChange(Change::ADD, newChild);

	return true;
}

/* protected */ bool BaseNode::removeChild(const std::string &name)
{
	const auto nameHash = std::hash<std::string>{}(name);
	const auto localNode = std::find_if(m_children.begin(), m_children.end(), [nameHash](const auto &child) {
		return child->m_nameHash == nameHash;
	});

	if (localNode != m_children.end()) {
		auto &prevParents = (*localNode)->m_previousParents;
		if ((*localNode)->m_parent == this) {
			(*localNode)->m_parent = (!prevParents.empty()) ? prevParents.back() : nullptr;

			if (!prevParents.empty())
				prevParents.pop_back();
		}
		else {
			auto amongParents = std::find(prevParents.begin(), prevParents.end(), this);

			if (amongParents != prevParents.end())
				prevParents.erase(amongParents);
			else
				return false;
		}

		onChildChange(Change::REMOVE, *localNode);
		(*localNode)->onParentChange(Change::REMOVE, this);

		propagateSubChildChange(Change::REMOVE, *localNode);

		m_children.erase(localNode);
		return true;
	}

	return false;
}

/* protected */ bool BaseNode::removeChild(const std::shared_ptr<BaseNode> &node)
{
	if (node->m_parent == this) {
		node->m_parent = (node->m_previousParents.size() > 0) ? node->m_previousParents.back() : nullptr;

		if (node->m_previousParents.size() > 0)
			node->m_previousParents.pop_back();
	}
	else {
		auto amongParents = std::find(node->m_previousParents.begin(), node->m_previousParents.end(), this);

		if (amongParents != node->m_previousParents.end())
			node->m_previousParents.erase(amongParents);
		else
			return false;
	}

	onChildChange(Change::REMOVE, node);
	node->onParentChange(Change::REMOVE, this);

	propagateSubChildChange(Change::REMOVE, node);

	m_children.erase(std::find(m_children.begin(), m_children.end(), node));
	return true;
}

/* protected */ bool BaseNode::signalChild(const std::string &name, const std::string &signal)
{
	const auto nameHash = std::hash<std::string>{}(name);
	const auto localNode = std::find_if(m_children.begin(), m_children.end(), [nameHash](const auto &child) {
		return child->m_nameHash == nameHash;
	});

	if (localNode != m_children.end()) {
		(*localNode)->onSignal(signal, this);
		return true;
	}

	return false;
}

/* private */ void BaseNode::propagateSubChildChange(Change type, const std::shared_ptr<BaseNode> &child)
{
	if (m_parent) {
		m_parent->onSubChildChange(type, child);
		m_parent->propagateSubChildChange(type, child);
	}

	for (const auto &parent : m_previousParents) {
		parent->onSubChildChange(type, child);
		parent->propagateSubChildChange(type, child);
	}
}

BaseNode::BaseNode(const std::string &name)
    : m_children(), m_name(name), m_nameHash(), m_parent(nullptr), m_previousParents()
{
	auto slashIterator = m_name.find('/');

	while (slashIterator != std::string::npos) {
		m_name = m_name.replace(m_name.begin() + slashIterator, m_name.begin() + slashIterator + 1, "_");
		slashIterator = m_name.find('/', slashIterator + 1);
	}

	m_nameHash = std::hash<std::string>{}(name);
}

/* virtual */ BaseNode::~BaseNode()
{
	for (auto &child : m_children) {
		auto &prevParents = child->m_previousParents;
		if (child->m_parent == this) {
			child->m_parent = (prevParents.size() > 0) ? prevParents.back() : nullptr;

			if (prevParents.size() > 0)
				prevParents.pop_back();
		}
		else {
			auto amongParents = std::find(prevParents.begin(), prevParents.end(), this);

			if (amongParents != prevParents.end())
				prevParents.erase(amongParents);
		}

		child->onParentChange(Change::REMOVE, this);
	}
}

CPPTREE_IMPL_DEF_FOR_MULTIPLE_SIGNATURES(
    std::shared_ptr<BaseNode> BaseNode::getNodeByPath(const std::string &path),
    std::shared_ptr<const BaseNode> BaseNode::getNodeByPath(const std::string &path) const,
    {
	    const auto firstSlash = path.find('/');

	    if (firstSlash == std::string::npos) {
		    const auto containingChild = std::find_if(m_children.begin(), m_children.end(), [path](const auto &child) {
			    return path == child->m_name;
		    });

		    if (containingChild == m_children.end())
			    return nullptr;
		    else
			    return *containingChild;
	    }
	    else {
		    const auto containerName = path.substr(0, firstSlash);
		    const auto containerNameHash = std::hash<std::string>{}(containerName);

		    const auto containingChild = std::find_if(m_children.begin(), m_children.end(), [containerNameHash](const auto &child) {
			    return containerNameHash == child->m_nameHash;
		    });

		    if (containingChild == m_children.end())
			    return nullptr;
		    else
			    return (*containingChild)->getNodeByPath(path.substr(firstSlash + 1));
	    }
    })

CPPTREE_IMPL_DEF_FOR_MULTIPLE_SIGNATURES(
    std::vector<std::shared_ptr<BaseNode>> BaseNode::getNodesByName(const std::string &name, unsigned int depth),
    std::vector<std::shared_ptr<const BaseNode>> BaseNode::getNodesByName(const std::string &name, unsigned int depth) const,
    {
	    return getNodesByNameHash(std::hash<std::string>{}(name));
    })

CPPTREE_IMPL_DEF_FOR_MULTIPLE_SIGNATURES(
    std::vector<std::shared_ptr<BaseNode>> BaseNode::getNodesByNameHash(std::size_t nameHash, unsigned int depth),
    std::vector<std::shared_ptr<const BaseNode>> BaseNode::getNodesByNameHash(std::size_t nameHash, unsigned int depth) const,
    {
	    using return_type = decltype(getNodesByNameHash(nameHash, depth));
	    if (depth == 0) {
		    return {};
	    }
	    else {
		    return_type result = {};
		    for (auto child : m_children) {
			    if (child->m_nameHash == nameHash)
				    result.push_back(child);

			    auto matchesOfChild = child->getNodesByNameHash(nameHash, depth - 1);
			    result.insert(result.end(), matchesOfChild.begin(), matchesOfChild.end());
		    }

		    return result;
	    }
    })

CPPTREE_IMPL_DEF_FOR_MULTIPLE_SIGNATURES(
    std::vector<std::shared_ptr<BaseNode>> BaseNode::getNodesByType(const std::string &type, unsigned int depth),
    std::vector<std::shared_ptr<const BaseNode>> BaseNode::getNodesByType(const std::string &type, unsigned int depth) const,
    {
	    using return_type = decltype(getNodesByType(type, depth));
	    if (depth == 0) {
		    return {};
	    }
	    else {
		    return_type result = {};

		    for (auto &child : m_children) {
			    if (child->getType() == type)
				    result.push_back(child);

			    auto matchesOfChild = child->getNodesByType(type, depth - 1);
			    result.insert(result.end(), matchesOfChild.begin(), matchesOfChild.end());
		    }

		    return result;
	    }
    })

CPPTREE_IMPL_DEF_FOR_MULTIPLE_SIGNATURES(
    std::vector<std::shared_ptr<BaseNode>> BaseNode::getNodesByTypeHash(std::size_t typeHash, unsigned int depth),
    std::vector<std::shared_ptr<const BaseNode>> BaseNode::getNodesByTypeHash(std::size_t typeHash, unsigned int depth) const,
    {
	    using return_type = decltype(getNodesByTypeHash(typeHash, depth));
	    if (depth == 0) {
		    return {};
	    }
	    else {
		    return_type result = {};

		    for (auto &child : m_children) {
			    if (child->getTypeHash() == typeHash)
				    result.push_back(child);

			    auto matchesOfChild = child->getNodesByTypeHash(typeHash, depth - 1);
			    result.insert(result.end(), matchesOfChild.begin(), matchesOfChild.end());
		    }

		    return result;
	    }
    })

CPPTREE_IMPL_DEF_FOR_MULTIPLE_SIGNATURES(
    std::shared_ptr<BaseNode> BaseNode::getNodeByNameHash(std::size_t nameHash),
    std::shared_ptr<BaseNode> BaseNode::getNodeByNameHash(std::size_t nameHash) const,
    {
	    for (const auto &child : m_children)
		    if (child->m_nameHash == nameHash)
			    return child;

	    return nullptr;
    })

std::size_t BaseNode::countNodes(unsigned int depth) const
{
	std::size_t result = m_children.size();

	if (depth > 0)
		for (const auto child : m_children)
			result += child->countNodes(depth - 1);

	return result;
}

std::size_t BaseNode::countParents() const
{
	return m_previousParents.size() + static_cast<std::size_t>(m_parent != nullptr);
}

std::string BaseNode::getPath() const
{
	using namespace std::string_literals;

	std::string result = m_name;

	BaseNode *parentptr = m_parent;
	while (parentptr != nullptr) {
		result.insert(0, parentptr->m_name + "/"s);
		parentptr = parentptr->m_parent;
	}

	return result;
}

std::vector<std::string> BaseNode::getAllPaths() const
{
	using namespace std::string_literals;

	auto result = std::vector<std::string>(countParents());
	auto resultInserter = result.begin();

	for (const auto parent : m_previousParents) {
		(*resultInserter++) = parent->getPath() + "/"s + m_name;
	}

	if (m_parent)
		(*resultInserter) = getPath();

	return result;
}

std::string BaseNode::getTree(bool includeTypes, unsigned int initialIndent, unsigned int levelIndent, unsigned int depth) const
{
	using namespace std::string_literals;

	std::string result = std::string(initialIndent, ' ') + "- "s + m_name + (includeTypes ? " : "s + getType() + "\n"s : "\n"s);

	if (depth == 0)
		result += std::string(initialIndent + levelIndent, ' ') + "- <...>\n"s;
	else
		for (const auto child : m_children)
			result += child->getTree(includeTypes, initialIndent + levelIndent, levelIndent, depth - 1);

	return result;
}

/* virtual */ std::string BaseNode::toString() const
{
	using namespace std::string_literals;
	return "<CPPTREE::BaseNode name=\"s + m_name + \">"s;
}

// BaseNode
#pragma endregion

#pragma region Node

Node::Node(const std::string &name)
    : BaseNode(name)
{
}

bool Node::addLocalNode(std::shared_ptr<BaseNode> node)
{
	return addChild(node);
}

bool Node::addNode(const std::string &path, std::shared_ptr<BaseNode> node)
{
	if (path == "" || path == std::string()) {
		return addLocalNode(node);
	}
	else {
		auto parent = std::dynamic_pointer_cast<Node>(getNodeByPath(path));

		if (!parent)
			return false;

		return parent->addChild(node);
	}
}

bool Node::removeLocalNode(const std::string &name)
{
	return removeChild(name);
}

bool Node::removeLocalNode(const std::shared_ptr<BaseNode> &node)
{
	return removeChild(node);
}

// Node
#pragma endregion

#pragma region RestrictiveNode

RestrictiveNode::RestrictiveNode(const std::string &name, std::vector<std::string> addtype, std::vector<std::string> remtype)
    : Node(name), m_settings{addtype, remtype}
{
}

/* virtual */ bool RestrictiveNode::addLocalNode(std::shared_ptr<BaseNode> node) /* override */
{
	if (!node)
		return false;
	return (std::count(m_settings.allow_addtype.begin(), m_settings.allow_addtype.end(), node->getType()) == 0)
	           ? false
	           : Node::addLocalNode(node);
}

/* virtual */ bool RestrictiveNode::removeLocalNode(const std::string &name) /* override */
{
	const auto nameHash = std::hash<std::string>{}(name);
	auto childIterator = std::find_if(m_children.begin(), m_children.end(), [nameHash](const auto &child) {
		return child->m_nameHash == nameHash;
	});

	if (childIterator != m_children.end()) {
		if (std::count(m_settings.allow_remtype.begin(), m_settings.allow_remtype.end(), (*childIterator)->getType()) == 0) {
			return false;
		}
		else {
			m_children.erase(childIterator);
			return true;
		}
	}

	return false;
}

/* virtual */ bool RestrictiveNode::removeLocalNode(const std::shared_ptr<BaseNode> &node) /* override */
{
	return (std::count(m_settings.allow_remtype.begin(), m_settings.allow_remtype.end(), node->getType()) == 0)
	           ? false
	           : Node::removeLocalNode(node);
}

// RestrictiveNode
#pragma endregion

} // namespace cpptree
