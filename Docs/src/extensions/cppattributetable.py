from __future__ import annotations

import xml.etree.ElementTree as ET
import sys
from collections import defaultdict
from pathlib import Path
from typing import Dict, List, Sequence

from docutils import nodes
from sphinx import addnodes
from sphinx.application import Sphinx
from sphinx.locale import _
from sphinx.util.docutils import SphinxDirective

from attributetable import (
    attributetable,
    attributetablecolumn,
    attributetabletitle,
    attributetable_item,
    TableElement,
)


# ---------------------------------------------------------------------------
# Placeholder node (unchanged)
# ---------------------------------------------------------------------------

class cppattributetableplaceholder(nodes.General, nodes.Element):
    pass


# ---------------------------------------------------------------------------
# Raw-HTML helper nodes for <details>/<summary> dropdown
#
# We emit these as passthrough raw-HTML nodes so the rest of the doctree
# pipeline (cross-refs, etc.) still works normally for the child links.
# ---------------------------------------------------------------------------

class cpp_overload_details(nodes.General, nodes.Element):
    """Wraps one overloaded method group – renders as <details>…</details>."""


class cpp_overload_summary(nodes.General, nodes.Element):
    """The clickable header inside <details> – renders as <summary>…</summary>."""


def visit_cpp_overload_details(self, node: cpp_overload_details) -> None:
    self.body.append('<details class="py-attribute-overload-details">\n')


def depart_cpp_overload_details(self, node: cpp_overload_details) -> None:
    self.body.append('</details>\n')


def visit_cpp_overload_summary(self, node: cpp_overload_summary) -> None:
    self.body.append(
        '<summary class="py-attribute-overload-summary">'
        '<i class="py-attribute-overload-icon fas fa-chevron-right"></i>'
    )


def depart_cpp_overload_summary(self, node: cpp_overload_summary) -> None:
    self.body.append('</summary>\n')


# ---------------------------------------------------------------------------
# Directive
# ---------------------------------------------------------------------------

class CppAttributeTable(SphinxDirective):
    has_content = False
    required_arguments = 1

    def run(self):
        node = cppattributetableplaceholder("")
        node["cpp-class"] = self.arguments[0].strip()
        return [node]


# ---------------------------------------------------------------------------
# XML helpers
# ---------------------------------------------------------------------------

def is_documented(member):
    brief = member.find("briefdescription")
    detailed = member.find("detaileddescription")
    brief_text = "".join(brief.itertext()).strip() if brief is not None else ""
    detailed_text = "".join(detailed.itertext()).strip() if detailed is not None else ""
    return bool(brief_text or detailed_text)


def find_class_xml(xml_dir, classname):
    index_file = xml_dir / "index.xml"
    tree = ET.parse(index_file)
    root = tree.getroot()

    for compound in root.findall("compound"):
        kind = compound.attrib.get("kind")
        name = compound.findtext("name")
        if kind not in ("class", "struct"):
            continue
        if name == classname:
            refid = compound.attrib["refid"]
            return xml_dir / f"{refid}.xml"

    return None


def get_cpp_class_results(xml_file, classname):
    tree = ET.parse(xml_file)
    root = tree.getroot()

    # Collect raw entries first, preserving order of first appearance.
    # attributes: name -> [TableElement, ...]
    # methods:    name -> [TableElement, ...]  (may have multiple overloads)
    attr_map: Dict[str, List[TableElement]] = defaultdict(list)
    method_map: Dict[str, List[TableElement]] = defaultdict(list)
    attr_order: List[str] = []
    method_order: List[str] = []

    for section in root.iter("sectiondef"):
        section_kind = section.attrib.get("kind", "")
        if not (
            section_kind.startswith("public")
            or section_kind.startswith("protected")
        ):
            continue

        for member in section.findall("memberdef"):
            if not is_documented(member):
                continue

            member_kind = member.attrib.get("kind")
            member_id = member.attrib.get("id")
            name = member.findtext("name")

            if not member_id or not name:
                continue

            element = TableElement(fullname=member_id, label=name, badge=None)

            if member_kind == "variable":
                if name not in attr_map:
                    attr_order.append(name)
                attr_map[name].append(element)

            elif member_kind == "function":
                if name not in method_map:
                    method_order.append(name)
                method_map[name].append(element)

    # Build final TableElement list.
    # * Single entry  → plain TableElement (no children), as before.
    # * Multiple entries (overloads) → one parent TableElement whose
    #   `children` hold the individual overloads.  The parent's `fullname`
    #   points to the first overload so it is still a valid anchor.

    def collapse(name_order, name_map) -> List[TableElement]:
        result = []
        for name in name_order:
            overloads = name_map[name]
            if len(overloads) == 1:
                result.append(overloads[0])
            else:
                # Number the child labels for clarity: addState (1), addState (2) …
                numbered = [
                    TableElement(
                        fullname=ov.fullname,
                        label=f"{name} ({i + 1})",
                        badge=ov.badge,
                    )
                    for i, ov in enumerate(overloads)
                ]
                parent = TableElement(
                    fullname=overloads[0].fullname,
                    label=name,
                    badge=None,
                    children=numbered,
                )
                result.append(parent)
        return result

    return {
        "Attributes": collapse(attr_order, attr_map),
        "Methods": collapse(method_order, method_map),
    }


