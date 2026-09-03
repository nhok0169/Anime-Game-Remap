#include "PyResGroupCollect.h"

#include <utility>

#include "../../../iniresources/PyIniGroupedResource.h"  // the *registered* grouped-resource type
#include "../regEdits/PyBaseRegEdit.h"  // reuses PyPartRanges (the "a bound Ranges, or a raw list
                                        // of bounds" resolver every predicate result goes through)


namespace {

// Auto-generated ids, matching the pure-Python original's own module-level ResGroupCollectAutoId
// counter (which started at -1 and pre-incremented, so the first instance gets 0).
long long nextAutoId() {
    static long long autoId = -1;
    ++autoId;
    return autoId;
}


PyResGroupCollectCore::Remapper::RemapTarget parseRemapTarget(const py::object &targetObj) {
    PyResGroupCollectCore::Remapper::RemapTarget result(parseGraphId(targetObj));

    py::sequence targetSeq = targetObj.cast<py::sequence>();
    if (targetSeq.size() > 3) {
        py::object renameFuncObj = py::reinterpret_borrow<py::object>(targetSeq[3]);
        if (!renameFuncObj.is_none()) {
            result.renameFunc = [renameFuncObj](const std::string &oldSectionName) {
                return renameFuncObj(py::str(oldSectionName)).cast<std::string>();
            };
        }
    }

    return result;
}

}


// ---------------------------------------------------------------------------------------
// PyGroupedResBuilder
// ---------------------------------------------------------------------------------------

PyGroupedResBuilder::PyGroupedResBuilder(py::object builder, py::object ini, PyIniResEditContext &ctx):
    builder_(std::move(builder)), ini_(std::move(ini)), ctx_(ctx) {}


AGRC::IniGroupedResource* PyGroupedResBuilder::build() {
    if (builder_.is_none()) {
        return nullptr;
    }

    py::object result = builder_.attr("build")(py::arg("isBuilt") = false);
    if (result.is_none()) {
        return nullptr;
    }

    // Cast to PyIniGroupedResource, not to AGRC::IniGroupedResource: only the former is a
    // registered pybind11 type ('IniGroupedResource' on the Python side), and pybind11 can only
    // cast to a type it has registered. The upcast afterwards is a plain C++ one.
    auto *raw = static_cast<AGRC::IniGroupedResource*>(result.cast<PyIniGroupedResource*>());
    groupsKeepAlive_.append(result);
    groupHandles_[raw] = result;
    return raw;
}


void PyGroupedResBuilder::store(AGRC::IniGroupedResource &resource) {
    if (ini_.is_none()) {
        return;
    }

    auto it = groupHandles_.find(&resource);
    if (it == groupHandles_.end()) {
        return;
    }

    ini_.attr("resources").attr("append")(it->second);
}


void PyGroupedResBuilder::addResource(AGRC::IniGroupedResource &group, const PyResGroupCollectCore::GraphId &resType,
                                       AGRC::IniResource &resource) {
    auto groupIt = groupHandles_.find(&group);
    if (groupIt == groupHandles_.end()) {
        return;
    }

    py::object resourceObj = ctx_.resourceToPy(&resource);
    if (resourceObj.is_none()) {
        return;
    }

    // Keyed by the whole (iniIndex, component, object) tuple, exactly as the pure-Python original
    // did -- PyIniGroupedResource's 'resources' is a real Python dict, not the C++ class's
    // string-keyed map.
    groupIt->second.attr("addResource")(graphIdToPy(resType), resourceObj);
}


// ---------------------------------------------------------------------------------------
// PyResGroupCollect
// ---------------------------------------------------------------------------------------

