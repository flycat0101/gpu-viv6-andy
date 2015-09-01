//===-- CanonicalType.h - C Language Family Type Representation -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the CanQual class template, which provides access to
//  canonical types.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_CANONICAL_TYPE_H
#define LLVM_CLANG_AST_CANONICAL_TYPE_H

#include "clang/AST/Type.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/type_traits.h"
#include <iterator>

namespace clang {

template<typename T> class CanProxy;
template<typename T> struct CanProxyAdaptor;

//----------------------------------------------------------------------------//
// Canonical, qualified type template
//----------------------------------------------------------------------------//

/// \brief Represents a canonical, potentially-qualified type.
///
/// The CanQual template is a lightweight smart pointer that provides access
/// to the canonical representation of a type, where all typedefs and other
/// syntactic sugar has been eliminated. A CanQualType may also have various
/// qualifiers (const, volatile, restrict) attached to it.
///
/// The template type parameter @p T is one of the Type classes (PointerType,
/// BuiltinType, etc.). The type stored within @c CanQual<T> will be of that
/// type (or some subclass of that type). The typedef @c CanQualType is just
/// a shorthand for @c CanQual<Type>.
///
/// An instance of @c CanQual<T> can be implicitly converted to a
/// @c CanQual<U> when T is derived from U, which essentially provides an
/// implicit upcast. For example, @c CanQual<LValueReferenceType> can be
/// converted to @c CanQual<ReferenceType>. Note that any @c CanQual type can
/// be implicitly converted to a QualType, but the reverse operation requires
/// a call to ASTContext::getCanonicalType().
///
///
template<typename T = Type>
class CanQual {
  /// \brief The actual, canonical type.
  QualType Stored;

public:
  /// \brief Constructs a NULL canonical type.
  CanQual() : Stored() { }

  /// \brief Converting constructor that permits implicit upcasting of
  /// canonical type pointers.
  template<typename U>
  CanQual(const CanQual<U>& Other,
          typename llvm::enable_if<llvm::is_base_of<T, U>, int>::type = 0);

  /// \brief Retrieve the underlying type pointer, which refers to a
  /// canonical type.
  T *getTypePtr() const { return cast_or_null<T>(Stored.getTypePtr()); }

  /// \brief Implicit conversion to a qualified type.
  operator QualType() const { return Stored; }

  /// \brief Implicit conversion to bool.
  operator bool() const { return !isNull(); }

  bool isNull() const {
    return Stored.isNull();
  }

  /// \brief Retrieve a canonical type pointer with a different static type,
  /// upcasting or downcasting as needed.
  ///
  /// The getAs() function is typically used to try to downcast to a
  /// more specific (canonical) type in the type system. For example:
  ///
  /// @code
  /// void f(CanQual<Type> T) {
  ///   if (CanQual<PointerType> Ptr = T->getAs<PointerType>()) {
  ///     // look at Ptr's pointee type
  ///   }
  /// }
  /// @endcode
  ///
  /// \returns A proxy pointer to the same type, but with the specified
  /// static type (@p U). If the dynamic type is not the specified static type
  /// or a derived class thereof, a NULL canonical type.
  template<typename U> CanProxy<U> getAs() const;

  /// \brief Overloaded arrow operator that produces a canonical type
  /// proxy.
  CanProxy<T> operator->() const;

  /// \brief Retrieve all qualifiers.
  Qualifiers getQualifiers() const { return Stored.getLocalQualifiers(); }

  /// \brief Retrieve the const/volatile/restrict qualifiers.
  unsigned getCVRQualifiers() const { return Stored.getLocalCVRQualifiers(); }

  /// \brief Determines whether this type has any qualifiers
  bool hasQualifiers() const { return Stored.hasLocalQualifiers(); }

  bool isConstQualified() const {
    return Stored.isLocalConstQualified();
  }
  bool isVolatileQualified() const {
    return Stored.isLocalVolatileQualified();
  }
  bool isRestrictQualified() const {
    return Stored.isLocalRestrictQualified();
  }

