#ifndef AGRemapCore_IniNamingTools_H
#define AGRemapCore_IniNamingTools_H

#include <optional>
#include <string>
#include <utility>


namespace AGRemapCore {

    /**
     * @brief
     @rst
     Utilities for some common naming conventions for .ini files :raw-html:`<br />` :raw-html:`<br />`

     Mirrors the pure-Python ``IniNamingTools`` class (``model/IniNamingTools.py``)
     @endrst
     */
    class IniNamingTools {
        public:

            /**
             * @brief
             @rst
             Makes the name of a `section`_ to be used for the resource `sections`_ of a .ini file
             :raw-html:`<br />` :raw-html:`<br />`

             Examples: ``"CuteLittleEi"`` -> ``"ResourceCuteLittleEi"``;
             ``"ResourceCuteLittleEi"`` -> ``"ResourceCuteLittleEi"`` (unchanged)
             @endrst
             *
             * @param name The name of the `section`_
             *
             * @return The name of the `section`_ as a resource in a .ini file
             */
            static std::string getResourceName(const std::string& name);

            /**
             * @brief
             @rst
             Removes the 'Resource' prefix from a section's name :raw-html:`<br />` :raw-html:`<br />`

             Examples: ``"ResourceCuteLittleEi"`` -> ``"CuteLittleEi"``;
             ``"LittleMissGanyu"`` -> ``"LittleMissGanyu"`` (unchanged)
             @endrst
             *
             * @param name The name of the `section`_
             *
             * @return The name of the `section`_ with the 'Resource' prefix removed
             */
            static std::string removeResourceName(const std::string& name);

            /**
             * @brief
             @rst
             Changes a `section`_ name to have the keyword from 'elementName' to identify that the
             `section`_ is created by this fix -- replaces the LAST occurrence of 'elementName'
             within 'name' with ``{modName}Remap{elementName}``, or appends it if 'elementName'
             isn't found :raw-html:`<br />` :raw-html:`<br />`

             Example: ``getRemapElementName("EiTriesToUseBlenderAndFails", "Blend", "Raiden")`` ->
             ``"EiTriesToUseRaidenRemapBlenderAndFails"``
             @endrst
             *
             * @param name The name of the `section`_
             * @param elementName The name of the target element
             * @param modName The name of the mod to fix
             *
             * @return The name of the `section`_ with the keyword of 'elementName', prefixed by the
             *      word 'Remap', added
             */
            static std::string getRemapElementName(const std::string& name, const std::string& elementName, const std::string& modName = "");

            /**
             * @brief
             @rst
             Changes a `section`_ name to have the keyword 'RemapBlend' to identify that the
             `section`_ is created by this fix -- see #getRemapElementName for the general behavior
             @endrst
             */
            static std::string getRemapBlendName(const std::string& name, const std::string& modName = "");

            /**
             * @brief
             @rst
             Changes a `section`_ name to have the keyword 'RemapPosition' to identify that the
             `section`_ is created by this fix -- see #getRemapElementName for the general behavior
             @endrst
             */
            static std::string getRemapPositionName(const std::string& name, const std::string& modName = "");

            /**
             * @brief
             @rst
             Changes a `section`_ name to have the keyword 'RemapTexcoord' to identify that the
             `section`_ is created by this fix -- see #getRemapElementName for the general behavior
             @endrst
             */
            static std::string getRemapTexcoordName(const std::string& name, const std::string& modName = "");

            /**
             * @brief
             @rst
             Changes a `section`_ name to have the keyword 'RemapIb' to identify that the `section`_
             is created by this fix -- see #getRemapElementName for the general behavior
             @endrst
             */
            static std::string getRemapIbName(const std::string& name, const std::string& modName = "");

            /**
             * @brief
             @rst
             Changes a `section`_ name to have the suffix of 'modName' followed by 'suffix'
             :raw-html:`<br />` :raw-html:`<br />`

             * If 'name' already ends with ``{modName}{suffix}``, returns 'name' unchanged
             * Else if 'name' ends with just 'suffix' (no 'modName'), replaces that trailing
               'suffix' with ``{modName}{suffix}``
             * Otherwise, appends ``{modName}{suffix}`` to the end of 'name'
             @endrst
             *
             * @param name The name of the `section`_
             * @param suffix The name of the suffix to put at the end of the `section`_
             * @param modName The name of the mod to fix
             *
             * @return The name of the `section`_ with the added suffix keyword
             */
            static std::string getModSuffixedName(const std::string& name, const std::string& suffix = "", const std::string& modName = "");

            /**
             * @brief
             @rst
             Changes a `section`_ name to have the suffix 'RemapFix' to identify that the `section`_
             is created by this fix :raw-html:`<br />` :raw-html:`<br />`

             Examples: ``getRemapFixName("EiIsDoneWithRemapFix", "Raiden")`` ->
             ``"EiIsDoneWithRaidenRemapFix"``; ``getRemapFixName("EiIsHappy", "Raiden")`` ->
             ``"EiIsHappyRaidenRemapFix"``
             @endrst
             *
             * @param name The name of the `section`_
             * @param modName The name of the mod to fix
             *
             * @return The name of the `section`_ with the added 'RemapFix' keyword
             */
            static std::string getRemapFixName(const std::string& name, const std::string& modName = "");

            /**
             * @brief
             @rst
             Changes a `section`_ name to have the keyword 'RemapTex' to identify that the `section`_
             is created by this fix -- see #getRemapFixName for examples of the same shape
             @endrst
             */
            static std::string getRemapTexName(const std::string& name, const std::string& modName = "");

            /**
             * @brief
             @rst
             Changes a `section`_ name to have the suffix 'RemapDL' to identify that the `section`_
             is created by this fix -- see #getRemapFixName for examples of the same shape
             @endrst
             */
            static std::string getRemapDLName(const std::string& name, const std::string& modName = "");