PyResGroupCollect::PyResGroupCollect(py::object resGroupTypes, py::object srcRegs, py::object resEdits,
                                      py::object groupedResBuilders, py::object partPredicates, py::object resPredicates,
                                      py::object remaps, py::object trackKeys, py::object keysToTrack,
                                      bool resGroupTypesSameTopology, const py::object &id):
    resGroupTypesObj(std::move(resGroupTypes)), srcRegsObj(std::move(srcRegs)), resEditsObj(std::move(resEdits)),
    groupedResBuildersObj(std::move(groupedResBuilders)), partPredicatesObj(std::move(partPredicates)),
    resPredicatesObj(std::move(resPredicates)), remapsObj(std::move(remaps)), trackKeysObj(std::move(trackKeys)),
    keysToTrackObj(std::move(keysToTrack)) {
    this->resGroupTypesSameTopology = resGroupTypesSameTopology;
    this->id = id.is_none() ? nextAutoId() : id.cast<long long>();

    // The section-name -> V conversion the spliced-in call sites need; the same shape
    // IfTemplateRunConfig's own valOfSectionName has.
    this->valOfSectionName = [](const std::string &name) { return name; };

    refresh(nullptr);
}


void PyResGroupCollect::refresh(PyIniResEditContext *ctx) {
    setResGroupTypes([this]() {
        std::vector<std::string> result;
        if (resGroupTypesObj.is_none()) {
            return result;
        }

        for (auto item : resGroupTypesObj) {
            result.push_back(py::str(item).cast<std::string>());
        }

        return result;
    }());

    srcRegs.clear();
    resEdits.clear();
    groupedResBuilders.clear();
    partPredicates.clear();
    resPredicates.clear();
    remaps.clear();
    trackKeys.clear();
    keysToTrack.clear();
    builders_.clear();

    if (!srcRegsObj.is_none()) {
        for (auto outer : srcRegsObj.cast<py::dict>()) {
            ByGraph<std::string> inner;
            for (auto item : py::reinterpret_borrow<py::object>(outer.second).cast<py::dict>()) {
                inner.insert_or_assign(parseGraphId(py::reinterpret_borrow<py::object>(item.first)),
                                        py::str(item.second).cast<std::string>());
            }

            srcRegs.insert_or_assign(parseGraphId(py::reinterpret_borrow<py::object>(outer.first)), std::move(inner));
        }
    }

    if (!resEditsObj.is_none()) {
        for (auto outer : resEditsObj.cast<py::dict>()) {
            tsl::ordered_map<std::string, ResEdit*> inner;
            for (auto item : py::reinterpret_borrow<py::object>(outer.second).cast<py::dict>()) {
                inner.insert_or_assign(py::str(item.first).cast<std::string>(),
                                        py::reinterpret_borrow<py::object>(item.second).cast<PyBaseResEditCore*>());
            }

            resEdits.insert_or_assign(parseGraphId(py::reinterpret_borrow<py::object>(outer.first)), std::move(inner));
        }
    }

    // The wrappers capture the context (and through it the .ini file finished groups are appended
    // to), which is only known per call -- hence rebuilding them here rather than once at
    // construction. 'clear()' is the only caller with no context, and it never reaches a builder.
    if (!groupedResBuildersObj.is_none() && ctx != nullptr) {
        for (auto item : groupedResBuildersObj.cast<py::dict>()) {
            builders_.push_back(std::make_unique<PyGroupedResBuilder>(py::reinterpret_borrow<py::object>(item.second),
                                                                       ctx->ini, *ctx));
            groupedResBuilders.insert_or_assign(py::str(item.first).cast<std::string>(), builders_.back().get());
        }
    }

    if (!partPredicatesObj.is_none()) {
        for (auto outer : partPredicatesObj.cast<py::dict>()) {
            ByGraph<PartPredicate> inner;
            for (auto item : py::reinterpret_borrow<py::object>(outer.second).cast<py::dict>()) {
                py::object predicate = py::reinterpret_borrow<py::object>(item.second);
                if (predicate.is_none()) {
                    continue;
                }

                inner.insert_or_assign(parseGraphId(py::reinterpret_borrow<py::object>(item.first)),
                                        [predicate](const IterQueryData &iterData) -> OrderRanges {
                    py::object result = predicate(py::cast(&iterData, py::return_value_policy::reference));

                    PyPartRanges ranges(result);
                    const OrderRanges *parsedRanges = ranges.get();
                    if (parsedRanges == nullptr) {
                        throw py::type_error("A ResGroupCollect partPredicate must return a Ranges (or a list of (start, end) bounds), not None");
                    }

                    return *parsedRanges;
                });
            }

            partPredicates.insert_or_assign(parseGraphId(py::reinterpret_borrow<py::object>(outer.first)), std::move(inner));
        }
    }

    if (!resPredicatesObj.is_none()) {
        for (auto outer : resPredicatesObj.cast<py::dict>()) {
            ByGraph<ResPredicate> inner;
            for (auto item : py::reinterpret_borrow<py::object>(outer.second).cast<py::dict>()) {
                py::object predicate = py::reinterpret_borrow<py::object>(item.second);
                if (predicate.is_none()) {
                    continue;
                }

                inner.insert_or_assign(parseGraphId(py::reinterpret_borrow<py::object>(item.first)),
                                        [predicate](const std::string &reg, const std::string &val, const IterQueryData &iterData) {
                    return predicate(reg, val, py::cast(&iterData, py::return_value_policy::reference)).cast<bool>();
                });
            }

            resPredicates.insert_or_assign(parseGraphId(py::reinterpret_borrow<py::object>(outer.first)), std::move(inner));
        }
    }

    if (!remapsObj.is_none()) {
        for (auto outer : remapsObj.cast<py::dict>()) {
            tsl::ordered_map<std::string, Remapper::RemapTarget> targets;
            for (auto item : py::reinterpret_borrow<py::object>(outer.second).cast<py::dict>()) {
                targets.insert_or_assign(py::str(item.first).cast<std::string>(),
                                          parseRemapTarget(py::reinterpret_borrow<py::object>(item.second)));
            }

            remaps.insert_or_assign(parseGraphId(py::reinterpret_borrow<py::object>(outer.first)), std::move(targets));
        }
    }

    trackKeysIsGlobal = py::isinstance<py::bool_>(trackKeysObj);
    trackKeysGlobal = trackKeysIsGlobal ? trackKeysObj.cast<bool>() : false;

    if (!trackKeysIsGlobal && !trackKeysObj.is_none()) {
        for (auto outer : trackKeysObj.cast<py::dict>()) {
            ByGraph<bool> inner;
            for (auto item : py::reinterpret_borrow<py::object>(outer.second).cast<py::dict>()) {
                inner.insert_or_assign(parseGraphId(py::reinterpret_borrow<py::object>(item.first)),
                                        py::reinterpret_borrow<py::object>(item.second).cast<bool>());
            }

            trackKeys.insert_or_assign(parseGraphId(py::reinterpret_borrow<py::object>(outer.first)), std::move(inner));
        }
    }

    if (!keysToTrackObj.is_none()) {
        for (auto outer : keysToTrackObj.cast<py::dict>()) {
            ByGraph<std::optional<std::unordered_set<std::string>>> inner;

            for (auto item : py::reinterpret_borrow<py::object>(outer.second).cast<py::dict>()) {
                py::object keysObj = py::reinterpret_borrow<py::object>(item.second);
                std::optional<std::unordered_set<std::string>> keys;

                if (!keysObj.is_none()) {
                    std::unordered_set<std::string> parsedKeys;
                    for (auto keyItem : keysObj) {
                        parsedKeys.insert(py::cast<std::string>(keyItem));
                    }
                    keys = std::move(parsedKeys);
                }

                inner.insert_or_assign(parseGraphId(py::reinterpret_borrow<py::object>(item.first)), std::move(keys));
            }

            keysToTrack.insert_or_assign(parseGraphId(py::reinterpret_borrow<py::object>(outer.first)), std::move(inner));
        }
    }
}


