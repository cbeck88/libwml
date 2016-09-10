#pragma once

#include <type_traits>
#include <vector>

#include <libwml/coerce_log.hpp>
#include <libwml/traits/attribute.hpp>
#include <libwml/traits/child_container.hpp>
#include <libwml/traits/tag.hpp>
#include <libwml/util/optional.hpp>
#include <libwml/wml.hpp>

namespace wml {

struct coercer {
  const wml::config & cfg_;
  coerce_log * log_;

  std::vector<char> used_;

  explicit coercer(const wml::config & c, coerce_log * l)
    : cfg_(c)
    , log_(l)
    , used_(cfg_.size()) {}

  // Attribute parsing
  template <typename T>
  std::enable_if_t<traits::attribute<T>::value> operator()(const char * key, T & value) {
    for (int idx = 0; idx < cfg_.size(); ++idx) {
      if (!used_[idx]) {
        if (const wml::Pair * p = util::get<wml::Pair>(&cfg_[idx])) {
          if (p->first == key) {
            if (auto maybe_error = traits::attribute<T>::coerce(value, p->second)) {
              if (log_) { log_->report_attribute_fail<T>(key, p->second, *maybe_error); }
            }
            used_[idx] = true;
            return;
          }
        }
      }
    }

    if (log_) { log_->report_attribute_fail<T>(key, "(none)", "Attribute not found!"); }
  }

  // With default value
  template <typename T, typename D>
  std::enable_if_t<traits::attribute<T>::value> operator()(const char * key, T & value,
                                                           D && default_value_func) {
    for (int idx = 0; idx < cfg_.size(); ++idx) {
      if (!used_[idx]) {
        if (const wml::Pair * p = util::get<wml::Pair>(&cfg_[idx])) {
          if (p->first == key) {
            if (auto maybe_error = traits::attribute<T>::coerce(value, p->second)) {
              if (log_) { log_->report_attribute_fail<T>(key, p->second, *maybe_error); }
            }
            used_[idx] = true;
            return;
          }
        }
      }
    }

    value = default_value_func();
  }

  // Optional attribute (if no explicit default, treat as though it has one)
  template <typename T, typename D>
  std::enable_if_t<traits::attribute<T>::value> operator()(const char * key,
                                                           util::optional<T> & value) {
    (*this)(key, value, []() -> util::optional<T> { return {}; });
  }

  // (Individual) child tag
  template <typename T>
  std::enable_if_t<traits::tag<T>::value> operator()(const char * key, T & value) {
    for (int idx = 0; idx < cfg_.size(); ++idx) {
      if (!used_[idx]) {
        if (const wml::body * b = util::get<wml::body>(&cfg_[idx])) {
          if (key == b->name) {
            log_context c{log_, key};

            traits::tag<T>::coerce(value, *b, log_);

            used_[idx] = true;
            return;
          }
        }
      }
    }

    if (log_) { log_->report_child_missing<T>(key, "Mandatory child not found!"); }
  }

  // optional child
  template <typename T>
  std::enable_if_t<traits::tag<T>::value> operator()(const char * key, util::optional<T> & value) {
    for (int idx = 0; idx < cfg_.size(); ++idx) {
      if (!used_[idx]) {
        if (const wml::body * b = util::get<wml::body>(&cfg_[idx])) {
          if (key == b->name) {
            log_context c{log_, key};

            value.emplace();
            traits::tag<T>::coerce(*value, *b, log_);

            used_[idx] = true;
            return;
          }
        }
      }
    }
  }

  // recursive_wrapper
  template <typename T>
  std::enable_if_t<traits::tag<T>::value> operator()(const char * key,
                                                     util::recursive_wrapper<T> & value) {
    (*this)(key, value.get());
  }

  // Child container
  template <typename T>
  std::enable_if_t<traits::child_container<T>::value> operator()(const char * key, T & value) {
    for (int idx = 0; idx < cfg_.size(); ++idx) {
      if (!used_[idx]) {
        if (const wml::body * b = util::get<wml::body>(&cfg_[idx])) {
          if (traits::child_container<T>::allows_tag(*b)) {
            traits::child_container<T>::insert_tag(value, *b, log_);
            used_[idx] = true;
            return;
          }
        }
      }
    }
  }

  void report_unused() const {
    for (int idx = 0; idx < cfg_.size(); ++idx) {
      if (!used_[idx]) {
        if (log_) { log_->report_unused(cfg_[idx]); }
      }
    }
  }
};

} // end namespace wml
