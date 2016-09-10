#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright © 2016 Chris Beck
# This work is free. You can redistribute it and/or modify it under the
# terms of the Do What The Fuck You Want To Public License, Version 2,
# as published by Sam Hocevar. See the LICENSE file for more details.
#
# Description:
# generate.py takes an XML specification and constructs from it a C++ header.
#
# It's purpose is to create a struct type for each WML tag.
# This struct has corresponding members for each attribute, and "container"
# members for each permitted type of child tag. (Several possibilities here.)
#
# Expected XML Format:
# ====================
#
# <tag>
# -----
#
# The <tag> node in the XML spec corresponds to a single such struct.
# It has attributes, name, cppname, and child tags <member>
# <tag> will cause a single such struct to be defined, with a custom default
# constructor which respects the default for each <member>.
# <tag> will also have a "visit" method declared which allows iteration over
# the tag.
# The wml::traits::tag trait will be specialized for this new type as well,
# giving it a name, and a generic "coerce from config" method.
#
# Example:
#
#   <tag name="foo" cppname="foo_tag">
#      <member type="int_val" name="x"/>
#      <member type="int_val" name="y"/>
#   </tag>
#
# Generated code:
#
#   struct foo_tag {
#     int_val x;
#     int_val y;
#
#     foo_tag() { ... }
#
#     template <typename V>
#     void visit(V && v) { ... }
#   };
#
#   namespace traits {
#
#   template <>
#   struct tag<foo_tag> : tag_base<foo_tag> {
#     static constexpr const char * name() { return "foo"; }
#   };
#
#   } // end namespace traits
#
#
#
# <fwd_tag>
# ---------
#
# The <fwd_tag> node creates a forward declaration in C++, in case there
# are circular references.
#
# Example:
#
#   <fwd_tag name="foo_tag" />
#
# Generated code:
#
#   struct foo_tag;
#
#
#
# <type_alias>
# ------------
#
# The <type_alias> node creates a type alias (typedef) in C++.
#
# Example:
#
#   <type_alias type="util::recursive_wrapper<unit_filter_tag>" name="unit_filter_wrapper" />
#
# Generated code:
#
#   using unit_filter_wrapper = "util::recursive_wrapper<unit_filter_tag>";
# 
#
#
# <group>
# -------
#
# Sometimes, the same sequence of attributes and children repeats over and over
# across many different WML tags. For instance, standard side filter and standard
# unit filter.
#
# To avoid this repitition, a group should be used.
# <group> creates a named collection of common <member> tags.
#
# First, a group is declared at top level, which contains the common info.
# Then, <group name="unit_filter" /> may appear in any <tag> definition.
#
# Groups must be defined before they are used, but the types contained need not
# be defined until an actual tag using the group appears.
#
# Example:
#
#   <group name="foo"> <member type="int_val" name="x" /> </group>
#   <group name="bar"> <member type="int_val" name="y" /> <group name="foo" /> </group>
#
#   <tag name="baz"> <member type="string_val" name="color"> <group name="baz" /> </tag>
#
# Generated code:
#
#   Similar to <tag name = "baz">
#                <member type="int_val" name="x" />
#                <member type="int_val" name="y" />
#                <member type="string_val" name="color" />
#              </tag>
#
# Note that <group> itself generates no code. We might change it so that <group>
# is implemented using C++ inheritance, but right now we don't.
#
#
#
# <heterogenous_sequence>
# -----------------------
#
# In many cases, containers for child tags of a tag can be simple. Maybe only
# one tag of the given type T may appear as a child. Then T or optional<T> may
# be the type of the corresponding struct member.
# If multiple T are permitted, then std::vector<T> is okay.
# For a catchall, all_children_map may be used.
#
# Sometimes, especially in things like action wml, there is a wide variety of
# tags that we must capture, and the order in which they appear matters. So
# none of the above solutions will work.
#
# The <heterogenous_sequence> node allows creating complex ad hoc containers.
# It creates a type of the form std::vector<variant<...>> over several tag types.
# Those types must be listed, and names associated to them, which might be different
# from their declared names. The heterogenous sequence will match any tags matching
# the declared names, and store them in sequence.
#
# Heteogenous sequence is used to define some complex structures like conditional wml
# and action wml, but also some other simpler cases.
#
# Example:
#   <heterogenous_sequence name="action_wml">
#      <tag type="kill_tag" alias="kill">
#      <tag type="recall_tag" alias="recall">
#      <tag type="set_recruit_tag" alias="set_recruit">
#   </heterogenous_sequence>
#
# Generated code:
#
#   struct action_wml : heterogenous_sequence_base<action_wml> {
#     static constexpr num_type = 3;
#     static constexpr const char * const * names() {
#       static const char * instance[] = { "kill", "recall", "set_recruit" };
#       return instance;
#     }
#     using var_t = util::variant<kill_tag, recall_tag, set_recruit_tag>;
#   };
#
# Note: A limitation of XML is that < and > characters must be escaped.
# However, these characters are needed for C++ template types, and the escapes
# greatly reduce readability.
# Therefore, in any attribute named 'type', we substitute '<' for '[' and '>'
# for ']', so that template types may be written in XML using square brackets.

