#pragma once

#include <libwml/mpl.hpp>
#include <libwml/foreach.hpp>
#include <libwml/traits/tag.hpp>
#include <libwml/visit_tag.hpp>
#include <libwml/heterogenous_sequence.hpp>
#include <libwml/wml.hpp>
#include <libwml/coercer.hpp>
#include <libwml/coerce_log.hpp>
#include <type_traits>

namespace wml {

// Used to provide this_tag symbol within tags
template <typename T>
struct tag_base {
  using this_tag = T;
};


// Used when declaring member list using a macro
namespace mpl {

template <typename ignored, typename... Ts>
using cdr_list = mpl::typelist<Ts...>;

} // end namespace mpl

// Used when specializing the tag trait

namespace traits {

template <typename T>
struct tag_trait_base : std::true_type {
  static void coerce(T & t, const wml::config & c, coerce_log * log) {
    coercer v{c, log};

    wml::visit_tag(t, v);

    v.report_unused();
  }
};

} // end namespace traits
} // end namespace wml


// Note: In below, the _CHOOOSER macros are used to make "overloaded" macros
// that do different things with different numbers of arguments.

// Macros which assemble a WML tag definition

#define TAG_MEMBER_INFO_NAME(NAME) tag_member_info_##NAME##_t

#define MAKE_DECLARATION_IMPL(TYPE, NAME, HAS_DEFAULT)                         \
TYPE NAME ;                                                                    \
struct TAG_MEMBER_INFO_NAME(NAME) : tag_member_base<TAG_MEMBER_INFO_NAME(NAME),              \
                                                    this_tag, TYPE, &this_tag::NAME, HAS_DEFAULT> { \
  static constexpr const char * name() { return #NAME; }                       \
};

#define MAKE_DECLARATION_THREE(TYPE, NAME, DEF)                                \
MAKE_DECLARATION_IMPL(TYPE, NAME, true)

#define MAKE_DECLARATION_TWO(TYPE, NAME)                                       \
MAKE_DECLARATION_IMPL(TYPE, NAME, false)

#define MAKE_DECLARATION_CHOOSER(...) FOURTH(__VA_ARGS__, MAKE_DECLARATION_THREE, MAKE_DECLARATION_TWO, _, _)
#define MAKE_DECLARATION(...) MAKE_DECLARATION_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

// Extra comma at end of MAKE_INITIALIZER ensures that even if DEF is missing,
// we just use empty token for it, which maps to default initialization.

#define MAKE_INITIALIZER_IMPL(TYPE, NAME, DEF, ...) , NAME(DEF)
#define MAKE_INITIALIZER(...) MAKE_INITIALIZER_IMPL(__VA_ARGS__,)

// Used to make the "members" list in wml::traits::tag
#define MAKE_META_LIST(TAG, TYPE, NAME, ...) , wml::TAG::TAG_MEMBER_INFO_NAME(NAME)

// Specializes the trait which associates a name to each tag struct

#define DECLARE_TAG_NAME(TAG, NAME)                                            \

// Forward facing macros

#define DECLARE_WML_TAG(TAG, SEQ)                                              \
  DECLARE_WML_TAG_AND_NAME(TAG, #TAG, SEQ)


#define DECLARE_WML_TAG_AND_NAME(TAG, NAME, SEQ)                               \
  namespace wml {                                                              \
    struct TAG : tag_base<TAG> {                                               \
      FOR_EACH(MAKE_DECLARATION, SEQ)                                          \
      TAG() : tag_base<TAG>() FOR_EACH(MAKE_INITIALIZER, SEQ) {}               \
    };                                                                         \
  }                                                                            \
namespace wml {                                                                \
namespace traits {                                                             \
template <>                                                                    \
struct tag<wml::TAG> : tag_trait_base<wml::TAG> {                              \
  static constexpr const char * name() { return NAME ; }                       \
  using members = mpl::cdr_list<void FOR_EACH_DATA(MAKE_META_LIST, TAG, SEQ)>; \
};                                                                             \
}                                                                              \
}                                                                              \
static_assert(true, "")


// MAKE_HETEROGENOUS_SEQUENCE is a legacy macro. I dimly remember that it was
// necessary that the same tag may sometimes be parsed with a different name,
// and then heterogenous sequence should be a list of types and strings, not
// just a list of types...
//
// For now I want to leave all the strings in there, and just strip them out
// using macros. If it turns out that the strings are really unnecessary then
// they will be removed and this can be simplified.

namespace wml {
  struct hs_dummy {};

  template <typename TL>
  struct make_heterogenous_sequence;

  template <typename... Ts>
  struct make_heterogenous_sequence<mpl::typelist<hs_dummy, Ts...>> {
    using type = heterogenous_sequence<Ts...>;
  };
} // end namespace wml

#define FETCH_TYPE(TYPE, NAME) , TYPE

#define MAKE_HETEROGENOUS_SEQUENCE(NAME, SEQ)                                  \
namespace wml {                                                                \
  using NAME = typename make_heterogenous_sequence< mpl::typelist< hs_dummy    \
                                      FOR_EACH(FETCH_TYPE, SEQ)                \
                                                    > >::type;                 \
}