# ---------------------------------------------------------------------------
# Node builder – cpp-specific version of class_results_to_node
# ---------------------------------------------------------------------------

def cpp_class_results_to_node(
    key: str, elements: Sequence[TableElement]
) -> attributetablecolumn:
    """Like attributetable.class_results_to_node but handles overload groups."""

    title = attributetabletitle(key, key)
    ul = nodes.bullet_list("")

    for element in elements:
        if not element.children:
            # ── Plain entry (no overloads) ──────────────────────────────────
            ref = nodes.reference(
                "",
                "",
                internal=True,
                refuri=f"#{element.fullname}",
                anchorname="",
                *[nodes.Text(element.label)],
            )
            para = addnodes.compact_paragraph("", "", ref)
            if element.badge is not None:
                ul.append(attributetable_item("", element.badge, para))
            else:
                ul.append(attributetable_item("", para))
        else:
            # ── Overloaded group – render as <details> ──────────────────────
            details = cpp_overload_details("")

            summary = cpp_overload_summary("")
            summary += nodes.Text(element.label)
            details += summary

            child_ul = nodes.bullet_list("")
            for child in element.children:
                child_ref = nodes.reference(
                    "",
                    "",
                    internal=True,
                    refuri=f"#{child.fullname}",
                    anchorname="",
                    *[nodes.Text(child.label)],
                )
                child_para = addnodes.compact_paragraph("", "", child_ref)
                child_ul.append(attributetable_item("", child_para))

            details += child_ul
            ul.append(attributetable_item("", details))

    return attributetablecolumn("", title, ul)


# ---------------------------------------------------------------------------
# Event handler
# ---------------------------------------------------------------------------

def process_cpp_attributetable(
    app: Sphinx,
    doctree: nodes.document,
    fromdocname: str,
):
    xml_dir = Path(app.config.doxygen_xml_dir)

    for node in doctree.traverse(cppattributetableplaceholder):
        classname = node["cpp-class"]

        xml_file = find_class_xml(xml_dir, classname)
        if xml_file is None:
            node.replace_self([])
            continue

        groups = get_cpp_class_results(xml_file, classname)

        table = attributetable("")

        for label, items in groups.items():
            if not items:
                continue
            table.append(
                cpp_class_results_to_node(
                    label,
                    sorted(items, key=lambda x: x.label),
                )
            )

        table["python-class"] = classname

        if len(table) == 0:
            node.replace_self([])
        else:
            node.replace_self([table])


# ---------------------------------------------------------------------------
# Sphinx setup
# ---------------------------------------------------------------------------

def setup(app: Sphinx):
    app.add_directive("cppattributetable", CppAttributeTable)

    app.add_node(cppattributetableplaceholder)

    # Register the two new HTML-passthrough nodes
    app.add_node(
        cpp_overload_details,
        html=(visit_cpp_overload_details, depart_cpp_overload_details),
    )
    app.add_node(
        cpp_overload_summary,
        html=(visit_cpp_overload_summary, depart_cpp_overload_summary),
    )

    app.add_config_value("doxygen_xml_dir", None, "env")

    app.connect("doctree-resolved", process_cpp_attributetable)

    return {"parallel_read_safe": True}