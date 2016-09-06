libwml
======

**libwml** is a C++ library which assists in reading collections of wesnoth WML files.
It can handle loading individual files, campaigns, add-ons, or even the entire WML core.

It can also do this *without* expanding preprocessor macros, and produce a data representation
of the content with all macros in an unexpanded state.

In a nutshell, **libwml** tries to be for WML what **libclang** is for C++.

History and Purpose
===================

The first version of **libwml** was created with the idea of producing a very robust and exact tool
which could rewrite wesnoth WML datafiles in fabi's WSL language. I created a version which was
capable of reading the entire wesnoth core sometime in mid 2015. Actually it found several bugs
in core content, mainly typos in attribute names, which were reported and fixed. (However I never finished
developing the tool, and never announced it publicly.)

The major technical issue here is that in lua, there is no standard preprocessor. The idea is to
try to use functions in most cases where Wesnoth uses macros. We'd like to automatically rewrite the
macros and macro-uses as function definitions and functions calls. This means that we cannot use the
same parsing strategy that Wesnoth uses for WML -- when wesnoth reads WML, it expands all macros immediately,
in an initial preprocessing step. This is fine if you just want to load and run WML, but if you want to
diagnose macro problems, collect information about macros, or rewrite WML macros differently, you want to avoid
expanding macros, which means the parser needs to be smart enough to represent unexpanded macros, treat those
as a grammatical element, and parse around them.

As an example:

Here's some (nonsensical) WML containing a macro definition, a tag, and a macro instance.

```
#define BAR X
bar: X X

[unit]
  name = foo {BAR baz}
[/unit]
```

When wesnoth loads this, it will first scan through all the files, process all includes macros and definitions,
and perform textual replacements for each macro. This is the preprocessor step. It will get rid of all the macros,
and produce pure WML which looks something like this:

```
[unit]
  name = foo bar: baz baz
[/unit]
```

In the WML -> WSL application, what we would *like* to be able to do with WML like this is translate macros into function calls. So we might end up with
something like this in lua:

```
function bar(X)
  return "bar: " .. X .. X
end

Unit {
  name = 'foo' .. bar('baz')
}
```

This is better because many macros call other macros, etc., and are extremely complex, and so after the WML has been preprocessed, a human doesn't really want to look at it anymore.
When we translate WML -> WSL, we'd like it to be readable and maintainable as lua after that. (Perhaps the WML source is still maintained, and then it's better to retranslate it when it is
updated. But either way, human readability and avoiding macro bloat is important to us.)

**libwml** doesn't go all the way there. It doesn't include an emitter, only a parser. It produces C++ data structures which represent a full AST of the WML file. It can produce an intermediate representation *vaguely* like this:

Macros:
```
BAR(X): string -> string
"bar: " .. X .. X
```

WML:
```
[unit]
  name = foo {BAR baz}
[/unit]
```

The early, 2015 version of *libwml* was a *proof of concept* to see that we
could realistically parse most core WML and add-ons this way.

At that time, WSL was far from complete and it wasn't realistic to attempt to write an actual *emitter*.
So all we had was a front end and I just left it at that and set it aside until later.

Later, Wesnoth2 project was launched. In that project, one of the things that they would like to do is
rewrite / reimplement wesnoth campaigns into FFL, the "frogatto formula language" used by the Anura engine. FFL does have preprocessor macros like Wesnoth, but it also has a proper
notion of functions, and doesn't rely on macros nearly to the extent that Wesnoth 1 does. Usually you try to use functions whenever possible. That's
quite similar to what WSL attempts to do. So **libwml** might help to make an automatic tool to rewrite Wesnoth
WML campaigns automatically as FFL appropriate for the wesnoth 2 project.

Potentially, something like **libwml** could be used inside wesnoth itself, because it would lead to more
rigorous parsing and error reporting. The game itself would be capable of catching typo problems that right now
it just ignores. Also, **libwml** parsing is much faster than the parsing that wesnoth does internally, for a variety
of reasons, and the final data representation is much more compact than the `config` structure that wesnoth uses.
However, it would be a lot of work to port wesnoth itself to use **libwml**. Also **libwml** is
coded in a style that is quite different from wesnoth's codebase, making critical use of some powerful template libraries.
This is really important for **libwml**, it allows a ton of code-reuse in a parser library like this, and allows the **libwml** codebase to be very compact.
But wesnoth's codebase historically doesn't use any template stuff like that. The current developers might not be comfortable with this style.
And while wesnoth is pretty slow to load it's data files, it's not so slow that ripping out the old code is absolutely necessary. A big change like that
is also likely to break things. So it's not clear it's worth it for the Wesnoth 1 engine. Also, I hear some wesnoth developers really like MSVC, and I'm not
particularly interested in arguing about it.