py::object PyResGroupCollect::resCallsToPy() const {
    py::dict result;

    for (const auto &resTypeEntry : resCalls) {
        py::dict bySrc;

        for (const auto &srcEntry : resTypeEntry.second) {
            py::dict sections;

            for (const auto &sectionEntry : srcEntry.second) {
                py::dict parts;

                for (const auto &partEntry : sectionEntry.second) {
                    py::dict calls;
                    for (const auto &callEntry : partEntry.second) {
                        calls[py::cast(callEntry.first)] = py::make_tuple(callEntry.second.val,
                                                                          callEntry.second.query.has_value()
                                                                              ? py::cast(*callEntry.second.query)
                                                                              : py::none());
                    }

                    parts[py::cast(partEntry.first)] = std::move(calls);
                }

                sections[py::str(sectionEntry.first)] = std::move(parts);
            }

            bySrc[graphIdToPy(srcEntry.first)] = std::move(sections);
        }

        result[graphIdToPy(resTypeEntry.first)] = std::move(bySrc);
    }

    return result;
}


void initCppResGroupCollect(pybind11::module_ &m) {
    auto cls = py::class_<PyResGroupCollect, PyBaseIniGraphGroupEdit, py::smart_holder>(m, "ResGroupCollect", R"doc(
This class inherits from :class:`BaseIniGraphGroupEdit`

Creates the :class:`IniSectionGraph` for a particular group of resources

Where :class:`ResRegCollect` handles one resource at a time, this handles several that belong
together and works out which combinations of them can actually co-occur -- two resources belong in
the same group only if the conditional branches they live under are simultaneously satisfiable

Parameters
----------
resGroupTypes: List[:class:`str`]
    The unique names for the type of resource groups

srcRegs: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`str`]]
    The different registers that reference the particular resource :raw-html:`<br />` :raw-html:`<br />`

    * The outer keys are the mod object for a particular type of resource in a resource group
    * The inner keys are the location of which :class:`IniSectionGraph` to search for
    * The values are the source registers

