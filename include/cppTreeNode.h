#ifndef CPPTREE_NODE_H
#define CPPTREE_NODE_H

#include "cppTreeMacros.h"

#include <deque>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace cpptree {
/** @brief Base class for representing a basic named node structure */
class BaseNode {
	friend class Node;
	friend class RestrictiveNode;

protected:
	enum class Change {
		ADD,
		REMOVE
	};

protected:
	std::vector<std::shared_ptr<BaseNode>> m_children;
	std::string m_name;
	std::size_t m_nameHash;

	std::deque<BaseNode *> m_previousParents;
	BaseNode *m_parent;

protected:
	//! @brief Called when a parent is about to be assigned
	inline virtual void onParentChange(Change type, const BaseNode *newParent) {}

	//! @brief Called when a child is added to the list
	inline virtual void onChildChange(Change type, const std::shared_ptr<BaseNode> &child) {}

	//! @brief Called when a child is added to any sub-element
	inline virtual void onSubChildChange(Change type, const std::shared_ptr<BaseNode> &child) {}

	//! @brief Special virtual function for handling user-made signals
	inline virtual void onSignal(const std::string &sig, const BaseNode *parent) {}

	//! @brief Called to check whether a node can get the given parent
	inline virtual bool isValidParent(const BaseNode *parent) const;

protected:
	//! @brief Adds a child to the list of children, calls the appropriate callbacks
	bool addChild(std::shared_ptr<BaseNode> newChild);

	//! @brief Removes a child from the list of children, calls the appropriate callbacks
	bool removeChild(const std::string &name);

	//! @brief Removes a child from the list of children, calls the appropriate callbacks
	bool removeChild(const std::shared_ptr<BaseNode> &node);

	//! @brief Call the onSignal handler of a child with the given name
	bool signalChild(const std::string &name, const std::string &signal);

private:
	void propagateSubChildChange(Change type, const std::shared_ptr<BaseNode> &child);

public:
	CPPTREE_IMPL_CONSTRUCT_AND_CREATE(
	    BaseNode,
	    (const std::string &name),
	    (name));

	BaseNode(const BaseNode &other) = delete;
	virtual ~BaseNode();

	inline virtual std::string getType() const { return nodeTypeName; }
	inline virtual std::size_t getTypeHash() const { return nodeType; }

	virtual std::string toString() const;

	//! @brief Tries to return a node by the path provided, otherwise returns nullptr
	CPPTREE_IMPL_DECL_FOR_MUT_CONST(
	    getNodeByPath(const std::string &path),
	    std::shared_ptr<BaseNode>,
	    std::shared_ptr<const BaseNode>);

	//! @brief Returns all the nodes in the tree with the given name, `depth` layers deep
	CPPTREE_IMPL_DECL_FOR_MUT_CONST(
	    getNodesByName(const std::string &name, unsigned int depth = (~0)),
	    std::vector<std::shared_ptr<BaseNode>>,
	    std::vector<std::shared_ptr<const BaseNode>>);

	//! @brief Returns all the nodes in the tree with the given name hash, `depth` layers deep
	CPPTREE_IMPL_DECL_FOR_MUT_CONST(
	    getNodesByNameHash(std::size_t nameHash, unsigned int depth = (~0)),
	    std::vector<std::shared_ptr<BaseNode>>,
	    std::vector<std::shared_ptr<const BaseNode>>);

	//! @brief Returns all the nodes in the tree with the given type, `depth` layers deep
	CPPTREE_IMPL_DECL_FOR_MUT_CONST(
	    getNodesByType(const std::string &type, unsigned int depth = (~0)),
	    std::vector<std::shared_ptr<BaseNode>>,
	    std::vector<std::shared_ptr<const BaseNode>>);

	//! @brief Returns all the nodes in the tree with the given type, `depth` layers deep
	CPPTREE_IMPL_DECL_FOR_MUT_CONST(
	    getNodesByTypeHash(std::size_t typeHash, unsigned int depth = (~0)),
	    std::vector<std::shared_ptr<BaseNode>>,
	    std::vector<std::shared_ptr<const BaseNode>>);

	//! @brief Returns a node with a given name hash, or nullptr
	CPPTREE_IMPL_DECL_FOR_MUT_CONST(
	    getNodeByNameHash(std::size_t nameHash),
	    std::shared_ptr<BaseNode>,
	    std::shared_ptr<BaseNode>);

	template <typename T, typename = std::enable_if_t<std::is_base_of_v<BaseNode, T>>>
	//! @brief Returns a node with a given name, or nullptr
	std::shared_ptr<T> getNodeByName(const std::string &name)
	{
		return std::dynamic_pointer_cast<T>(getNodeByNameHash(std::hash<std::string>{}(name)));
	}

	template <typename T, typename = std::enable_if_t<std::is_base_of_v<BaseNode, T>>>
	//! @brief Returns a node with a given name, or nullptr
	std::shared_ptr<T> getNodeByName(const std::string &name) const
	{
		return std::dynamic_pointer_cast<T>(getNodeByNameHash(std::hash<std::string>{}(name)));
	}

