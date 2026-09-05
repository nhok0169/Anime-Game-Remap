#ifndef AGRemapCore_GlobalRemapIniRemover_TPP
#define AGRemapCore_GlobalRemapIniRemover_TPP

#include <utility>


namespace AGRemapCore {

    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    std::function<std::shared_ptr<BaseIniRemover<>>(IniFile*)> GlobalRemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::factory() {
        return [](IniFile* iniFile) -> std::shared_ptr<BaseIniRemover<>> {
            // setIniFile is what builds the IniFileRemoveContext -- see RemapIniRemover::setIniFile.
            // The plain <std::string, std::string> instantiation, not this one: an IniFile* can only
            // ever be turned into an IniFileRemoveContext, which is that instantiation's context.
            std::shared_ptr<GlobalRemapIniRemover<>> result = std::make_shared<GlobalRemapIniRemover<>>();
            result->setIniFile(iniFile);
            return result;
        };
    }


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    GlobalRemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::GlobalRemapIniRemover(Context* ctx, RemoverConfig config):
        Base(ctx, std::move(config)) {}


    template <typename K, typename V, typename KeyHash, typename KeyEqual, typename RemoverBase>
    std::string GlobalRemapIniRemover<K, V, KeyHash, KeyEqual, RemoverBase>::remove(bool parse, bool writeBack, IniRemovalContext context) {
        // The one thing this class is. 'context' is our own copy (taken by value all the way down
        // from BaseIniRemover::remove), so the caller's object is not written through -- see the
        // declaration's note.
        context.ignoreModType = true;
        return Base::remove(parse, writeBack, context);
    }
}

#endif