import sys
import argparse
import xml.etree.ElementTree as ET
import re
import os

def error(message):
  raise Exception(message)

def replace_type_characters(str):
  return str.replace('[', '<').replace(']', '>')

# represents an output location
# keeps track of indentation level
class Writer(object):
  def __init__(self, outfile):
    self.out_file = open(outfile, 'w')
    self.indentation = 0

  def out(self, text):
    self.out_file.write(text)

  def align(self):
    self.out("  " * self.indentation)

  def newline(self):
    self.out('\n')
    self.align();

  def outln(self, text):
    self.out(text)
    self.newline()

  def indent(self):
    self.indentation = self.indentation + 1

  def unindent(self):
    self.indentation = self.indentation - 1
    

# represents a member of a tag
# should be constructed from a <member> node from an xml file.
class Member(object):
  def __init__(self, child):
    if child.tag != 'member':
      error("Expected xml tag of type 'member', found '" + child.tag + "'")

    self.type = child.get('type')
    self.name = child.get('name')
    self.default = child.get('default')
    self.alias = child.get('alias')

    if self.type is None:
      error("Member had no type")
    if self.name is None:
      error("Member had no name")

    self.type = replace_type_characters(self.type)

  def write_declaration(self, w):
    w.out(self.type + ' ' + self.name + ';')

  def write_initializer(self, w):
    w.out(self.name + '(')
    if self.default is not None:
      w.out(self.default)
    w.out(')')

  def write_visitation_args(self, w, is_extended):
    w.out('"')
    if is_extended and self.alias is not None:
      w.out(self.alias)
    else:
      w.out(self.name)

    w.out('", this->' + self.name)

    if is_extended and self.default is not None:
      w.out(', []()->' + self.type + ' { return ' + self.type + '(' + self.default + '); }')


# helper function: given an xml node, and a map from group names to member lists,
# produce a fully-flattened member list from the <member> and <group> children.

def member_list(child, groups):
  result = []
  for m in child:
    if m.tag == 'member':
      result = result + [ Member(m) ]
    elif m.tag == 'group':
      gname = m.get('name')
      if gname is None:
        error("Missing group name")
      if groups[gname] is None:
        error("Unknown group name: '" + gname + "'")
      result = result + groups[gname]
    else:
      error('Unexpected child node: ' + m.tag)
  return result


# represents a tag
# should be constructed from a <tag> node in an xml file.
class Tag(object):
  def __init__(self, child, groups):
    if child.tag != 'tag':
      error("Expected xml node of type 'tag', found '" + child.tag + "'")

    self.name = child.get('name')
    if self.name is None:
      error("Tag did not have a name")

    self.cppname = child.get('cppname')
    if self.cppname is None:
      self.cppname = self.name

    self.members = member_list(child, groups)

  def declare_members(self, w):
    for m in self.members:
      w.newline()
      m.write_declaration(w)

  def default_ctor(self, w):
    w.newline()
    w.out(self.cppname + "()")
    w.indent()
    first = True
    for m in self.members:
      w.newline()
      if first:
        w.out(": ")
        first = False
      else:
        w.out(', ')

      m.write_initializer(w)

    w.unindent()
    w.newline()
    w.out("{}")

  def define_visit_func(self, w, decl, is_extended):
    w.newline()
    w.outln("template <typename Visitor>")
    w.out(decl + " {")
    w.indent()
    for m in self.members:
      w.newline()
      w.out('std::forward<Visitor>(vis)(')
      m.write_visitation_args(w, is_extended)
      w.out(');')
    w.unindent()
    w.newline()
    w.out("}")

  def define_visit_funcs(self, w):
    self.define_visit_func(w, "void visit(Visitor && vis) &", False)
    w.outln("")
    self.define_visit_func(w, "void visit(Visitor && vis) const &", False)
    w.outln("")
    self.define_visit_func(w, "void visit_extended(Visitor && vis) &", True)
    w.outln("")
    self.define_visit_func(w, "void visit_extended(Visitor && vis) const &", True)

  def define_struct(self, w):
    w.out('struct ' + self.cppname + ' {')
    w.indent()

    self.declare_members(w)

    w.newline()

    self.default_ctor(w)

    w.newline()

    self.define_visit_funcs(w)

    w.unindent()
    w.newline()

    w.outln('};')

  def define_trait(self, w):
    w.outln('namespace traits {')
    w.outln('')

    w.outln('template <>')
    w.out('struct tag<' + self.cppname + '> : tag_base<' + self.cppname + '> {')
    w.indent()
    w.newline()

    w.out('static constexpr const char * name() { return "' + self.name + '"; }');

    w.unindent()
    w.newline()
    w.outln('};')

    w.outln('')
    w.outln('} // end namespace traits')

  def write(self, w):
    self.define_struct(w)
    w.outln('')
    self.define_trait(w)
    w.outln('')


