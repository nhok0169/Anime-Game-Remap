#include "PyIfTemplateTree.h"

#include "PyIfTemplateNode.h"


namespace py = pybind11;
namespace AGRC = AGRemapCore;


void initCppIfTemplateTree(pybind11::module_ &m) {
    py::class_<PyIfTemplateTree>(m, "IfTemplateTree", R"doc(
The parse tree for some :class:`IfTemplate`, reached via :attr:`IfTemplate.tree` -- never
constructed directly.

.. note::
    The parse tree is structured such that:

    * A node is composed of :class:`IfContentPart`\s or other nodes
    * The children to the node occur when the node enters a specific branching condition :raw-html:`<br />` :raw-html:`<br />`

    eg. Suppose we have this branching structure

    .. code-block:: ini
        :linenos:

        ...(does stuff)...
        if ...(bool)...
            if ...(bool)...
                ...(does stuff)...
            else if ...(bool)...
                ...(does stuff)...
            endif
        else ...(bool)...
            ...(does stuff)...
            if ...(bool)...
                if ...(bool)...
                    ...(does stuff)...
                endif
                ...(does stuff)...
            endif
            ...(does stuff)...
            if
            endif
        endif
        ...(does stuff)...

    :raw-html:`<br />`

    Let ``C`` be some :class:`IfContentPart` (the parts that say ``...(does stuff)...``)

    Let ``B`` be some branching point (the parts that say ``if`` or ``else``)

    Let ``[...]`` be some node

    Let ``X`` be a node without any parts

    The parse tree generated for the above code would be:

    .. code-block::

               [C B B C]
                  | |
             +----+ +----+
             |           |
           [B B]     [C B C B]
            | |         |   |
         +--+ +--+    [B C] X
         |       |     |
        [C]     [C]   [C]

.. note::
    A leaf node with no parts at all (the ``X`` above -- an empty condition, eg. a bare
    ``if``/``endif`` with nothing between them) only ever shows up in a tree built for
    :meth:`IfTemplate.add`'s own bookkeeping. Every :attr:`IfTemplate.tree` a real caller sees is
    always built one of two other ways instead, and the difference matters if you're inspecting
    :attr:`root`/:attr:`IfTemplateNode.parts` directly:

    * **By default** (how :attr:`IfTemplate.tree` is built by the constructor): an otherwise-empty
      leaf node gets one synthetic, empty :class:`IfContentPart` placeholder instead of staying
      empty -- so

      .. code-block:: ini

          if
          endif

      (parse subtree ``[B]`` -> ``X``) becomes, for tree-building purposes, equivalent to

      .. code-block:: ini

          if
              ...(does nothing)...
          endif

      (parse subtree ``[B]`` -> ``[C]``) -- every leaf in the worked example above that would
      otherwise be an empty ``X`` node picks up this placeholder instead, eg. the ``if`` with
      nothing in it right before the final ``endif``.
    * **After calling** :meth:`IfTemplate.normalize` **specifically**: on top of the placeholder
      behavior above, an empty ``else`` clause is also synthesized for any conditional that
      doesn't already end with a single ``else`` -- so

      .. code-block:: ini

          if
              ...(does stuff)...
          else if
              ...(does stuff)...
          endif

      (parse subtree ``[B B]`` with two leaf children) becomes

      .. code-block:: ini

          if
              ...(does stuff)...
          else if
              ...(does stuff)...
          else
              ...(does nothing)...
          endif

      (parse subtree ``[B B B]`` with three leaf children -- the new ``else`` picking up the same
      empty-placeholder treatment as above). Applied to the whole worked example above, every
      ``if``/``elif`` chain that doesn't already end with a plain ``else`` gains one, including
      the already-empty ``if``/``endif`` at the bottom (which ends up with *two* leaf children --
      one placeholder for the original empty ``if`` branch, one for its synthesized ``else``).

.. note::
    The nodes are the `IfContentPart`\s of the `IfTemplate`, wrapped in :class:`IfTemplateNode`\s
    forming the tree structure. See :class:`IfTemplateNode` for the parts/children each node holds.
    )doc")

        .def_property_readonly("root", [](PyIfTemplateTree &self) -> PyIfTemplateNode* {
            return self.root();
        }, py::return_value_policy::reference_internal,
    py::doc(R"doc(Optional[:class:`IfTemplateNode`]: The root node in the parse tree)doc"))

        .def("clear", &PyIfTemplateTree::clear,
    py::doc(R"doc(Clears the tree)doc"));
}