	template <typename T, typename = std::enable_if_t<std::is_base_of_v<BaseNode, T>>>
	//! @brief Tries to return a node by the path provided, otherwise returns nullptr
	std::shared_ptr<T> getNodeByPath(const std::string &path)
	{
		return std::dynamic_pointer_cast<T>(getNodeByPath(path));
	}

	template <typename T, typename = std::enable_if_t<std::is_base_of_v<BaseNode, T>>>
	//! @brief Tries to return a node by the path provided, otherwise returns nullptr
	std::shared_ptr<const T> getNodeByPath(const std::string &path) const
	{
		return std::dynamic_pointer_cast<T>(getNodeByPath(path));
	}

	template <typename T, typename = std::enable_if_t<std::is_base_of_v<BaseNode, T>>>
	//! @brief Returns the current node as a T pointer, if T inherits BaseNode
	T *as()
	{
		return dynamic_cast<T *>(this);
	}

	template <typename T, typename = std::enable_if_t<std::is_base_of_v<BaseNode, T>>>
	//! @brief Returns the current node as a T pointer, if T inherits BaseNode
	const T *as() const
	{
		return dynamic_cast<const T *const>(this);
	}

	//! @brief Counts all the nodes `depth` layers deep
	std::size_t countNodes(unsigned int depth = (~0)) const;
	std::size_t countParents() const;

	std::string getPath() const;
	std::vector<std::string> getAllPaths() const;

	inline std::string getName() const { return m_name; }
	inline const std::string &getName_c() const { return m_name; }
	inline std::vector<std::shared_ptr<BaseNode>> getChildren() const { return m_children; }
	inline const std::vector<std::shared_ptr<BaseNode>> &getChildren_c() const { return m_children; }

	/**
	 * @brief Returns a string representation of the node tree
	 *
	 * @param includeTypes Whether to include types in the tree
	 * @param initialIndent Base indentation for all lines
	 * @param levelIndent Number of extra spaces for each new level
	 * @param depth Number of layers to display
	 */
	std::string getTree(bool includeTypes = false, unsigned int initialIndent = 0, unsigned int levelIndent = 2, unsigned int depth = (~0)) const;

	constexpr static const std::size_t nodeType = 0;
	constexpr static const char *nodeTypeName = "BaseNode";
};

/** @brief An extension to the BaseNode class exposing node adding and removing functionality */
class Node : public BaseNode {
public:
	CPPTREE_IMPL_CONSTRUCT_AND_CREATE(
	    Node,
	    (const std::string &name),
	    (name));

	//! @brief Adds a node to the current node
	virtual bool addLocalNode(std::shared_ptr<BaseNode> node);

	//! @brief Adds a node at a specified path - path "" is the same as calling addLocalNode
	virtual bool addNode(const std::string &path, std::shared_ptr<BaseNode> node);

	template <typename T, typename = std::enable_if_t<std::is_base_of_v<BaseNode, T>>>
	//! @brief Adds a node to the current node
	bool addLocalNode(std::shared_ptr<T> node)
	{
		return addLocalNode(std::dynamic_pointer_cast<BaseNode>(node));
	}

	template <typename T, typename = std::enable_if_t<std::is_base_of_v<BaseNode, T>>>
	//! @brief Adds a node at a specified path - path "" is the same as calling addLocalNode
	bool addNode(const std::string &path, std::shared_ptr<T> node)
	{
		return addNode(path, std::dynamic_pointer_cast<BaseNode>(node));
	}

	//! @brief Tries to remove a local node with the name provided
	virtual bool removeLocalNode(const std::string &name);

	//! @brief Tries to remove a local node, fails if the node is not a child of the current node
	virtual bool removeLocalNode(const std::shared_ptr<BaseNode> &node);

	CPPTREE_IMPL_GET_TYPE(nodeType);

	constexpr static const std::size_t nodeType = 1;
	constexpr static const char *nodeTypeName = "Node";
};

/** @brief Class restricting adding and removing of nodes to certain node types */
class RestrictiveNode : public Node {
protected:
	struct {
		std::vector<std::string> allow_addtype;
		std::vector<std::string> allow_remtype;
	} m_settings;

public:
	CPPTREE_IMPL_CONSTRUCT_AND_CREATE(
	    RestrictiveNode,
	    (const std::string &name, std::vector<std::string> addtype = {}, std::vector<std::string> remtype = {}),
	    (name, addtype, remtype));

	virtual bool addLocalNode(std::shared_ptr<BaseNode> node) override;

	virtual bool removeLocalNode(const std::string &name) override;
	virtual bool removeLocalNode(const std::shared_ptr<BaseNode> &node) override;
};

using BaseNodePtr = std::shared_ptr<BaseNode>;
using NodePtr = std::shared_ptr<Node>;
using RestrictiveNodePtr = std::shared_ptr<RestrictiveNode>;

} // namespace cpptree

#endif // !defined(CPPTREE_NODE_H)