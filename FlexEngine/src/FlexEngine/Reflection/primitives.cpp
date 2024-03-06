#include "Reflection/base.h"

/// <summary>
/// TypeDescriptor for primitive types
/// <para>Abstracted for easy editing</para>
/// </summary>
#define TYPE_DESC(NAME, TYPE) \
  struct TypeDescriptor_##NAME : TypeDescriptor \
  { \
    TypeDescriptor_##NAME() : TypeDescriptor{ #TYPE, sizeof(TYPE) } {} \
    virtual void Dump(const void* obj, int, std::ostream& os) const override \
    { \
      os << #TYPE << "{" << *(const TYPE*)obj << "}"; \
    } \
    virtual void Serialize(const void* obj, std::ostream& os) const override \
    { \
      os << #TYPE << "{" << *(const TYPE*)obj << "}"; \
    } \
  }; \
  template <> TypeDescriptor* GetPrimitiveDescriptor<TYPE>() \
  { \
    static TypeDescriptor_##NAME type_desc; return &type_desc; \
  }

namespace FlexEngine
{

  namespace Reflection
  {
    // todo: add all primitive types

    TYPE_DESC(Int, int)
    TYPE_DESC(Float, float)
    TYPE_DESC(Double, double)
    TYPE_DESC(StdString, std::string)

  }

}