A final consideration is that **libwml** could be used to to create a tool like **wmllint** which is very precise
and rigorous. The original **wmllint** tool was just a python script based on line-by-line regular expressions. It doesn't
parse WML in at all the same way that the main game does, and it's easy to write WML that is well-formed and which the game will load,
but which wmllint simply fails to parse. Effectively, **wmllint** is a quick-and-dirty hack, and when it is modifying your files, it is effectively
using dumb pattern matching and guesswork -- it has no idea of the larger context of the code that it is modifying or
what it means. A linter based on **libwml** has a full parse tree available when it is making changes, and it knows essentially
all of the information on the wesnoth wiki about how a WML campaign or scenario is *supposed* to look and what are appropriate values
for different fields, etc. So it should be easier to get much better results, and adding new lint operations should be straightforward rather than kludgy.

Current
=======

In mid 2016 I decided to resurrect the project, for the following reasons:

- Even if not all of these prospective applications actually happens, if one or two of them happens, and they are able to share code via some library like
  **libwml**, it might save everyone a ton of work, and increase maintainability of all of them collectively. It might be that more of them are likely to happen
  given that something like **libwml** exists.
- I had an idea that I could make a **wmllint** tool in C++ based on **libwml**, and cross-compile it to javascript via emscripten. Then you could potentially use it
  from your web browser without installing it, by drag-and-dropping your add-on folder into the page. That seems like a pretty nifty trick in general and I would like
  know how to make C++ javascript applications like that potentially for other projects as well. This is a good excuse for me to learn how to do that.
- I have been looking for a good practice project I can use to try out `boost::spirit::x3`, the new version of `boost::spirit`, which uses C++14 and supposedly
  improves greatly on `boost::spirit::qi`. The original version of **libwml** used `qi`, but it was somewhat messy. This version is an opportunity for me to figure out
  how `x3` works, while also simplifying and fixing some problems in the old version. (Note: It's currently still using `qi`, I didn't actually port it to `x3` yet.)

Usage
=====

In wesnoth's code base, all WML is represented by an object called `config`. You can look in the Wesnoth code for how it is defined, but basically it is a dynamic data
structure that represents the *body* of a WML tag. It contains

- A map of *attribute* keys and values, like this:
  ```
    std::map<std::string, attribute_value>;
  ```
  This is used to represent fields `X = Y` in a wml tag. `attribute_value` is somewhat like
  a `boost::variant` which can represent multiple different types of primitive values.

- A map of *config children*, like this:
  ```
    std::map<std::string, std::vector<config>>;
  ```
  which can represent all of the subtags of a given tag, and keep track of the order of similarly-named tags.

Every WML table is reperesented this way, and most things like multiplayer game settings or save file data is placed into configs in order to be transferred between different
parts of the engine. The network protocol also uses configs.

The point of the `config` is that it can accomodate any possible WML layout. The list of expected attributes, their possible types of values, and the allowed types of children
and their number, is all dynamically determined.

In **libwml**, we do it differently. For each possible type of tag, we have a different C++ structure type. Each possible attribute appears as a member variable, and it's type
reflects the allowed possible values. For each possible type of child tag, there is an appropriate corresponding container structure.

The result is that a tag is not represented by a generic "config" -- when you use **libwml**, you know statically the type of each config based on how it was parsed, you know what fields
to expect, and you know that any typos or errors would have been caught at the parsing step. Each tag is represented as compactly as possible, because we know statically what it's
members are and don't have to query dynamically from some `std::map`.

For instance, in the wesnoth codebase using configs, we might have code like

```
  if (cfg["name"] == "charlie") { ... }
```

The expression `cfg["name"]` will attempt to find an attribute `"name"` inside the config, and return a config attribute value reference if it finds this.
The config attribute value type is a somewhat amorphous type which could represent strings, integers, booleans, etc., and you usually coerce it to the type that you expect, although in this case you don't have to.