# represents a heterogenous sequence definition
class Sequence(object):
  def __init__(self, child):
    self.name = child.get("name")
    if self.name is None:
      error("Expected 'heterogenous_sequence' to have a 'name'")
    self.strings = []
    self.types = []

    for c in child:
      if c.name != "tag":
        error("Unexpected member of 'heterogenous_sequence', expected <tag> found <" + c.name + ">")
      else:
        my_alias = c.get('alias')
        if my_alias is None:
          error("Missing 'alias' key of heterogenous_sequence tag child")

        my_type = c.get('type')
        if my_type is None:
          error("Missing 'type' key of heterogenous_sequence tag child")

        my_type = replace_type_characters(my_type)

        self.strings = self.strings + [my_alias]
        self.types = self.types + [my_type]

  def define_struct(self, w):
    w.out('struct ' + self.name + ' : heterogenous_sequence_base<' + self.name + '> {')
    w.indent()
    w.newline()
    w.outln('static constexpr int num_types = ' + len(self.types) + ';')
    w.out('static constexpr const char * const * names () {')
    w.indent()
    w.newline()
    w.outln('static constexpr const char * instance[] = { ' + ', '.join(self.strings) + ' };')
    w.out('return instance;')
    w.unindent()
    w.newline()
    w.outln('}')
    w.out('using var_t = util::variant<' + ', '.join(self.types) + '>;')

    w.unindent()
    w.newline()
    w.outln('}')    

  def define_trait(self, w):
    w.outln('namespace traits {')
    w.newline()

    w.outln('template <>')
    w.out('struct child_container<' + self.name + '> : hs_container_base<' + self.cppname + '> {};')

    w.newline()
    w.outln('} // end namespace traits')


  def write(self, w):
    self.define_struct(w)
    w.outln('')
    self.define_trait(w)
    w.outln('')

# represents the generator
class Generator(object):
  def __init__(self, target):
    self.target = target
    self.w = None
    self.groups = {}
  
  def process(self, infile, outfile):
    registry = ET.parse(infile).getroot()
    self.w = Writer(outfile)

    self.write_copyright()
    self.write_includes()

    self.w.outln('namespace wml {')
    self.w.outln('')

    for child in registry:
      if child.tag == 'tag':
        self.define_tag(child)
      elif child.tag == 'fwd_tag':
        self.declare_tag(child)
      elif child.tag == 'type_alias':
        self.make_type_alias(child)
      elif child.tag == 'heterogenous_sequence':
        self.define_hs(child)
      elif child.tag == 'group':
        self.define_group(child)
      else:
        error("Unexpected toplevel node: " + child.tag)

    self.w.outln('')
    self.w.outln('} // end namespace wml')

  def write_copyright(self):
    self.w.outln('// THIS FILE WAS AUTOMATICALLY GENERATED')
    self.w.outln('// Source file: ' + self.target + '.xml')
    self.w.outln('')

  def write_includes(self):
    self.w.outln('#pragma once')
    self.w.outln('')
    self.w.outln('#include <libwml/ast_components.hpp>')
    self.w.outln('#include <libwml/wml.hpp>')
    self.w.outln('#include <libwml/attributes.hpp>')
    self.w.outln('#include <libwml/child_containers.hpp>')
    self.w.outln('#include <libwml/util/variant.hpp>')
    self.w.outln('#include <libwml/util/optional.hpp>')
    self.w.outln('')
    self.w.outln('#include <type_traits>')
    self.w.outln('#include <utility>')
    self.w.outln('#include <vector>')
    self.w.outln('')

  def write_tag_decl(self, name):
    self.w.outln('struct ' + name + ';')
    self.w.newline()

  def make_type_alias(self, c):
    my_name = c.get('name')
    if my_name is None:
      error("Missing 'name' attribute of type_alias node")
    my_type = c.get('type')
    if my_type is None:
      error("Missing 'type' attribute of type_alias node")

    my_type = replace_type_characters(my_type)

    self.w.outln('using ' + my_name + ' = ' + my_type + ';')

  def declare_tag(self, c):
    val = c.get('name')
    if val is not None:
      self.write_tag_decl(val)
    else:
      val = c.get('cppname')
      if val is not None:
        self.write_tag_decl(val)
      else:
        error("Missing 'name' entry of 'fwd_tag' xml element")

  def define_tag(self, c):
    t = Tag(c, self.groups)
    t.write(self.w)

  def define_hs(self, c):
    s = Sequence(c)
    s.write(self.w)

  def define_group(self, c):
    name = c.get('name')
    if name is None:
      error("group tag must have a name attribute")
    if self.groups[name] is not None:
      error("group '" + name + "' cannot be redefined")
    self.groups[name] = member_list(c, groups)


argparser = argparse.ArgumentParser(description='Generate C++ structure definitions from XML schema file.')
argparser.add_argument('files', metavar='file.xml', nargs='+', help='XML files to be parsed')
argparser.add_argument('--dir', metavar='dir', required=True, help='Destination directory. Should be cmake build directory or similar.')
args = argparser.parse_args()

outdir = args.dir + '/'

for infile in args.files:
  name = os.path.basename(infile).split('.xml')[0]

  generator = Generator(name)
  generator.process(infile, outdir + name + ".hpp")