resEdits: Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[:class:`str`, :class:`BaseResEdit`]]
    Describes how each resource in a resource group should be built :raw-html:`<br />` :raw-html:`<br />`

    * The outer keys are the mod object for a particular type of resource in a resource group
    * The inner keys are the names for the type of resource groups
    * The values are the edits for the type of resource

groupedResBuilders: Dict[:class:`str`, :class:`IniGroupedResBuilder`]
    The builders used to construct a type of grouped resource

partPredicates: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`SectionIterQueryData`], :class:`Ranges`]]]]
    The predicates for which particular order indices to process for some :class:`IfContentPart` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

resPredicates: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable[[:class:`str`, :class:`str`, :class:`SectionIterQueryData`], :class:`bool`]]]]
    The predicates to check whether some reference to the resource should be used :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

remaps: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[:class:`str`, Union[Tuple[:class:`int`, :class:`str`, :class:`str`], Tuple[:class:`int`, :class:`str`, :class:`str`, Callable[[:class:`str`], :class:`str`]]]]]]
    Whether to remap the graphs searched from :attr:`srcRegs`. The values follow the same format as
    :attr:`GraphGroupRemap.remap` :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

trackKeys: Union[:class:`bool`, Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`bool`]]]
    Whether to track the `KVPs`_ in the .ini file when searching for particular resources :raw-html:`<br />` :raw-html:`<br />`

    If this parameter is a boolean, this flag will be globally used for all graphs :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

keysToTrack: Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Optional[Set[:class:`str`]]]]]
    Specific keys to track in the .ini file when searching particular resources :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``

resGroupTypesSameTopology: :class:`bool`
    A flag used to enable an optimization to reduce the number of `satisfiable (SAT) problems`_
    needed to be computed when there are multiple types of resource groups. The flag assumes that
    each resource type for all types of resource groups have the same topology :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``False``