  /// \brief Determines if this canonical type is furthermore
  /// canonical as a parameter.  The parameter-canonicalization
  /// process decays arrays to pointers and drops top-level qualifiers.
  bool isCanonicalAsParam() const {
    return Stored.isCanonicalAsParam();
  }

  /// \brief Retrieve the unqualified form of this type.
  CanQual<T> getUnqualifiedType() const;

  /// \brief Retrieves a version of this type with const applied.
  /// Note that this does not always yield a canonical type.
  QualType withConst() const {
    return Stored.withConst();
  }

  /// \brief Determines whether this canonical type is more qualified than
  /// the @p Other canonical type.
  bool isMoreQualifiedThan(CanQual<T> Other) const {
    return Stored.isMoreQualifiedThan(Other.Stored);
  }

  /// \brief Determines whether this canonical type is at least as qualified as
  /// the @p Other canonical type.
  bool isAtLeastAsQualifiedAs(CanQual<T> Other) const {
    return Stored.isAtLeastAsQualifiedAs(Other.Stored);
  }

  /// \brief If the canonical type is a reference type, returns the type that
  /// it refers to; otherwise, returns the type itself.
  CanQual<Type> getNonReferenceType() const;

  /// \brief Retrieve the internal representation of this canonical type.
  void *getAsOpaquePtr() const { return Stored.getAsOpaquePtr(); }

  /// \brief Construct a canonical type from its internal representation.
  static CanQual<T> getFromOpaquePtr(void *Ptr);

  /// \brief Builds a canonical type from a QualType.
  ///
  /// This routine is inherently unsafe, because it requires the user to
  /// ensure that the given type is a canonical type with the correct
  // (dynamic) type.
  static CanQual<T> CreateUnsafe(QualType Other);

  void dump() const { Stored.dump(); }

  void Profile(llvm::FoldingSetNodeID &ID) const {
    ID.AddPointer(getAsOpaquePtr());
  }
};

template<typename T, typename U>
inline bool operator==(CanQual<T> x, CanQual<U> y) {
  return x.getAsOpaquePtr() == y.getAsOpaquePtr();
}

template<typename T, typename U>
inline bool operator!=(CanQual<T> x, CanQual<U> y) {
  return x.getAsOpaquePtr() != y.getAsOpaquePtr();
}

/// \brief Represents a canonical, potentially-qualified type.
typedef CanQual<Type> CanQualType;

inline CanQualType Type::getCanonicalTypeUnqualified() const {
  return CanQualType::CreateUnsafe(getCanonicalTypeInternal());
}

inline const DiagnosticBuilder &operator<<(const DiagnosticBuilder &DB,
                                           CanQualType T) {
  DB << static_cast<QualType>(T);
  return DB;
}

//----------------------------------------------------------------------------//
// Internal proxy classes used by canonical types
//----------------------------------------------------------------------------//

#define LLVM_CLANG_CANPROXY_TYPE_ACCESSOR(Accessor)                    \
CanQualType Accessor() const {                                           \
return CanQualType::CreateUnsafe(this->getTypePtr()->Accessor());      \
}

#define LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(Type, Accessor)             \
Type Accessor() const { return this->getTypePtr()->Accessor(); }

/// \brief Base class of all canonical proxy types, which is responsible for
/// storing the underlying canonical type and providing basic conversions.
template<typename T>
class CanProxyBase {
protected:
  CanQual<T> Stored;

public:
  /// \brief Retrieve the pointer to the underlying Type
  T* getTypePtr() const { return Stored.getTypePtr(); }

  /// \brief Implicit conversion to the underlying pointer.
  ///
  /// Also provides the ability to use canonical type proxies in a Boolean
  // context,e.g.,
  /// @code
  ///   if (CanQual<PointerType> Ptr = T->getAs<PointerType>()) { ... }
  /// @endcode
  operator const T*() const { return this->Stored.getTypePtr(); }

  /// \brief Try to convert the given canonical type to a specific structural
  /// type.
  template<typename U> CanProxy<U> getAs() const {
    return this->Stored.template getAs<U>();
  }

  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(Type::TypeClass, getTypeClass)

