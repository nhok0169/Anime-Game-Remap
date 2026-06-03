from __future__ import annotations

import xml.etree.ElementTree as ET
import sys
from pathlib import Path
from typing import Dict, List

from docutils import nodes
from sphinx.application import Sphinx
from sphinx.locale import _
from sphinx.util.docutils import SphinxDirective

from attributetable import (
    attributetable,
    class_results_to_node,
    TableElement,
)


class cppattributetableplaceholder(nodes.General, nodes.Element):
    pass


class CppAttributeTable(SphinxDirective):
    has_content = False
    required_arguments = 1

    def run(self):
        node = cppattributetableplaceholder("")
        node["cpp-class"] = self.arguments[0].strip()
        return [node]


def is_documented(member):
    brief = member.find("briefdescription")
    detailed = member.find("detaileddescription")

    brief_text = ""
    detailed_text = ""

    if brief is not None:
        brief_text = "".join(brief.itertext()).strip()

    if detailed is not None:
        detailed_text = "".join(detailed.itertext()).strip()

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

    groups = {
        "Attributes": [],
        "Methods": [],
    }

    for section in root.iter("sectiondef"):
        section_kind = section.attrib.get("kind", "")

        # Only public/protected members
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

            if member_kind == "variable":
                groups["Attributes"].append(
                    TableElement(
                        fullname=member_id,
                        label=name,
                        badge=None,
                    )
                )

            elif member_kind == "function":
                groups["Methods"].append(
                    TableElement(
                        fullname=member_id,
                        label=name,
                        badge=None,
                    )
                )

    return groups


def process_cpp_attributetable(
    app: Sphinx,
    doctree: nodes.document,
    fromdocname: str,
):
    xml_dir = Path(app.config.doxygen_xml_dir)

    for node in doctree.traverse(cppattributetableplaceholder):
        classname = node["cpp-class"]

        xml_file = find_class_xml(
            xml_dir,
            classname,
        )

        if xml_file is None:
            node.replace_self([])
            continue

        groups = get_cpp_class_results(
            xml_file,
            classname,
        )

        table = attributetable("")

        for label, items in groups.items():

            if not items:
                continue

            table.append(
                class_results_to_node(
                    label,
                    sorted(items, key=lambda x: x.label),
                )
            )

        table["python-class"] = classname

        if len(table) == 0:
            node.replace_self([])
        else:
            node.replace_self([table])


def setup(app: Sphinx):

    app.add_directive(
        "cppattributetable",
        CppAttributeTable,
    )

    app.add_node(cppattributetableplaceholder)

    app.add_config_value(
        "doxygen_xml_dir",
        None,
        "env",
    )

    app.connect(
        "doctree-resolved",
        process_cpp_attributetable,
    )

    return {
        "parallel_read_safe": True,
    }