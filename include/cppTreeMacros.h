#ifndef CPPTREE_MACROS_H
#define CPPTREE_MACROS_H

#include <cstdint>
#include <memory>

#ifdef CPPTREE_CUSTOM_ALLOCATOR
#define CPPTREE_ALLOCATOR CPPTREE_CUSTOM_ALLOCATOR
#else
#define CPPTREE_ALLOCATOR std::make_shared
#endif

/**
 * @def Create a constructor and a @c className::create static method for the class, with the same arguments
 * @c className::create will call the constructor through std::make_shared, with the arguments passed in @c constructorParameters
 */
#define CPPTREE_IMPL_CONSTRUCT_AND_CREATE(className, constructorArguments, constructorParameters) \
	className constructorArguments;                                                               \
	[[nodiscard]] inline static std::shared_ptr<className> create constructorArguments            \
	{                                                                                             \
		return CPPTREE_ALLOCATOR<className> constructorParameters;                                \
	}

/**
 * @def Creates a getType() and getTypeHash() override for classes inherited from BaseNode
 */
#define CPPTREE_IMPL_GET_TYPE(nodeType)                     \
	inline virtual std::string getType() const override     \
	{                                                       \
		return nodeTypeName;                                \
	}                                                       \
                                                            \
	inline virtual std::size_t getTypeHash() const override \
	{                                                       \
		return nodeType;                                    \
	}

/**
 * @def Implements a function signature for const and mutable scenarios
 * @see cpptree::Node
 */
#define CPPTREE_IMPL_DECL_FOR_MUT_CONST(sig, ret_mut, ret_const) \
	ret_mut sig;                                                 \
	ret_const sig const;

#endif // !defined(CPPTREE_MACROS_H)