  // Type predicates
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isObjectType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isIncompleteType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isIncompleteOrObjectType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isPODType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isVariablyModifiedType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isIntegerType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isEnumeralType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isBooleanType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isCharType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isWideCharType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isIntegralType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isIntegralOrEnumerationType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isRealFloatingType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isComplexType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isAnyComplexType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isFloatingType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isRealType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isArithmeticType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isVoidType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isDerivedType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isScalarType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isAggregateType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isAnyPointerType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isVoidPointerType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isFunctionPointerType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isMemberFunctionPointerType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isClassType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isStructureType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isStructureOrClassType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isUnionType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isComplexIntegerType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isNullPtrType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isDependentType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isOverloadableType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isArrayType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, hasPointerRepresentation)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, hasObjCPointerRepresentation)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, hasIntegerRepresentation)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, hasSignedIntegerRepresentation)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, hasUnsignedIntegerRepresentation)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, hasFloatingRepresentation)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isPromotableIntegerType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isSignedIntegerType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isUnsignedIntegerType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isConstantSizeType)
  LLVM_CLANG_CANPROXY_SIMPLE_ACCESSOR(bool, isSpecifierType)

  /// \brief Retrieve the proxy-adaptor type.
  ///
  /// This arrow operator is used when CanProxyAdaptor has been specialized
  /// for the given type T. In that case, we reference members of the
  /// CanProxyAdaptor specialization. Otherwise, this operator will be hidden
  /// by the arrow operator in the primary CanProxyAdaptor template.
  const CanProxyAdaptor<T> *operator->() const {
    return static_cast<const CanProxyAdaptor<T> *>(this);
  }
};

/// \brief Replacable canonical proxy adaptor class that provides the link
/// between a canonical type and the accessors of the type.
///
/// The CanProxyAdaptor is a replaceable class template that is instantiated
/// as part of each canonical proxy type. The primary template merely provides
/// redirection to the underlying type (T), e.g., @c PointerType. One can
/// provide specializations of this class template for each underlying type
/// that provide accessors returning canonical types (@c CanQualType) rather
/// than the more typical @c QualType, to propagate the notion of "canonical"
/// through the system.
template<typename T>
struct CanProxyAdaptor : CanProxyBase<T> { };

/// \brief Canonical proxy type returned when retrieving the members of a
/// canonical type or as the result of the @c CanQual<T>::getAs member
/// function.
///
/// The CanProxy type mainly exists as a proxy through which operator-> will
/// look to either map down to a raw T* (e.g., PointerType*) or to a proxy
/// type that provides canonical-type access to the fields of the type.
template<typename T>
class CanProxy : public CanProxyAdaptor<T> {
public:
  /// \brief Build a NULL proxy.
  CanProxy() { }

  /// \brief Build a proxy to the given canonical type.
  CanProxy(CanQual<T> Stored) { this->Stored = Stored; }

  /// \brief Implicit conversion to the stored canonical type.
  operator CanQual<T>() const { return this->Stored; }
};

} // end namespace clang

namespace llvm {

/// Implement simplify_type for CanQual<T>, so that we can dyn_cast from
/// CanQual<T> to a specific Type class. We're prefer isa/dyn_cast/cast/etc.
/// to return smart pointer (proxies?).
template<typename T>
struct simplify_type<const ::clang::CanQual<T> > {
  typedef T* SimpleType;
  static SimpleType getSimplifiedValue(const ::clang::CanQual<T> &Val) {
    return Val.getTypePtr();
  }
};
template<typename T>
struct simplify_type< ::clang::CanQual<T> >
: public simplify_type<const ::clang::CanQual<T> > {};

// Teach SmallPtrSet that CanQual<T> is "basically a pointer".
template<typename T>
class PointerLikeTypeTraits<clang::CanQual<T> > {
public:
  static inline void *getAsVoidPointer(clang::CanQual<T> P) {
    return P.getAsOpaquePtr();
  }
  static inline clang::CanQual<T> getFromVoidPointer(void *P) {
    return clang::CanQual<T>::getFromOpaquePtr(P);
  }
  // qualifier information is encoded in the low bits.
  enum { NumLowBitsAvailable = 0 };
};

} // end namespace llvm


#endif // LLVM_CLANG_AST_CANONICAL_TYPE_H