id: Optional[:class:`int`]
    The unique id for this object. If this value is ``None``, then an id is autogenerated :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``None``
    )doc");

    cls.def(py::init([](py::object resGroupTypes, py::object srcRegs, py::object resEdits, py::object groupedResBuilders,
                        py::object partPredicates, py::object resPredicates, py::object remaps, py::object trackKeys,
                        py::object keysToTrack, bool resGroupTypesSameTopology, const py::object &id) {
        return std::make_unique<PyResGroupCollect>(std::move(resGroupTypes), std::move(srcRegs), std::move(resEdits),
                                                    std::move(groupedResBuilders), std::move(partPredicates),
                                                    std::move(resPredicates), std::move(remaps), std::move(trackKeys),
                                                    std::move(keysToTrack), resGroupTypesSameTopology, id);
    }), py::arg("resGroupTypes"), py::arg("srcRegs"), py::arg("resEdits"), py::arg("groupedResBuilders"),
        py::arg("partPredicates") = py::none(), py::arg("resPredicates") = py::none(), py::arg("remaps") = py::none(),
        py::arg("trackKeys") = false, py::arg("keysToTrack") = py::none(), py::arg("resGroupTypesSameTopology") = false,
        py::arg("id") = py::none());

    cls.def_property("resGroupTypes", [](const PyResGroupCollect &self) { return self.resGroupTypesObj; },
                     [](PyResGroupCollect &self, py::object value) { self.resGroupTypesObj = std::move(value); },
                     py::doc(R"doc(List[:class:`str`]: The unique names for the type of resource groups)doc"));

    cls.def_property("srcRegs", [](const PyResGroupCollect &self) { return self.srcRegsObj; },
                     [](PyResGroupCollect &self, py::object value) { self.srcRegsObj = std::move(value); },
                     py::doc(R"doc(
Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], :class:`str`]]:
The different registers that reference the particular resource
    )doc"));

    cls.def_property("resEdits", [](const PyResGroupCollect &self) { return self.resEditsObj; },
                     [](PyResGroupCollect &self, py::object value) { self.resEditsObj = std::move(value); },
                     py::doc(R"doc(
Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[:class:`str`, :class:`BaseResEdit`]]:
Describes how each resource in a resource group should be built
    )doc"));

    cls.def_property("groupedResBuilders", [](const PyResGroupCollect &self) { return self.groupedResBuildersObj; },
                     [](PyResGroupCollect &self, py::object value) { self.groupedResBuildersObj = std::move(value); },
                     py::doc(R"doc(
Dict[:class:`str`, :class:`IniGroupedResBuilder`]: The builders used to construct a type of grouped
resource
    )doc"));

    cls.def_property("partPredicates", [](const PyResGroupCollect &self) { return self.partPredicatesObj; },
                     [](PyResGroupCollect &self, py::object value) { self.partPredicatesObj = std::move(value); },
                     py::doc(R"doc(
Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable]]:
The predicates for which particular order indices to process for some :class:`IfContentPart`
    )doc"));

    cls.def_property("resPredicates", [](const PyResGroupCollect &self) { return self.resPredicatesObj; },
                     [](PyResGroupCollect &self, py::object value) { self.resPredicatesObj = std::move(value); },
                     py::doc(R"doc(
Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Callable]]:
The predicates to check whether some reference to the resource should be used
    )doc"));

    cls.def_property("remaps", [](const PyResGroupCollect &self) { return self.remapsObj; },
                     [](PyResGroupCollect &self, py::object value) { self.remapsObj = std::move(value); },
                     py::doc(R"doc(
Optional[Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[:class:`str`, Tuple]]]: Whether
to remap the graphs searched from :attr:`srcRegs`
    )doc"));

    cls.def_property("trackKeys", [](const PyResGroupCollect &self) { return self.trackKeysObj; },
                     [](PyResGroupCollect &self, py::object value) { self.trackKeysObj = std::move(value); },
                     py::doc(R"doc(
Union[:class:`bool`, Dict]: Whether to track the `KVPs`_ in the .ini file when searching for
particular resources
    )doc"));

    cls.def_property("keysToTrack", [](const PyResGroupCollect &self) { return self.keysToTrackObj; },
                     [](PyResGroupCollect &self, py::object value) { self.keysToTrackObj = std::move(value); },
                     py::doc(R"doc(
Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Dict[Tuple[:class:`int`, :class:`str`, :class:`str`], Optional[Set[:class:`str`]]]]:
Specific keys to track in the .ini file when searching particular resources
    )doc"));

    cls.def_readwrite("resGroupTypesSameTopology", &PyResGroupCollect::resGroupTypesSameTopology, py::doc(R"doc(
:class:`bool`: Whether each resource type for all types of resource groups have the same topology
    )doc"));

    cls.def_readwrite("id", &PyResGroupCollect::id, py::doc(R"doc(
:class:`int`: The unique id for this object
    )doc"));

    cls.def_property_readonly("resCalls", &PyResGroupCollect::resCallsToPy, py::doc(R"doc(
Dict: The calls to each resource, keyed by resource type, then source graph, then `section`_ name,
then part id, then order index. The values are ``(resource section name, query)`` tuples

.. note::
    This is scratch state, rebuilt by every :meth:`edit` and cleared again afterwards. Reading it
    back gives a freshly built ``dict``, not a live view
    )doc"));

    cls.def("clear", [](PyResGroupCollect &self) {
        self.refresh(nullptr);
        self.Core::clear();
    }, py::doc(R"doc(Clears :attr:`resCalls` and every resource edit's own saved state)doc"));

    cls.def("edit", [](PyResGroupCollect &self, py::list graphGroups, const py::object &modType, const std::string &modName) {
        PyIniResEditContext ctx(py::none(), modType);
        self.refresh(&ctx);

        PyIniGraphGroups groups(graphGroups);
        // nullptr for modType: nothing here reads it directly, and the Python-side ModType has no
        // C++ counterpart to cast to -- the resource edits get it through their own context.
        self.Core::edit(groups, nullptr, modName);
        return graphGroups;
    }, py::arg("graphGroups"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
Collects and groups the references to the resources

.. note::
    With no .ini file there is nothing to build the resources *for*, so this collects and groups but
    builds nothing -- exactly as the pure-Python original did

Parameters
----------
graphGroups: List[:class:`IniGraphGroup`]
    The group of graphs to edit for each .ini file

modType: Optional[:class:`ModType`]
    The type of mod to fix

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
List[:class:`IniGraphGroup`]
    The same list that was passed in, after editing
    )doc"));

    cls.def("editFromIni", [](PyResGroupCollect &self, py::list graphGroups, const py::object &ini, const py::object &modType,
                              const std::string &modName) {
        PyIniResEditContext ctx(ini, modType);
        self.refresh(&ctx);

        PyIniGraphGroups groups(graphGroups);
        self.Core::editWithContext(groups, ctx, nullptr, modName);
        return graphGroups;
    }, py::arg("graphGroups"), py::arg("ini"), py::arg("modType"), py::arg("modName") = "", py::doc(R"doc(
Collects, groups, replicates and connects the resources for 'ini'

Parameters
----------
graphGroups: List[:class:`IniGraphGroup`]
    The group of graphs to edit for each .ini file

ini: :class:`IniFile`
    The associated original .ini file

modType: :class:`ModType`
    The type of mod to fix

modName: :class:`str`
    The name of the mod to fix to :raw-html:`<br />` :raw-html:`<br />`

    **Default**: ``""``

Returns
-------
List[:class:`IniGraphGroup`]
    The same list that was passed in, after editing
    )doc"));
}