In **libwml**, the analogous expression would be

```
  cfg.name
```

and the result of this expression would be something like `std::string` or `boost::optional<std::string>`. If you make a typo in `name` or similar, it will be a compile-time error rather than a silent runtime error. And interacting with the `config` is simpler because we know statically the type of the member.

This also makes it much easier to write an emitter. You don't just get some very generic config object which you have to try to figure out how to translate into some programming language -- you get a data structure which corresponds
to a very particular component within the WML AST grammar, with some particular semantic meaning, and you don't have to query it dynamically to try to figure out what it is so that you know how to treat it. You can write very precise
functions that handle each component exactly. If you forget to handle one of them, it's a compile-time error rather than some obscure runtime error.

Basically, the decision to make the type of tag known at compile-time, rather than just using a `config` object like wesnoth does, makes parsing harder but emitting easier, and allows
certain optimizations. It also makes things simpler in some sense -- we don't need to have a bunch of external schema XML files and a loader for them, we basically just write out the schema
directly in C++.

If you want to just have everything parsed to a config-like object, **libwml** can also do that, you just have to leave off the last step of the loading process, which attempts to
coerce the content into top-level WML.

For more details and examples see documentation.

Requirements and Organization
=============================

**libwml** is a cross-platform header-only C++14 library. It is supported to build it for windows, linux, OS X, and also to cross-compile it using emscripten,
using `gcc >= 5.0`, and `clang >= 3.8`.

It is not supported to build it using MSVC. There are several bugs in MSVC templates and preprocessor capabilties, most glaringly, that the MSVC implementation
of `__VA_ARGS__` is seriously broken and not standards conforming, and working around it is too tedious and difficult for me to really be interested.
This is a long standing MSVC bug that microsoft seems to have no interest in fixing. There are similar issues in the MSVC implementation of template instantiation.
These features are crucial for **libwml**, without which the size of the library would be much larger and it would be much harder to maintain.

To build on windows, I recommend to use mingw, or if you want to use visual studio, use it with clang.

**libwml** depends on:
- `boost >= 1.60`
- an external variant type called `strict_variant`. (See [github](https://github.com/cbeck88/strict-variant)) (You could probably build it also with `boost::variant` or `std::variant`, or any other C++14 variant type as you like. The header file `util/variant.hpp` forwards the subset of the interface that we need.)
- an external optional type derived from the standardization proposal of C++17. (See [github](https://github.com/akrzemi1/Optional)) (You could probably build it also with `boost::optional` if you want.)

The **libwml** code can be found in `include/` directory.

The `lib/` directory contains the current in-tree versions of the external dependencies besides `boost`.

There is an example executable built with it which is found in the `src/` directory. This executable attempts to parse a wml file or directory, and reports any problems it finds.

The `assets/` directory contains a number of mainline campaigns which were used as test cases.

The example executable can be build using `cmake`. There are also a number of bash scripts in the root of the repository which you can use to get push-button builds with various compilers.

Please note that the example executable is *NOT* part of **libwml** and you don't need to build it to use **libwml**. You only need the stuff in `include`, and possibly
in `lib/` if you don't have that already.

Licensing and Distribution
==========================

**libwml** is available under the terms of the WTFPL version 2.0. See `/LICENSE`, or [online](http://www.wtfpl.net).

All code in `lib/` is available under the boost software license.

Alternative Approaches
======================

There are several other recent projects that attempted to tackle issues of parsing WML rigorously. It's probably worth also to mention that `wmlindent` and several other of the python tools in the wesnoth source tree, such as `wmlxgettext`, struggle with this in some cases.

Nobun created a second version of `wmlxgettext` https://forums.wesnoth.org/viewtopic.php?f=10&t=43213, which has documentation here: http://wmlxgettext-unoff.readthedocs.io/en/latest/

In terms of high level strategy, `wmlxgettext2` is somewhere in between the **libwml** strategy and the original line-by-line regex strategy. Basically, `wmlxgettext2`
keeps track of some context for each line, using a state machine, but the transitions of this state machine are still determined by line-by-line regex, if I understand
correctly. So it has more contextual info than straight regex allows, but less than a full AST. It is sophisticated enough that it supports things like translatable strings within
lua blocks.

Another line-by-line regex approach has been attempted by fabi, however I don't know where to find the sources of that right now.