            /**
             * @brief
             @rst
             Changes a `section`_ name to be a new non-blend resource created by this fix -- see
             #getResourceName and #getRemapFixName for more info
             @endrst
             */
            static std::string getRemapFixResourceName(const std::string& name, const std::string& modName = "");

            /**
             * @brief
             @rst
             Changes a `section`_ name to be a texture resource created by this fix -- see
             #getResourceName and #getRemapTexName for more info
             @endrst
             */
            static std::string getRemapTexResourceName(const std::string& name, const std::string& modName = "");

            /**
             * @brief
             @rst
             Changes a `section`_ name to be a downloaded-file resource created by this fix -- see
             #getResourceName and #getRemapDLName for more info
             @endrst
             */
            static std::string getRemapDLResourceName(const std::string& name, const std::string& modName = "");

            /**
             * @brief
             @rst
             Changes a `section`_ name to be a new blend resource that this fix will create -- see
             #getResourceName and #getRemapBlendName for more info
             @endrst
             */
            static std::string getRemapBlendResourceName(const std::string& name, const std::string& modName = "");

            /**
             * @brief
             @rst
             Changes a `section`_ name to be a new position resource that this fix will create --
             see #getResourceName and #getRemapPositionName for more info
             @endrst
             */
            static std::string getRemapPositionResourceName(const std::string& name, const std::string& modName = "");

            /**
             * @brief
             @rst
             Retrieves the file path for a fixed element, using `pathlib`_-style folder resolution
             (a bare filename with no directory component resolves to folder ``"."``, so the result
             always has a folder prefix -- contrast with #getFixedElementFile) :raw-html:`<br />`
             :raw-html:`<br />`
             @endrst
             *
             * @param file The file path to the original file
             * @param modName The name of the mod to fix to
             * @param fileExt The file extension for the file path of the fixed element. If this is
             *      ``std::nullopt``, uses the file extension already present in 'file'
             *
             * @return The file path of the fixed file of the element
             */
            static std::string getFixedFile(const std::string& file, const std::string& modName = "", std::optional<std::string> fileExt = std::nullopt);

            /**
             * @brief
             @rst
             Retrieves the file path for a fixed element :raw-html:`<br />` :raw-html:`<br />`

             .. note::
                Unlike #getFixedFile, a bare filename with no directory component returns with no
                folder prefix at all (matches the pure-Python original's own explicit special-case
                for this)
             @endrst
             *
             * @param file The file path to the original file
             * @param elementName The name of the element to fix
             * @param modName The name of the mod to fix to
             * @param fileExt The file extension for the file path of the fixed element. If this is
             *      ``std::nullopt``, uses the file extension already present in 'file'
             *
             * @return The file path of the fixed file of the element
             */
            static std::string getFixedElementFile(const std::string& file, const std::string& elementName, const std::string& modName = "", std::optional<std::string> fileExt = std::nullopt);

            /**
             * @brief Retrieves the file path for the fixed RemapBlend.buf file
             *
             * @param blendFile The file path to the original Blend.buf file
             * @param modName The name of the mod to fix to
             *
             * @return The file path of the fixed RemapBlend.buf file
             */
            static std::string getFixedBlendFile(const std::string& blendFile, const std::string& modName = "");

            /**
             * @brief Retrieves the file path for the fixed RemapPosition.buf file
             *
             * @param positionFile The file path to the original Position.buf file
             * @param modName The name of the mod to fix to
             *
             * @return The file path of the fixed RemapPosition.buf file
             */
            static std::string getFixedPositionFile(const std::string& positionFile, const std::string& modName = "");

            /**
             * @brief
             @rst
             Retrieves the file path for the fixed RemapTex.dds file :raw-html:`<br />`
             :raw-html:`<br />`

             .. note::
                Built like the pure-Python original's own ``os.path.dirname``/``os.path.basename``
                based implementation, not the `pathlib`_-based one #getFixedFile/#getFixedElementFile
                use -- a bare filename with no directory component returns with no folder prefix at
                all (same no-prefix behavior as #getFixedElementFile, but arrived at differently)
             @endrst
             *
             * @param texFile The file path to the original .dds file
             * @param modName The name of the mod to fix to
             *
             * @return The file path of the fixed RemapTex.dds file
             */
            static std::string getFixedTexFile(const std::string& texFile, const std::string& modName = "");

            /**
             * @brief Retrieves the name to some generic ``TextureOverride`` `section`_ this software has made
             *
             * @param component The name of the component
             * @param obj The name of the object
             * @param modName The name of the mod
             *
             * @return The name for the `section`_
             */
            static std::string getTextureOverrideRemapFix(const std::string& component, const std::string& obj, const std::string& modName = "");

            /**
             * @brief
             @rst
             Retrieves the new name of the `section`_ for a new mod object :raw-html:`<br />`
             :raw-html:`<br />`

             Finds the LAST case-insensitive occurrence of ``{objName.first}{objName.second}``
             (each part `capitalized <#capitalize>`_) within 'name' and replaces it with
             ``{newObjName.first}{newObjName.second}`` (also capitalized); if not found, falls back
             to #getRemapFixName with 'modName' extended by the new object's name instead
             @endrst
             *
             * @param name The name of the `section`_
             * @param modName The name of the mod to be fixed
             * @param objName The (component, object) name pair for the original mod object for the `section`_
             * @param newObjName The (component, object) name pair for the new mod object for the `section`_
             *
             * @return The new name for the `section`_
             */
            static std::string getObjRemapFixName(const std::string& name, const std::string& modName,
                                                   const std::pair<std::string, std::string>& objName,
                                                   const std::pair<std::string, std::string>& newObjName);
    };
}

#endif
