#include "AGRemapCore/data/IniFixData/DarkDiffuse.h"

#include "AGRemapCore/model/files/TextureFile.h"
#include "AGRemapCore/model/strategies/texEditors/TexEditor.h"


namespace AGRemapCore {

    // 1 / 2.2 -- spelled as the division rather than 0.4545... so it reads as the sRGB exponent it
    // is, exactly as the pure-Python "1 / ColourConsts.StandardGamma" does.
    const double DarkDiffuse::Gamma = 1.0 / 2.2;


    void DarkDiffuse::edit(TextureFile& texFile) {
        // setTransparency already no-ops on a texture with no image, so there is nothing to guard
        // here that it does not guard itself.
        TexEditor::setTransparency(texFile, 0);

        // Metadata rather than a pixel pass: save() applies it through GammaFilter immediately
        // before encoding, which is the only point at which the correction is wanted. Doing it here
        // would gamma-correct the buffer and then correct it again on the way out.
        texFile.setGamma(Gamma);
    }
}
